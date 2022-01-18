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
#include "itc_manager.h"

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
}

void NodeBaseImpl::syncCb(const zmq::message_t &msg) {
    const auto* sync = msg::GetSyncBroadcast(msg.data());
    bool local = sync->node()->pid() == kPid && sync->node()->tcp_addr()->str() == sub_container_->getTcpAddr();
    if (local) return;

    if (sync->type()==msg::SyncType_Remove)
        AssertLog(false, "not implemented yet");

    auto update = RemoteManager::GlobalManager().wholeSyncUpdate(msg.data(), msg.size());

    updatePubConnection(sync->node(), update);
}

void NodeBaseImpl::syncFunc(){
    flatbuffers::FlatBufferBuilder builder;
    flatbuffers::Offset<msg::SyncBroadcast> sync_fb;
    auto node_id = msg::CreateNodeIdDirect(builder, nid_, kPid,
                                           sub_container_->getTcpAddr().c_str(),
                                           sub_container_->getIpcAddr().c_str(),
                                           name_.c_str());

    // broadcast self sync info
    vector<flatbuffers::Offset<msg::SubscriberInfo>> subs_vect;
    {
        // do we need this lock? seems only reading could take place in parallel
        lock_guard<mutex> lock(sub_container_->subs_mu_);
        subs_vect.reserve(sub_container_->subs_.size());
        for (const auto &t : sub_container_->subs_){
            subs_vect.push_back(msg::CreateSubscriberInfoDirect(builder,
                        t.first.c_str(), (*t.second.begin())->protocol_.c_str()));
        }
    }
    sync_fb = msg::CreateSyncBroadcastDirect(builder, node_id,
                            sync_version_.load(std::memory_order_acquire),
                              msg::SyncType_Whole, &subs_vect, nullptr);
    builder.Finish(sync_fb);
    sync_pub_->publishIpc(zmq::message_t(builder.GetBufferPointer(), builder.GetSize()));
    log.d(__func__, "broadcast sync once: %s",
          ToString(msg::GetSyncBroadcast(builder.GetBufferPointer())).c_str());
}

void NodeBaseImpl::checkRemote() {
    auto removal = RemoteManager::GlobalManager().checkAndRemove();
    for (const auto &str : removal){
        const auto* sync = msg::GetSyncBroadcast(str.data());
        RemoteManager::NodeUpdate update;
        update.topics_removed.reserve(sync->subscribers()->size());
        for (const auto *sub: *sync->subscribers()) {
            update.topics_removed.push_back(sub->topic()->str());
        }
        updatePubConnection(sync->node(), update);
    }
}

bool NodeBaseImpl::shouldConnectIpc(const msg::NodeId *sync) const {
    if (param_.enable_ipc_socket &&
        sync->ipc_addr()->size()>0 &&
        IpFromTcp(sync->tcp_addr()->str()) == kIpStr)
        return true;
    return false;
}

bool NodeBaseImpl::shouldConnectTcp(const msg::NodeId *sync) const {
    if (param_.enable_tcp_socket &&
        sync->tcp_addr()->size()>0)
        return true;
    return false;
}

SubscriberBaseImpl* NodeBaseImpl::addSubscriber(const string &topic,
                            const shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
                            const IpcFunc &ipc_cb,
                            const ItcFunc &itc_cb,
                            const DeserFunc &deserialize_f,
                            const string &protocol) {
    auto sub = make_unique<SubscriberBaseImpl>(
            topic, tp, queue_size, ipc_cb, itc_cb, deserialize_f, protocol);
    auto *sub_raw = sub.get();
    bool new_topic = sub_container_->addSub(move(sub));
    if (new_topic){
        sync_version_.fetch_add(1, memory_order_release);
        sync_scheduler_.cancel(sync_task_);
        sync_task_ = make_shared<Scheduler::Task>(*sync_task_);
        sync_scheduler_.schedule(sync_task_);
    }
    ItcManager::GlobalManager().addSub(sub_raw);
    return sub_raw;
}

void NodeBaseImpl::removeSubscriber(SubscriberBaseImpl* &sub){
    AssertLog(sub, "");
    // remove pointer from itc first
    ItcManager::GlobalManager().removeSub(sub);
    // remove sub from container
    bool topic_removed = sub_container_->removeSub(sub);
    if (topic_removed){
        sync_version_.fetch_add(1, std::memory_order_release);
        sync_scheduler_.cancel(sync_task_);
        sync_task_ = make_shared<Scheduler::Task>(*sync_task_);
        sync_scheduler_.schedule(sync_task_);
    }
    sub = nullptr;
}


PublisherBaseImpl * NodeBaseImpl::addPublisher(const string &topic,
                                               const string &protocol) {
    auto pub = make_unique<PublisherBaseImpl>(topic, protocol, context_);
    auto *pub_raw =  pub.get();

    // connect in single thread sync_tp_ to avoid locking for connect operation.
    // it is ok to capture reference since we will wait the future.
    auto connect_future = sync_tp_->enqueue([&]{
        auto itr = RemoteManager::GlobalManager().topic_book.find(topic);
        if (itr!=RemoteManager::GlobalManager().topic_book.end()){
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
    });
    connect_future.wait();
    this_thread::sleep_for(50ms);

    return pub_raw;
}


void NodeBaseImpl::removePublisher(PublisherBaseImpl *pub) {
    AssertLog(pub, "");
    auto remove_future = sync_tp_->enqueue([pub, this]{
        MapVecRemove(pubs_, pub->topic_, pub,
                     [pub](const unique_ptr<PublisherBaseImpl> &p){return p.get()==pub;});

    });
    remove_future.wait();
}

void NodeBaseImpl::shutdown() {
    if (is_down_.load(std::memory_order_acquire))
        return;

    log.w(__func__, __func__ );
    // todo ivan. broadcast its death
    std::this_thread::sleep_for(100ms);

    // shutdown sub_container
    syncsub_container_->stop();
    sub_container_->stop();

    sync_tp_->stopAndClear();

    // remove itc subs
    auto subs = sub_container_->getSubs();
    ItcManager::GlobalManager().batchRemove(subs);
    // note: subs in sub_container are not removed here

    is_down_.store(true, memory_order_release);
    sync_scheduler_.stop();
}

void NodeBaseImpl::connect(const std::string &addr_in, const std::string &addr_out) {
    if (connected.test_and_set())
        AssertLog(false, "double connect");

    bool res1 = sync_pub_->connect(addr_in);
    bool res2 = syncsub_container_->connectRaw(addr_out);

    AssertLog(res1&&res2, "failed to connect to " + addr_in + " and " + addr_out);
    syncsub_container_->start();

    if (param_.enable_tcp_socket){
        bool tcp_binding = sub_container_->bindTcpRaw();
        AssertLog(tcp_binding, "");
    }
    if (param_.enable_ipc_socket){
        bool ipc_binding = sub_container_->bindIpcRaw();
        AssertLog(ipc_binding, "");
    }
    this_thread::sleep_for(50ms);
    sub_container_->start();

    sync_task_ = make_shared<Scheduler::Task>([this]{ syncFunc();}, kNodeHbPeriod*1e-3, 0);
    sync_scheduler_.schedule(sync_task_);
    sync_scheduler_.schedule( Scheduler::Task{ [this]{ checkRemote();}, kNodeOfflineCheckPeriod*1e-3, 0});
}

}
