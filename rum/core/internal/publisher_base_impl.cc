//
// Created by Ivan B on 2021/3/24.
//

#include "publisher_base_impl.h"

#include "../msg/rum_header_generated.h"
#include <rum/common/common.h>
#include <rum/common/log.h>
#include <rum/common/zmq_helper.h>
#include "itc_manager.h"

using namespace std;

#define TAG (topic_+"_pub")

namespace rum {

rum::PublisherBaseImpl::PublisherBaseImpl(string topic, std::string protocol,
                                          shared_ptr<zmq::context_t> context, bool to_bind):
        topic_(move(topic)), protocol_(move(protocol)), to_bind_(to_bind), context_(move(context)){
    constexpr int kHWM = 0;

    zmq_publisher_ = make_unique<zmq::socket_t>(*context_, ZMQ_PUB);
    zmq_publisher_->setsockopt(ZMQ_SNDHWM, &kHWM, sizeof(kHWM));

    flatbuffers::FlatBufferBuilder header_builder;
    auto header_fb = msg::CreateMsgHeaderDirect(header_builder,
            msg::MsgType_Message, topic_.c_str(), protocol_.c_str());
    header_builder.Finish(header_fb);
    // todo ivan. can we do this with null deleter? how to avoid memory leak once publisher is destroyed?
    msg_header_.rebuild(header_builder.GetBufferPointer(), header_builder.GetSize());
}

PublisherBaseImpl::~PublisherBaseImpl() {
    lock_guard<mutex> lock(destr_mu_);
    if (destr_callback_) destr_callback_();
}

bool PublisherBaseImpl::bindTcpRaw(const std::string &addr) {
    if (!to_bind_ || !tcp_addr_.empty()) return false;
    tcp_addr_ = BindTcp(*zmq_publisher_, addr);
    return !tcp_addr_.empty();
}

bool PublisherBaseImpl::bindIpcRaw(){
    if (!to_bind_ || !ipc_addr_.empty()) return false;
    ipc_addr_ = BindIpc(*zmq_publisher_);
    return !ipc_addr_.empty();
}

bool PublisherBaseImpl::connect(const string &addr) {
    // note: conn_list_ is not synced
    if (conn_list_.find(addr)==conn_list_.end()) {
        try {
            lock_guard<mutex> lock(zmq_mu_);
            zmq_publisher_->connect(addr);
            log.v(TAG, "connected to %s", addr.c_str());
        }
        catch (...) {
            log.e(TAG, "err connect to %s", addr.c_str());
            return false;
        }
        conn_list_.insert(addr);
    }
    return true;
}

bool PublisherBaseImpl::disconnect(const string &addr) {
    // note: conn_list_ is not synced
    auto itr = conn_list_.find(addr);
    if (itr != conn_list_.end()) {
        try {
            lock_guard<mutex> lock(zmq_mu_);
            zmq_publisher_->disconnect(addr);
        }
        catch (...) {
            log.e(TAG, "err disconnect %s", addr.c_str());
            return false;
        }
        conn_list_.erase(itr);
    }
    return true;
}

bool PublisherBaseImpl::isConnected() {
    // there is no easy way to know if address bound
    if (to_bind_) return true;

    // Is it ok to not lock here?
    // lock_guard<mutex> lock(zmq_mu_);
    return !conn_list_.empty();
}

// void PublisherBaseImpl::addItcSub(SubscriberBaseImpl* sub_wp){
//     lock_guard<mutex> lock(itc_mu_);
//     itc_subs_.push_back(sub_wp);
// }

bool PublisherBaseImpl::publishIpc(zmq::message_t &header, zmq::message_t &body) {
    lock_guard<mutex> lock(zmq_mu_);
    if (send(*zmq_publisher_, header, true) == 0){
        if (send(*zmq_publisher_, body, false) == 0)
            return true;
    }
    AssertLog(false, "Failed to publishIpc");
    return false;
}

bool PublisherBaseImpl::publishIpc(zmq::message_t &body){
    zmq::message_t header(msg_header_.data(), msg_header_.size());
    return publishIpc(header, body);
}

bool PublisherBaseImpl::scheduleItc(const shared_ptr<const void> &msg) {
    return ItcManager::GlobalManager().scheduleItc(topic_, msg);
}

int PublisherBaseImpl::send(zmq::socket_t &socket, zmq::message_t &message, bool wait) {
    zmq::detail::send_result_t  result;
    try {
        result = socket.send(message,
                             wait? zmq::send_flags::sndmore : zmq::send_flags::none);
    }
    catch (...){
        goto err;
    }

    if (!result.has_value())
        goto err;

    return 0;

    err:
    log.e(__FUNCTION__, "failed to publishIpc");
    // AssertLog(false, "failed to publishIpc");
    return -1;
}

void PublisherBaseImpl::setDestrCallback(const function<void()> &destr_callback) {
    lock_guard<mutex> lock(destr_mu_);
    destr_callback_ = destr_callback;
}

}
