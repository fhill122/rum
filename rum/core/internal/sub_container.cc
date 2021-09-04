//
// Created by Ivan B on 2021/3/24.
//

#include "sub_container.h"

#include <rum/common/common.h>
#include <rum/common/log.h>
#include <rum/common/zmq_helper.h>
#include "rum/core/internal/publisher_base_impl.h"
#include "rum/core/msg/rum_header_generated.h"

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
        zmq_subscriber_->connect(addr);
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
        tu::NameThread("SubContainer");
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
    // todo proper err handling
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
    else if (header_fb->type() == msg::MsgType::MsgType_Message){
        log.v(TAG, "received a msg of topic %s", header_fb->name()->c_str());
        // todo ivan. consider dispatch with daemon thread / multi thread
        lock_guard<mutex> lock(subs_mu_);
        auto itr = subs_.find(header_fb->name()->str());
        if (itr == subs_.end()){
            log.d(TAG, "unknown topic: %s", header_fb->name()->c_str());
            for (const auto &s : subs_){
                log.d(TAG, "topic: %s", s.first.c_str());
            }
            return true;
        }
        for (auto &sub : itr->second){
            // todo ivan. this introduced another delay source
            sub->enqueue({body, itr->second.size()==1, false});
        }
        // todo ivan. if it is a service call reply, should we just notify here?
    }

    return true;
}

void SubContainer::interrupt() {
    flatbuffers::FlatBufferBuilder header_builder;
    auto header_fb = msg::CreateMsgHeaderDirect(header_builder, msg::MsgType_Interrupt, "", "");
    header_builder.Finish(header_fb);

    zmq::message_t header_msg(header_builder.GetBufferPointer(), header_builder.GetSize());
    zmq::message_t body_msg(0);
    bool pub_res = PublisherBaseImpl::send(*zmq_interrupter_, header_msg, true);
    AssertLog(pub_res == 0, "failed to interrupt");
    pub_res = PublisherBaseImpl::send(*zmq_interrupter_, body_msg, false);
    AssertLog(pub_res == 0, "failed to interrupt");
}

bool SubContainer::addSub(SubscriberBaseImpl *sub) {
    lock_guard<mutex> lock(subs_mu_);
    auto itr = subs_.find(sub->topic_);
    if (itr == subs_.end()) {
        subs_.emplace(sub->topic_, vector<SubscriberBaseImpl *>{sub});
        return true;
    } else {
        assert(!itr->second.empty());
        assert(find(itr->second.begin(), itr->second.end(), sub) ==  itr->second.end());
        AssertLog(sub->protocol_ == itr->second[0]->protocol_,
                  "Adding subscriber of " + sub->protocol_ + " with different protocol");
        itr->second.push_back(sub);
        return false;
    }
}

bool SubContainer::removeSub(SubscriberBaseImpl *sub) {
    lock_guard<mutex> lock(subs_mu_);
    auto itr = subs_.find(sub->topic_);
    assert(itr!=subs_.end());
    auto itr2 = find(itr->second.begin(), itr->second.end(), sub);
    assert( itr2 != itr->second.end() );

    if (itr->second.size()==1){
        subs_.erase(itr);
        return true;
    }
    else{
        itr->second.erase(itr2);
        return false;
    }
}

}