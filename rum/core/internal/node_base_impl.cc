//
// Created by Ivan B on 2021/4/16.
//

#include "node_base_impl.h"

#include <rum/common/zmq_helper.h>
#include <rum/common/log.h>
#include <rum/common/common.h>
#include "rum/common/misc.h"
#include <rum/core/msg/rum_sync_generated.h>
#include <rum/extern/ivtb/stopwatch.h>
#include "rum/core/msg/util.h"
#include "remote_manager.h"
#include "itc_manager.h"

using namespace std;
using namespace ivtb;

namespace rum{

atomic_int NodeBaseImpl::id_pool_{1};

// string addrConcatenate(const string &addr1, const string &addr2){
//     return addr1 + ";;;" + addr2;
// }

NodeBaseImpl::NodeBaseImpl(string name, Param param):
        name_(move(name)), param_(move(param)), context_(shared_context()),
        nid_(id_pool_.fetch_add(1, memory_order_relaxed)) {
    sub_container_ = make_unique<SubContainer>(context_, true);
    syncsub_container_ = make_unique<SubContainer>(context_, false);
    sync_pub_ = make_unique<PublisherBaseImpl>(kSyncTopic, "", context_, false);
    auto sync_sub = make_unique<SubscriberBaseImpl>(kSyncTopic, sync_tp_, 1000,
            [this](zmq::message_t& msg){ syncCb(msg);}, nullptr);
    syncsub_container_->addSub(move(sync_sub));
}

void NodeBaseImpl::syncCb(zmq::message_t &msg) {
    const auto* sync = msg::GetSyncBroadcast(msg.data());
    bool local = sync->node()->pid() == kPid && sync->node()->tcp_addr()->str() == sub_container_->getTcpAddr();
    if (local) return;

    if (sync->type()==msg::SyncType_Remove)
        AssertLog(false, "not implemented yet");

    auto update = RemoteManager::GlobalManager().wholeSyncUpdate(msg.data(), msg.size());

    // connect/disconnect publishers
    auto loopOp =
            [&](const vector<string> &topics,
                const function<void(const vector<unique_ptr<PublisherBaseImpl>>&,
                const msg::NodeId*)> &f){
        for (const auto &topic : topics){
            auto itr = pubs_.find(topic);
            if (itr==pubs_.end()) continue;
            f(itr->second, sync->node());
        }
    };

    // connect/disconnect to ipc socket
    if (!sync->node()->ipc_addr()->str().empty() &&
        IpFromTcp(sync->node()->tcp_addr()->str()) == kIpStr){
        loopOp(update.topics_new,
            [](const vector<unique_ptr<PublisherBaseImpl>>& pubs, const msg::NodeId* node){
            for (auto &pub : pubs) pub->connect(node->ipc_addr()->str());
        });
        loopOp(update.topics_removed,
               [](const vector<unique_ptr<PublisherBaseImpl>>& pubs, const msg::NodeId* node){
                   for (auto &pub : pubs) pub->disconnect(node->ipc_addr()->str());
        });
    }
    // connect/disconnect to tcp socket
    else{
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

void NodeBaseImpl::syncF(){
    flatbuffers::FlatBufferBuilder builder;
    flatbuffers::Offset<msg::SyncBroadcast> sync_fb;
    auto node_id = msg::CreateNodeIdDirect(builder, nid_, kPid,
                                           sub_container_->getTcpAddr().c_str(),
                                           sub_container_->getIpcAddr().c_str(),
                                           name_.c_str());

    // auto fb_msg = msg::CreateSubscriberInfo()
    vector<flatbuffers::Offset<msg::SubscriberInfo>> subs_vect;
    {
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

SubscriberBaseImpl* NodeBaseImpl::addSubscriber(const string &topic,
                            const shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
                            const function<void(zmq::message_t&)> &ipc_cb,
                            const function<void(const void *)> &itc_cb, string protocol) {
    auto sub = make_unique<SubscriberBaseImpl>(
            topic, tp, queue_size, ipc_cb, itc_cb, move(protocol));
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
    bool topic_removed = sub_container_->removeSub(sub);
    if (topic_removed){
        sync_version_.fetch_add(1, std::memory_order_release);
        sync_scheduler_.cancel(sync_task_);
        sync_task_ = make_shared<Scheduler::Task>(*sync_task_);
        sync_scheduler_.schedule(sync_task_);
    }
    ItcManager::GlobalManager().removeSub(sub);
    sub = nullptr;
}

RUM_THREAD_UNSAFE
PublisherBaseImpl * NodeBaseImpl::addPublisher(const string &topic,
                                               const string &protocol) {
    auto pub = make_unique<PublisherBaseImpl>(topic, protocol, context_);
    auto *pub_raw =  pub.get();

    auto connect_future = sync_tp_->enqueue([&]{
        auto itr = RemoteManager::GlobalManager().topic_book.find(topic);
        if (itr!=RemoteManager::GlobalManager().topic_book.end()){
            for (const auto *node_info : itr->second){
                const auto* sync_fb = node_info->getSyncFb();
                if (IpFromTcp(sync_fb->node()->tcp_addr()->str()) == kIpStr){
                    pub->connect(sync_fb->node()->ipc_addr()->str());
                }else{
                    pub->connect(sync_fb->node()->tcp_addr()->str());
                }
            }
        }
        MapVecAdd(pubs_, topic, move(pub));
    });
    connect_future.wait();

    return pub_raw;
}

RUM_THREAD_UNSAFE
void NodeBaseImpl::removePublisher(PublisherBaseImpl *pub) {
    MapVecRemove(pubs_, pub->topic_, pub,
                 [pub](const unique_ptr<PublisherBaseImpl> &p){return p.get()==pub;});
}

void NodeBaseImpl::shutdown() {
    // shutdown sub_container
    syncsub_container_->stop();
    sub_container_->stop();

    sync_tp_->stopAndClear();

    is_down_.store(true, memory_order_release);
    sync_scheduler_.stop();
}

void NodeBaseImpl::connect(const std::string &addr_in, const std::string &addr_out) {
    if (connected.test_and_set())
        AssertLog(false, "double connect");

    bool res1 = sync_pub_->connect(addr_in);
    bool res2 = syncsub_container_->connectRaw(addr_out);
    AssertLog(res1&&res2, "failed to connect to " + addr_in + " and " + addr_out);
    domain_ = {addr_in, addr_out};
    syncsub_container_->start();

    bool tcp_binding = sub_container_->bindTcpRaw();
    AssertLog(tcp_binding, "");
    if (param_.enable_ipc_socket){
        bool ipc_binding = sub_container_->bindIpcRaw();
        AssertLog(ipc_binding, "");
    }
    sub_container_->start();

    sync_task_ = make_shared<Scheduler::Task>([this]{syncF();}, kNodeHbPeriod*1e-3, 0);
    sync_scheduler_.schedule(sync_task_);
}

}
