//
// Created by Ivan B on 2021/3/24.
//

#include "publisher_base_impl.h"

#include <rum/common/common.h>
#include <rum/common/log.h>
#include <rum/common/zmq_helper.h>
#include "itc_manager.h"

#define TAG (topic_+"_pub")

using namespace std;

namespace rum {

rum::PublisherBaseImpl::PublisherBaseImpl(string topic, std::string protocol,
                                          shared_ptr<zmq::context_t> context, bool to_bind, msg::MsgType msg_type):
        topic_(move(topic)), protocol_(move(protocol)), to_bind_(to_bind), context_(move(context)),
        msg_type_(msg_type){
    constexpr int kHWM = 0;

    zmq_publisher_ = make_unique<zmq::socket_t>(*context_, ZMQ_PUB);
    zmq_publisher_->setsockopt(ZMQ_SNDHWM, &kHWM, sizeof(kHWM));

    if (msg_type_ == msg::MsgType_Message){
        flatbuffers::FlatBufferBuilder header_builder;
        auto header_fb = msg::CreateMsgHeaderDirect(header_builder,
                                                    msg_type_, topic_.c_str(), protocol_.c_str());
        header_builder.Finish(header_fb);
        // todo ivan. we can do this with null deleter. but how to avoid memory leak once publisher is destroyed? linger time force to zero?
        topic_header_.rebuild(header_builder.GetBufferPointer(), header_builder.GetSize());
    }
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
            ZmqSyncedOp(*zmq_publisher_, ZmqOpType::Connect, addr);
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
            ZmqSyncedOp(*zmq_publisher_, ZmqOpType::Disconnect, addr);
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
    AssertLog(msg_type_ == msg::MsgType_Message, "");
    zmq::message_t header(topic_header_.data(), topic_header_.size());
    return publishIpc(header, body);
}

bool PublisherBaseImpl::publishReqIpc(unsigned int id, zmq::message_t &body) {
    AssertLog(msg_type_ == msg::MsgType_ServiceRequest, "");
    AssertLog(!cli_id_.empty(), "");
    auto* header_builder = new flatbuffers::FlatBufferBuilder();
    auto req_info_fb = msg::CreateReqInfoDirect(*header_builder, cli_id_.c_str(), id);
    auto header_fb = msg::CreateMsgHeaderDirect(*header_builder, msg::MsgType_ServiceRequest,
                            topic_.c_str(), protocol_.c_str(), msg::ExtraInfo_ReqInfo, req_info_fb.Union());
    header_builder->Finish(header_fb);
    zmq::message_t msg_header_(header_builder->GetBufferPointer(), header_builder->GetSize(),
        [](void *, void* builder){
            delete (flatbuffers::FlatBufferBuilder*)builder;
        }, header_builder);

    return publishIpc(msg_header_, body);
}

bool PublisherBaseImpl::publishRepIpc(unsigned int id, char status, zmq::message_t &body) {
    AssertLog(msg_type_ == msg::MsgType_ServiceResponse, "");
    auto* header_builder = new flatbuffers::FlatBufferBuilder();
    auto rep_info_fb = msg::CreateRepInfo(*header_builder, status, id);
    auto header_fb = msg::CreateMsgHeaderDirect(*header_builder, msg::MsgType_ServiceResponse,
                            topic_.c_str(), protocol_.c_str(), msg::ExtraInfo_RepInfo, rep_info_fb.Union());

    header_builder->Finish(header_fb);
    zmq::message_t msg_header_(header_builder->GetBufferPointer(), header_builder->GetSize(),
                               [](void *, void* builder){
                                   delete (flatbuffers::FlatBufferBuilder*)builder;
                               }, header_builder);

    return publishIpc(msg_header_, body);
}

bool PublisherBaseImpl::scheduleItc(const shared_ptr<const void> &msg) {
    return ItcManager::GlobalManager()->scheduleItc(topic_, msg);
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

#undef TAG
