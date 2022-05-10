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

void SubscriberBaseImpl::TopicInterProcMsg::processSelf(SubscriberBaseImpl *sub) {
    {
        lock_guard lock(deserialize_mu);
        if (!deserialized_obj){
            deserialized_obj = sub->deserialize_f_(msg, protocol);
        }
    }

    if (deserialized_obj){
        sub->inter_proc_callback_(deserialized_obj);
    } else{
        log.e(sub->topic_+"_sub", "failed to deserialize");
    }
}

void SubscriberBaseImpl::TopicInterProcMsgOwned::processSelf(SubscriberBaseImpl *sub) {
    auto deserialized_obj = sub->deserialize_f_(msg, protocol);
    if (deserialized_obj){
        sub->inter_proc_callback_(deserialized_obj);
    } else{
        log.e(sub->topic_+"_sub", "failed to deserialize");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

SubscriberBaseImpl::SubscriberBaseImpl(std::string topic,
                                       const std::shared_ptr<ThreadPool> &tp, const size_t queue_size,
                                       InterProcFunc inter_cb,
                                       IntraProcFunc intra_cb,
                                       DeserFunc<> deserialize_f,
                                       string protocol,
                                       msg::MsgType msg_type)
        : topic_(move(topic)), protocol_(move(protocol)), tp_(tp), queue_size_(queue_size),
          inter_proc_callback_(move(inter_cb)), intra_proc_callback_(move(intra_cb)), deserialize_f_(move(deserialize_f)),
          single_t_(tp->threads()==1){}

SubscriberBaseImpl::~SubscriberBaseImpl() {
    lock_guard<mutex> lock(destr_mu_);
    if (destr_callback_) destr_callback_();
}

void SubscriberBaseImpl::enqueue(const shared_ptr<SubMsg> &msg) {
    AssertLog(msg, "");

#ifdef RUM_USE_MOODYCAMEL_Q
    msg_q_.enqueue(msg);
    if(queue_size_>0 && msg_q_.size_approx()>queue_size_){
        shared_ptr<SubMsg> drop;
        msg_q_.try_dequeue(drop);
    }
#else
    {
        // lock as could be scheduled from any threads
        lock_guard lock(queue_mu_);
        msg_q_.push(msg);
        if (queue_size_>0 && msg_q_.size()>queue_size_){
            // todo ivan. drop action: for srv, it should quickly inform clients
            msg_q_.pop();
        }
    }
#endif

    tp_->enqueue([weak_this = weak_from_this()]() mutable {
        auto sub = weak_this.lock();
        if (!sub) return ;

        shared_ptr<SubMsg> front_msg;
#ifdef RUM_USE_MOODYCAMEL_Q
        bool dequeued = sub->msg_q_.try_dequeue(front_msg);
        if(dequeued){
            front_msg->processSelf(sub.get());
        }
#else
        {
            // have to lock even single thread tp_ as push and overflow pop happening in other threads
            lock_guard lock(sub->queue_mu_);
            if(sub->msg_q_.empty()) return;
            front_msg = move(sub->msg_q_.front());
            sub->msg_q_.pop();
        }

        front_msg->processSelf(sub.get());
#endif
    });
}

void SubscriberBaseImpl::clearAndReleaseTp(){
#ifdef RUM_USE_MOODYCAMEL_Q
    auto remain_size = msg_q_.size_approx();
    vector<shared_ptr<SubMsg>> deq_vec(remain_size);
    msg_q_.try_dequeue_bulk(deq_vec.begin(), remain_size);
#else
    {
        lock_guard lock(queue_mu_);
        msg_q_ = decltype(msg_q_)();
    }
#endif
    // we have to make sure after this point, no enqueue would happen
    tp_.reset();
}

void SubscriberBaseImpl::setDestrCallback(const function<void()> &destr_callback) {
    lock_guard<mutex> lock(destr_mu_);
    destr_callback_ = destr_callback;
}

}

#undef TAG