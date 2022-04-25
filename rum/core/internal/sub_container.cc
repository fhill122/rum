//
// Created by Ivan B on 2021/3/24.
//

#include "sub_container.h"

#include "rum/common/common.h"
#include "rum/common/log.h"
#include "rum/common/zmq_helper.h"
#include "rum/common/misc.h"
#include "rum/core/msg/rum_header_generated.h"
#include "publisher_base_impl.h"
#include "client_base_impl.h"

using namespace std;

constexpr char TAG[] = "SubContainer";

namespace rum {

SubContainer::SubContainer(std::shared_ptr<zmq::context_t> context, bool to_bind):
        to_bind_(to_bind), context_(std::move(context)) {
    constexpr int kTimeout = 100;
    constexpr int kRecvHwm = 0;

    zmq_subscriber_ = make_unique<zmq::socket_t>(*context_, ZMQ_SUB);
    zmq_subscriber_->setsockopt(ZMQ_RCVTIMEO, &kTimeout, sizeof(kTimeout));
    zmq_subscriber_->setsockopt(ZMQ_RCVHWM, &kRecvHwm, sizeof(kRecvHwm));
    zmq_subscriber_->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
}

SubContainer::~SubContainer() {
    stop();
}

bool SubContainer::bindTcpRaw(const std::string &addr){
    if (!to_bind_ || !tcp_addr_.empty()) return false;
    tcp_addr_ = BindTcp(*zmq_subscriber_, addr);
    return !tcp_addr_.empty();
}

bool SubContainer::bindIpcRaw(){
    if (!to_bind_ || !ipc_addr_.empty()) return false;
    ipc_addr_ = BindIpc(*zmq_subscriber_);
    return !ipc_addr_.empty();
}

bool SubContainer::connectRaw(const std::string &addr){
    try{
        ZmqSyncedOp(*zmq_subscriber_, ZmqOpType::Connect, addr);
    }
    catch (...){
        log.e(TAG, "err connect to %s", addr.c_str());
        return false;
    }
    log.v(TAG, "connected to %s", addr.c_str());
    return true;
}

bool SubContainer::start() {
    // setup internal interrupter
    zmq_interrupter_ = make_unique<zmq::socket_t>(*context_, ZMQ_PUB);
    auto itc_addr = GenItcAddr();
    try{
        zmq_interrupter_->bind(itc_addr);
        zmq_subscriber_->connect(itc_addr);
    }
    catch (...){
        log.e(TAG, "failed to setup interrupter");
        return false;
    }

    // start loop
    loop_t_ = make_unique<thread>([this]{
        ivtb::NameThread("SubContainer");
        log.v(TAG, "loop start");
        while(loop()){}
        log.v(TAG, "loop end");
    });

    return true;
}

void SubContainer::stop() {
    if (loop_t_){
        if (loop_t_->joinable()){
            interrupt();
            loop_t_->join();
        }
    }
}

bool SubContainer::receive(zmq::message_t *msg) {
    bool got_msg = false;
    while (!got_msg){
        try {
            got_msg = zmq_subscriber_->recv(msg);
        }
        catch (...){
            log.e(TAG, "err during recv");
            return false;
        }
    }
    return true;
}

bool SubContainer::getMsg(zmq::message_t &header, zmq::message_t &body) {
    bool res = receive(&header);
    if (!res)
        goto err;
    // assertLog(header->more(),"@" + TAG); // todo assert here occasionally, why? zmq bug?

    res = receive(&body);
    if (!res)
        goto err;
    // assertLog(!body->more(), "@" + TAG); // todo assert here occasionally, why? zmq bug?

    return true;

    err:
    // todo ivan. proper err handling
    log.e(TAG, " exception in zmq socket recv");
    return false;
}

bool SubContainer::loop() {
    zmq::message_t header;
    auto body = make_shared<zmq::message_t>();
    bool res = getMsg(header,*body);
    AssertLog(res, "Err receive msg in SubContainer");

    const msg::MsgHeader *header_fb = msg::GetMsgHeader(header.data());
    if (header_fb->type() == msg::MsgType::MsgType_Interrupt){
        return false;
    }
    // topic message handling
    else if (header_fb->type() == msg::MsgType::MsgType_Message){
        log.v(TAG, "received a msg of topic %s", header_fb->name()->c_str());
        // todo ivan. consider dispatch with daemon thread / multi thread
        lock_guard<mutex> lock(subs_mu_);
        auto itr = subs_.find(header_fb->name()->str());
        if (itr == subs_.end()){
            // happens if a sub just removed
            log.d(TAG, "unknown topic: %s", header_fb->name()->c_str());
            return true;
        }
        shared_ptr<SubscriberBaseImpl::SubMsg> sub_msg;
        if (itr->second.size()==1){
            sub_msg = make_shared<SubscriberBaseImpl::TopicIpcMsgOwned>(
                    move(body), header_fb->protocal()->str() );
            // well, sub_msg could be moved here to enqueue
        }else{
            sub_msg = make_shared<SubscriberBaseImpl::TopicIpcMsg>(
                    move(body), header_fb->protocal()->str() );
        }
        for (auto &sub : itr->second){
            // todo ivan. could use enqueue token. move for last one
            sub->enqueue(sub_msg);
        }
    }
    // srv request
    else if (header_fb->type() == msg::MsgType_ServiceRequest){
        log.v(TAG, "received a srv request of %s", header_fb->name()->c_str());
        // todo ivan. consider dispatch with daemon thread / multi thread
        lock_guard<mutex> lock(subs_mu_);
        auto itr = subs_.find(header_fb->name()->str());
        if (itr == subs_.end()){
            // happens if a sub just removed
            log.d(TAG, "unknown srv: %s", header_fb->name()->c_str());
            return true;
        }
        AssertLog(header_fb->extra_type()==msg::ExtraInfo_ReqInfo, "");
        auto *req_info = header_fb->extra_as_ReqInfo();
        auto sub_msg = make_shared<SubscriberBaseImpl::SrvIpcRequest>(move(body), header_fb->protocal()->str(),
                                                                      req_info->id(), req_info->client_id()->str());
        AssertLog(itr->second.size()==1, "multiple local services");
        // todo ivan. could use enqueue token
        itr->second[0]->enqueue(sub_msg);
    }
    // for srv response, we do all the work here.
    else if (header_fb->type() == msg::MsgType_ServiceResponse){
        lock_guard lock(ClientBaseImpl::wait_list_mu_);
        AssertLog(header_fb->extra_type()==msg::ExtraInfo_RepInfo, "");
        auto *rep_info = header_fb->extra_as_RepInfo();
        auto itr = ClientBaseImpl::wait_list_.find(rep_info->id());
        if (itr == ClientBaseImpl::wait_list_.end()){
            // already timed out
        } else{
            auto rep = make_shared<pair<shared_ptr<Message>, string>>();
            rep->first = move(body);
            rep->second = header_fb->protocal()->str();
            {
                lock_guard result_lock(itr->second->mu);
                itr->second->response = move(rep);
                if (rep_info->status()==0){
                    itr->second->status = SrvStatus::OK;
                } else{
                    itr->second->status = SrvStatus::ServerErr;
                }
            }
            itr->second->cv.notify_one();
            ClientBaseImpl::wait_list_.erase(itr);
        }
    }

    return true;
}

void SubContainer::interrupt() {
    flatbuffers::FlatBufferBuilder header_builder;
    auto header_fb = msg::CreateMsgHeaderDirect(header_builder, msg::MsgType_Interrupt);
    header_builder.Finish(header_fb);

    zmq::message_t header_msg(header_builder.GetBufferPointer(), header_builder.GetSize());
    zmq::message_t body_msg(0);
    bool pub_res = PublisherBaseImpl::send(*zmq_interrupter_, header_msg, true);
    AssertLog(pub_res == 0, "failed to interrupt");
    pub_res = PublisherBaseImpl::send(*zmq_interrupter_, body_msg, false);
    AssertLog(pub_res == 0, "failed to interrupt");
}

bool SubContainer::addSub(unique_ptr<SubscriberBaseImpl> sub_uptr) {
    shared_ptr<SubscriberBaseImpl> sub = move(sub_uptr);
    lock_guard<mutex> lock(subs_mu_);
    string topic = sub->topic_;
    string protocol = sub->protocol_;
    auto itr = MapVecAdd(subs_, topic, move(sub));
    if (itr!=subs_.end()){
        AssertLog(protocol == itr->second[0]->protocol_,
                  "Adding subscriber with different protocol");
        return false;
    }
    else{
        ++version_;
        return true;
    }
}

bool SubContainer::removeSub(SubscriberBaseImpl *sub) {
    shared_ptr<SubscriberBaseImpl> sub_sptr;
    bool topic_removed;
    {
        lock_guard<mutex> lock(subs_mu_);
        // move sub from subs_
        tie(topic_removed, sub_sptr) = MapVecMove(subs_, sub->topic_, sub,
                          [sub](const shared_ptr<SubscriberBaseImpl> &obj){return sub==obj.get();});
        if (topic_removed) ++version_;
    }
    weak_ptr<SubscriberBaseImpl> sub_wptr = sub_sptr;
    // release ownership of tp so tp destroy will not happen after callback in tp threads
    sub_sptr->clearAndReleaseTp();
    // destroy(possible) sub here without lock
    sub_sptr.reset();
    // make sure all ongoing callbacks are finished. This approach is ugly but adds little extra work to sub callback
    while(!sub_wptr.expired()){
        this_thread::sleep_for(1ms);
    }
    return topic_removed;
}

void SubContainer::clearSubs() {
    lock_guard<mutex> lock(subs_mu_);
    subs_.clear();
}

std::vector<SubscriberBaseImpl *> SubContainer::getSubs() {
    lock_guard<mutex> lock(subs_mu_);
    vector<SubscriberBaseImpl *> out;
    out.reserve(2*subs_.size());
    for (const auto& topic_subs : subs_){
        for (const auto& sub : topic_subs.second)
            out.push_back(sub.get());
    }
    return out;
}

}
