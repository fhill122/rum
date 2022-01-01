//
// Created by Ivan B on 2021/3/28.
//

#include "subscriber_base_impl.h"

#include <rum/common/optional_lock.h>
#include <rum/common/log.h>
#include <rum/common/buffer.h>

#include <utility>

using namespace std;

namespace rum{

#define TAG topic_+"_sub"

SubscriberBaseImpl::SubscriberBaseImpl(std::string topic,
        const std::shared_ptr<ivtb::ThreadPool> &tp, const size_t queue_size,
        function<void(zmq::message_t&)> ipc_cb, function<void(const void *)> itc_cb,
        std::string protocol)
        : topic_(move(topic)), protocol_(move(protocol)), tp_(tp), queue_size_(queue_size),
          ipc_callback_(move(ipc_cb)), itc_callback_(move(itc_cb)), single_t_(tp->threads()==1){
    assert(tp->threads()>0);
}

SubscriberBaseImpl::~SubscriberBaseImpl() {
    lock_guard<mutex> lock(destr_mu_);
    if (destr_callback_) destr_callback_();
}

void SubscriberBaseImpl::enqueue(const Msg &msg) {
    log.v(TAG, "to enqueue a msg");
    {
        lock_guard<mutex> lock(queue_mu_);
        msg_q_.push(msg);
        if (queue_size_>0 && msg_q_.size()>queue_size_){
            msg_q_.pop();
        }
    }

    tp_->enqueue([this](){
        Msg msg;
        {
            lock_guard<mutex> lock(queue_mu_);
            if (msg_q_.empty()) return;
            msg = move(msg_q_.front());
            msg_q_.pop();
        }

        // todo ivan. should only have 1 callback, and 1 deserialization function,
        //  this also provides easy binding
        if (msg.itc){
            itc_callback_(msg.msg.get());
        }
        else {
            // copy data if not own it(multiple subs).
            // why?
            // serialization may not take a const, but that could be handled(copy) in serialization.
            // zmq message could be directly sent without copy.
            // todo ivan. think more about this design.
            //  at least we don't need to copy every time.
            //  also could share the deserialization
            if (!msg.own) {
                auto msg_p = (zmq::message_t*)msg.msg.get();
                zmq::message_t zmq_msg( msg_p->data(), msg_p->size() );
                ipc_callback_(zmq_msg);
            }
            else{
                ipc_callback_(*(zmq::message_t*)msg.msg.get());
            }
        }
    });
}

void SubscriberBaseImpl::setDestrCallback(const function<void()> &destr_callback) {
    lock_guard<mutex> lock(destr_mu_);
    destr_callback_ = destr_callback;
}

}
