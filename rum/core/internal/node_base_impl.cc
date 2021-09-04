//
// Created by Ivan B on 2021/4/16.
//

#include "node_base_impl.h"

#include <rum/common/zmq_helper.h>
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/core/msg/rum_sync_generated.h>
#include <rum/extern/ivtb/stopwatch.h>

using namespace std;

namespace rum{

ItcManager NodeBaseImpl::itc_manager_;
atomic_int NodeBaseImpl::id_pool_{1};

string addrConcatenate(const string &addr1, const string &addr2){
    return addr1 + ";;;" + addr2;
}

NodeBaseImpl::NodeBaseImpl(string name, string domain, string addr):
        name_(move(name)), context_(shared_context()), nid_(id_pool_.fetch_add(1, memory_order_relaxed)) {
    sub_container_ = make_unique<SubContainer>(context_, true);
    syncsub_container_ = make_unique<SubContainer>(context_, false);
    sync_pub_ = make_unique<PublisherBaseImpl>(kSyncTopic, "", context_, false);
    sync_sub_ = make_unique<SubscriberBaseImpl>(kSyncTopic, make_shared<ThreadPool>(1), 1000,
            [this](zmq::message_t& msg){ syncCb(msg);}, nullptr);
    syncsub_container_->addSub(sync_sub_.get());
}

void NodeBaseImpl::syncCb(zmq::message_t &msg) {
}

void NodeBaseImpl::syncF(){
    // todo ivan. doing here. test this
    ivtb::Stopwatch timer;
    unique_lock<mutex> lock(sync_mu_);
    auto last_sync_v = sync_version_;
    while (!is_down_.load(memory_order_acquire)){
        double remain_t = kNodeHbPeriod-timer.passedMs();
        // if less than 10ms or sync version changed we quit waiting.
        // remain_t > heartbeat: system time change maybe
        while( remain_t > 10.0 && remain_t < kNodeHbPeriod &&
                last_sync_v==sync_version_ && is_down_.load(memory_order_acquire)){
            hb_cv_.wait_for(lock, (remain_t)*1ms);
            remain_t = kNodeHbPeriod-timer.passedMs();
        }

        flatbuffers::FlatBufferBuilder builder;
        auto node_id = msg::CreateNodeIdDirect(builder, nid_, pid, syncsub_container_->getTcpAddr().c_str(),
                                               syncsub_container_->getIpcAddr().c_str(), name_.c_str());

        // auto fb_msg = msg::CreateSubscriberInfo()
        vector<flatbuffers::Offset<msg::SubscriberInfo>> subs_vect (topics_.size());
        for (const auto &t : topics_){
            subs_vect.push_back(msg::CreateSubscriberInfoDirect(builder, t.first.c_str(), t.second.c_str()));
        }

        auto sync_fb = msg::CreateSyncBroadcastDirect(builder, node_id, sync_version_,
                                                      msg::SyncType_Whole, &subs_vect, nullptr);
        builder.Finish(sync_fb);
        sync_pub_->publishIpc(zmq::message_t(builder.GetBufferPointer(), builder.GetSize()));
        log.v(__func__, "broadcast sync once");

        last_sync_v = sync_version_;
        timer.start();
    }
}

std::unique_ptr<SubscriberBaseImpl> NodeBaseImpl::createSubscriber() {
    return std::unique_ptr<SubscriberBaseImpl>();
}

void NodeBaseImpl::shutdown() {
    // shutdown sub_container
    syncsub_container_->stop();
    sub_container_->stop();

    // remove subs in pub's itc list

    //
    is_down_.store(true, memory_order_release);
    hb_cv_.notify_one();
    sync_t_->join();
}

void NodeBaseImpl::registerSubscriber(SubscriberBaseImpl *sub) {
    bool new_topic = sub_container_->addSub(sub);
    if (new_topic){
        lock_guard<mutex> lock(sync_mu_);
        topics_.emplace(sub->topic_, sub->protocol_);
        ++sync_version_;

        // todo notify sync thread
    }
}

void NodeBaseImpl::registerPublisher(PublisherBaseImpl *pub) {

}

void NodeBaseImpl::unregisterSubscriber(SubscriberBaseImpl *sub) {
    // remove sub in sub_container

    // remove sub in itc manager's nodes' pubs' itc list

    // immediately broadcast new sync
}

void NodeBaseImpl::unregisterPublisher(PublisherBaseImpl *pub) {

}

void NodeBaseImpl::connect(const std::vector<std::pair<std::string, std::string>> &addrs) {
    if (connected.test_and_set())
        AssertLog(false, "double connect");

    for (const auto &addr_pair : addrs){
        bool res1 = sync_pub_->connect(addr_pair.first);
        bool res2 = syncsub_container_->connectRaw(addr_pair.second);
        AssertLog(res1&&res2, "failed to connect to " + addr_pair.first + " and " + addr_pair.second);

        lock_guard<mutex> lock(itc_manager_.nodes_mu);
        auto itr = itc_manager_.nodes.find(addr_pair.first);
        if (itr == itc_manager_.nodes.end()){
            itc_manager_.nodes.emplace(addr_pair.first, std::vector<NodeBaseImpl*>{this});
        }
        else{
            itr->second.push_back(this);
        }
    }
    domains_ = addrs;

    sub_container_->start();
    syncsub_container_->start();

    sync_t_ = make_unique<thread>([this]{
        syncF();
    });
}

}
