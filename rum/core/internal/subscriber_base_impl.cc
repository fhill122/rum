//
// Created by Ivan B on 2021/3/28.
//

#include "subscriber_base_impl.h"

#include "rum/extern/ivtb/optional_lock.h"
#include "rum/common/log.h"
#include "rum/common/buffer.h"

#include <utility>

#define TAG (topic_+"_sub")

using namespace std;

namespace rum{


SubscriberBaseImpl::Msg::Msg(const shared_ptr<const void> &msg, bool own, bool itc, string protocol)
        : msg(msg), own(own), itc(itc), protocol(move(protocol)) {}

// SubscriberBaseImpl::Msg::Msg(const SubscriberBaseImpl::Msg &msg) {}

SubscriberBaseImpl::SubscriberBaseImpl(std::string topic,
        const std::shared_ptr<ivtb::ThreadPool> &tp, const size_t queue_size,
        IpcFunc ipc_cb,
        ItcFunc itc_cb,
        DeserFunc deserialize_f,
        string protocol)
        : topic_(move(topic)), protocol_(move(protocol)), tp_(tp), queue_size_(queue_size),
          ipc_callback_(move(ipc_cb)), itc_callback_(move(itc_cb)), deserialize_f_(move(deserialize_f)),
          single_t_(tp->threads()==1){
    assert(tp->threads()>0);
}

SubscriberBaseImpl::~SubscriberBaseImpl() {
    lock_guard<mutex> lock(destr_mu_);
    if (destr_callback_) destr_callback_();
}

void SubscriberBaseImpl::enqueue(const shared_ptr<Msg> &msg) {
    log.v(TAG, "to enqueue a msg");
    AssertLog(msg, "");

    {
        // lock as could be scheduled from any threads
        lock_guard lock(queue_mu_);
        msg_q_.push(msg);
        if (queue_size_>0 && msg_q_.size()>queue_size_){
            msg_q_.pop();
        }
    }

    tp_->enqueue([this](){
        shared_ptr<Msg> msg;
        {
            ivtb::OptionalLock lock(queue_mu_, true);
            if (msg_q_.empty()) return;
            // log.w(TAG, "q size %d", msg_q_.size());
            msg = move(msg_q_.front());
            msg_q_.pop();
        }


        if (msg->itc){
            itc_callback_(msg->msg);
        }
        else {
            if (!msg->own) {
                lock_guard lock(msg->mu);
                if (!msg->deserialized_obj){
                    shared_ptr<const Message> message_ptr = static_pointer_cast<const Message>(msg->msg);
                    msg->deserialized_obj = deserialize_f_(message_ptr, msg->protocol);
                }
            }
            else{
                shared_ptr<const Message> message_ptr = static_pointer_cast<const Message>(msg->msg);
                msg->deserialized_obj = deserialize_f_(message_ptr, msg->protocol);
            }

            if (msg->deserialized_obj){
                ipc_callback_(msg->deserialized_obj);
            } else{
                log.e(TAG, "failed to deserialize");
            }
        }
    });
}

void SubscriberBaseImpl::setDestrCallback(const function<void()> &destr_callback) {
    lock_guard<mutex> lock(destr_mu_);
    destr_callback_ = destr_callback;
}

}

#undef TAG