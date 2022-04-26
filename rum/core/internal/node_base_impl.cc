//
// Created by Ivan B on 2021/4/16.
//

#include "node_base_impl.h"

#include "rum/common/zmq_helper.h"
#include "rum/common/log.h"
#include "rum/common/common.h"
#include "rum/common/misc.h"
#include "rum/core/msg/rum_sync_generated.h"
#include "rum/extern/ivtb/stopwatch.h"
#include "rum/core/msg/util.h"

using namespace std;
using namespace ivtb;

namespace rum{

atomic_int NodeBaseImpl::id_pool_{1};

NodeBaseImpl::NodeBaseImpl(string name, NodeParam param):
        name_(move(name)), param_(move(param)), context_(shared_context()),
        nid_(id_pool_.fetch_add(1, memory_order_relaxed)) {
    sub_container_ = make_unique<SubContainer>(context_, true);
    syncsub_container_ = make_unique<SubContainer>(context_, false);
    sync_pub_ = make_unique<PublisherBaseImpl>(kSyncTopic, "", context_, false);
    auto sync_sub = make_unique<SubscriberBaseImpl>(kSyncTopic, sync_tp_, 1000,
            [this](const shared_ptr<const void> &msg){
                syncCb(*static_pointer_cast<const Message>(msg));
            },
            nullptr,
            [](shared_ptr<const Message> &msg, const string&){return move(msg);} );
    syncsub_container_->addSub(move(sync_sub));
}

NodeBaseImpl::~NodeBaseImpl() {
    shutdown();
}

void NodeBaseImpl::updatePubConnection(const msg::NodeId *remote_node, RemoteManager::NodeUpdate &update) {
    auto loopOp =
            [this, remote_node](const vector<string> &topics,
                const function<void(const vector<unique_ptr<PublisherBaseImpl>>&,
                                    const msg::NodeId*)> &f){
                for (const auto &topic : topics){
                    auto itr = pubs_.find(topic);
                    if (itr==pubs_.end()) continue;
                    f(itr->second, remote_node);
                }
            };

    // messages
    if (shouldConnectIpc(remote_node)){
        loopOp(update.topics_new,
               [](const vector<unique_ptr<PublisherBaseImpl>>& pubs, const msg::NodeId* node){
                   for (auto &pub : pubs) pub->connect(node->ipc_addr()->str());
               });
        loopOp(update.topics_removed,
               [](const vector<unique_ptr<PublisherBaseImpl>>& pubs, const msg::NodeId* node){
                   for (auto &pub : pubs) pub->disconnect(node->ipc_addr()->str());
               });
    }
    else if (shouldConnectTcp(remote_node)){
        loopOp(update.topics_new,
               [](const vector<unique_ptr<PublisherBaseImpl>>& pubs, const msg::NodeId* node){
                   for (auto &pub : pubs) pub->connect(node->tcp_addr()->str());
               });
        loopOp(update.topics_removed,
               [](const vector<unique_ptr<PublisherBaseImpl>>& pubs, const msg::NodeId* node){
                   for (auto &pub : pubs) pub->disconnect(node->tcp_addr()->str());
               });
    }

    // srvs
    for (const auto &rep_topic : update.rep_topics_new){
        auto srv_name = (string)SrvFromRepTopic(rep_topic);
        auto itr = servers_.find(srv_name);
        if (itr==servers_.end()) continue;
        auto *server = itr->second.get();
        // connected as well in this step
        auto *pub = internalAddPublisher(rep_topic, server->pub_protocol_, msg::MsgType::MsgType_ServiceResponse);
        server->addPub(pub);
    }
    for (const auto &rep_topic : update.rep_topics_removed){
        auto srv_name = (string)SrvFromRepTopic(rep_topic);
        auto itr = servers_.find(srv_name);
        if (itr==servers_.end()) continue;
        auto *server = itr->second.get();
        auto *pub = server->removePub((string)IdFromRepTopic(rep_topic));
        MapVecRemove(pubs_, pub->topic_, pub,
                     [pub](const unique_ptr<PublisherBaseImpl> &p){return p.get()==pub;});
    }

}

void NodeBaseImpl::syncCb(const zmq::message_t &msg) {
    const auto* sync = msg::GetSyncBroadcast(msg.data());
    bool local = sync->node()->pid() == kPid && sync->node()->tcp_addr()->str() == sub_container_->getTcpAddr();
    if (local) return;

    if (sync->type()==msg::SyncType_Remove)
        AssertLog(false, "not implemented yet");

    auto update = remote_manager_->wholeSyncUpdate(msg.data(), msg.size());
    if (!update.empty())
        log.d(__func__, update.toString());
    updatePubConnection(sync->node(), update);
}

void NodeBaseImpl::syncFunc(){
    using namespace flatbuffers;

    vector<flatbuffers::Offset<msg::SubscriberInfo>> subs_vect;
    vector<flatbuffers::Offset<msg::SubscriberInfo>> clis_vect;
    bool update_required = false;
    // retrieve infos from sub_container_
    {
        lock_guard<mutex> lock(sub_container_->subs_mu_);
        unsigned long sub_version = sub_container_->version_;

        if (sub_version!=sync_fb_version_ || !sync_fb_builder_){
            update_required = true;
            sync_fb_version_ = sub_version;
            sync_fb_builder_ = make_shared<FlatBufferBuilder>();

            // subs_vect.reserve(sub_container_->subs_.size());
            for (const auto &t : sub_container_->subs_){
                if (IsRepTopic(t.first.c_str())){
                    clis_vect.push_back(msg::CreateSubscriberInfoDirect(*sync_fb_builder_,
                                t.first.c_str(), (*t.second.begin())->protocol_.c_str()));

                } else{
                    subs_vect.push_back(msg::CreateSubscriberInfoDirect(*sync_fb_builder_,
                                t.first.c_str(), (*t.second.begin())->protocol_.c_str()));
                }
            }
        }
    }

    // fill other infos that require no sub_container_'s lock
    if (update_required){
        flatbuffers::Offset<msg::SyncBroadcast> sync_fb;
        auto node_id = msg::CreateNodeIdDirect(*sync_fb_builder_, nid_, kPid,
                                               sub_container_->getTcpAddr().c_str(),
                                               sub_container_->getIpcAddr().c_str(),
                                               name_.c_str());
        sync_fb = msg::CreateSyncBroadcastDirect(*sync_fb_builder_, node_id, sync_fb_version_,
                                                 msg::SyncType_Whole, &subs_vect, &clis_vect);
        sync_fb_builder_->Finish(sync_fb);
    }

    // build and send the message
    auto *builder_count_keep = new shared_ptr<FlatBufferBuilder>(sync_fb_builder_);
    Message fb_msg{sync_fb_builder_->GetBufferPointer(), sync_fb_builder_->GetSize(),
                   [](void*, void* builder_sptr){ delete (shared_ptr<FlatBufferBuilder>*)builder_sptr;},
                   builder_count_keep};
    sync_pub_->publishIpc(fb_msg);
    log.d(__func__, "broadcast sync once: %s",
          ToString(msg::GetSyncBroadcast(sync_fb_builder_->GetBufferPointer())).c_str());
}

void NodeBaseImpl::checkRemote() {
    auto removal = remote_manager_->checkAndRemove();
    for (const auto &str : removal){
        const auto* sync = msg::GetSyncBroadcast(str.data());
        RemoteManager::NodeUpdate update;
        update.topics_removed.reserve(sync->subscribers()->size());
        for (const auto *sub: *sync->subscribers()) {
            update.topics_removed.push_back(sub->topic()->str());
        }
        update.rep_topics_removed.reserve(sync->clients()->size());
        for (const auto *cli: *sync->clients()) {
            update.rep_topics_removed.push_back(cli->topic()->str());
        }
        updatePubConnection(sync->node(), update);
    }
}

bool NodeBaseImpl::shouldConnectIpc(const msg::NodeId *sync) const {
    if (param_.enable_ipc_txrx &&
        sync->ipc_addr()->size()>0 &&
        IpFromTcp(sync->tcp_addr()->str()) == kIpStr)
        return true;
    return false;
}

bool NodeBaseImpl::shouldConnectTcp(const msg::NodeId *sync) const {
    if (param_.enable_tcp_tx &&
        sync->tcp_addr()->size()>0)
        return true;
    return false;
}

SubscriberBaseImpl* NodeBaseImpl::addSubscriber(const string &topic,
                            const shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
                            const IpcFunc &ipc_cb,
                            const ItcFunc &itc_cb,
                            const DeserFunc<> &deserialize_f,
                            const string &protocol) {
    auto sub = make_unique<SubscriberBaseImpl>(
            topic, tp, queue_size, ipc_cb, itc_cb, deserialize_f, protocol);
    auto *sub_raw = sub.get();
    bool new_topic = sub_container_->addSub(move(sub));
    if (new_topic){
        sync_scheduler_.cancel(sync_task_);
        sync_task_ = make_shared<Scheduler::Task>(*sync_task_);
        sync_scheduler_.schedule(sync_task_);
    }
    itc_manager_->addSub(sub_raw);
    return sub_raw;
}

void NodeBaseImpl::removeSubscriber(SubscriberBaseImpl* &sub){
    AssertLog(sub, "");
    // remove pointer from itc first
    itc_manager_->removeSub(sub);
    // remove sub from container
    bool topic_removed = sub_container_->removeSub(sub);
    if (topic_removed){
        sync_scheduler_.cancel(sync_task_);
        sync_task_ = make_shared<Scheduler::Task>(*sync_task_);
        sync_scheduler_.schedule(sync_task_);
    }
    sub = nullptr;
}

PublisherBaseImpl* NodeBaseImpl::internalAddPublisher(const string &topic,
                                  const string &protocol, msg::MsgType msg_type) {
    auto pub = make_unique<PublisherBaseImpl>(topic, protocol, context_, false, msg_type);
    auto *pub_raw =  pub.get();
    std::unordered_map<std::string, std::vector<RemoteManager::NodeInfo*>> *book = &remote_manager_->sub_book;

    auto itr = book->find(topic);
    if (itr!=book->end()){
        for (const auto *node_info : itr->second){
            const auto* sync_fb = node_info->getSyncFb();
            if (shouldConnectIpc(sync_fb->node())){
                pub->connect(sync_fb->node()->ipc_addr()->str());
            }else if (shouldConnectTcp(sync_fb->node())){
                pub->connect(sync_fb->node()->tcp_addr()->str());
            }
        }
    }
    MapVecAdd(pubs_, topic, move(pub));
    return pub_raw;
}

PublisherBaseImpl * NodeBaseImpl::addPublisher(const string &topic, const string &protocol,
                                               msg::MsgType msg_type) {
    // connect in single thread sync_tp_ to avoid locking for connect operation.
    // it is ok to capture reference since we will wait the future.
    auto connect_future = sync_tp_->enqueue([&](){
        return internalAddPublisher(topic, protocol, msg_type);
    });
    auto* pub_raw = connect_future.get();
    this_thread::sleep_for(50ms);
    return pub_raw;
}


void NodeBaseImpl::removePublisher(PublisherBaseImpl* &pub) {
    AssertLog(pub, "");
    auto remove_future = sync_tp_->enqueue([pub, this]{
        MapVecRemove(pubs_, pub->topic_, pub,
                     [pub](const unique_ptr<PublisherBaseImpl> &p){return p.get()==pub;});

    });
    remove_future.wait();
    pub = nullptr;
}

ClientBaseImpl *NodeBaseImpl::addClient(const string &srv_name, const string &pub_protocol) {
    // todo ivan. can we pass pub,sub factory function to client constructor? or separate pub/sub creation from setup
    auto node_str = getStrId();
    auto cli_id = GetRepTopic(srv_name, node_str);
    auto *pub = addPublisher(GetReqTopic(srv_name), pub_protocol, msg::MsgType_ServiceRequest);
    pub->set_cli_id(cli_id);
    // itc or ipc will never be called
    auto *sub = addSubscriber(cli_id, ClientBaseImpl::sub_dumb_tp_, 0,
                              nullptr, nullptr, nullptr);
    return new ClientBaseImpl(pub, sub);
}

void NodeBaseImpl::removeClient(ClientBaseImpl *&client) {
    // todo ivan. doing here. sync issue?
    removeSubscriber(client->sub_);
    removePublisher(client->pub_);
    delete client;
    client = nullptr;
}

ServerBaseImpl *NodeBaseImpl::addServer(const string &srv_name,
                                        const shared_ptr<ivtb::ThreadPool> &tp,
                                        size_t queue_size,
                                        const SrvIpcFunc &ipc_func,
                                        const SrvItcFunc &itc_func,
                                        const string &sub_protocol,
                                        const string &pub_protocol) {
    // todo ivan. doing here
    auto server = make_unique<ServerBaseImpl>(pub_protocol);
    auto *server_raw = server.get();

    auto server_add_future = sync_tp_->enqueue([&](){
        auto itr = servers_.find(srv_name);
        if (itr!=servers_.end()) return false;

        // create new pub to exists remote clients.
        std::unordered_map<std::string, std::vector<RemoteManager::NodeInfo*>> *book = &remote_manager_->exclusive_sub_book;

        auto remote_itr = book->find(srv_name);
        if (remote_itr!=book->end()){
            for (const auto *node_info : remote_itr->second){
                const auto* sync_fb = node_info->getSyncFb();
                for (auto *cli : *sync_fb->clients()){
                    auto *pub = internalAddPublisher(
                            cli->topic()->str(), server->pub_protocol_, msg::MsgType::MsgType_ServiceResponse);
                    server->addPub(pub);
                }
            }
        }

        servers_[srv_name] = move(server);
        return true;
    });
    bool unique_server = server_add_future.get();
    if(!unique_server){
        printer.e("rum", "failed to add server %s as already exists", srv_name.c_str());
        return nullptr;
    }

    auto *sub = addSubscriber(GetReqTopic(srv_name), tp, queue_size,
                              server_raw->genSubIpc(ipc_func),
                              server_raw->genSubItc(itc_func),
                              nullptr, sub_protocol);
    server_raw->setSub(sub);

    return server_raw;
}

void NodeBaseImpl::removeServer(ServerBaseImpl *&server) {
    string srv_name{server->srvName()};
    removeSubscriber(server->sub_);
    for (auto& pair_itr : server->pubs_)
        removePublisher(pair_itr.second);

    auto server_remove_future = sync_tp_->enqueue([&](){
        servers_.erase(srv_name);
    });
    server_remove_future.wait();

    server = nullptr;
}

void NodeBaseImpl::shutdown() {
    if (is_down_.load(std::memory_order_acquire))
        return;

    // log.w(__func__, __func__ );
    // todo ivan. broadcast its death
    std::this_thread::sleep_for(100ms);

    // shutdown sub_container
    syncsub_container_->stop();
    sub_container_->stop();

    sync_scheduler_.stop();
    sync_tp_->stopAndClear();

    // subs
    auto subs = sub_container_->getSubs();
    itc_manager_->batchRemove(subs);
    sub_container_->clearSubs();

    is_down_.store(true, memory_order_release);
    // log.w(__func__, "finished" );

    // todo ivan. deal with server and client
}

void NodeBaseImpl::connect(const std::string &addr_in, const std::string &addr_out) {
    if (connected.test_and_set())
        AssertLog(false, "double connect");

    bool res1 = sync_pub_->connect(addr_in);
    bool res2 = syncsub_container_->connectRaw(addr_out);
    this_thread::sleep_for(50ms);

    AssertLog(res1&&res2, "failed to connect to " + addr_in + " and " + addr_out);
    syncsub_container_->start();

    bool tcp_binding = sub_container_->bindTcpRaw();
    AssertLog(tcp_binding, "");
    if (param_.enable_ipc_txrx){
        bool ipc_binding = sub_container_->bindIpcRaw();
        AssertLog(ipc_binding, "");
    }
    this_thread::sleep_for(50ms);
    sub_container_->start();

    sync_task_ = make_shared<Scheduler::Task>([this]{
            sync_tp_->enqueue([this]{syncFunc();});
        }, kNodeHbPeriod*1e-3, 0);
    sync_scheduler_.schedule(sync_task_);
    sync_scheduler_.schedule( Scheduler::Task{ [this]{
            sync_tp_->enqueue([this]{checkRemote();});
        }, kNodeOfflineCheckPeriod*1e-3, 0});
}

}
