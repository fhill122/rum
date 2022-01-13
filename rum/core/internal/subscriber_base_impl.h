//
// Created by Ivan B on 2021/3/28.
//

#ifndef RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_

#include "rum/common/def.h"
#include "rum/common/message.h"
#include "rum/extern/ivtb/thread_pool.h"
#include "rum/extern/zmq/zmq.hpp"
#include "rum/serialization/serializer.h"

namespace rum {

class SubscriberBaseImpl {
  public:
    struct Msg{
        // pointer of itc object or Message
        const std::shared_ptr<const void> msg;
        // if own, zmq_msg could be modified directly without copy, only makes sense when itc=false
        const bool own = false;
        // whether msg is itc object or Message
        const bool itc = false;
        const std::string protocol;

        std::mutex mu;
        std::shared_ptr<const void> deserialized_obj = nullptr;

        Msg(const std::shared_ptr<const void> &msg, bool own, bool itc, std::string protocol="");
    };

  private:
    std::shared_ptr<ivtb::ThreadPool> tp_;
    // todo ivan. consider a lock free queue: https://github.com/cameron314/concurrentqueue
    // std::queue<std::shared_ptr<zmq::message_t>> msg_q_;
    std::queue<std::shared_ptr<Msg>> msg_q_; RUM_LOCK_BY(queue_mu_)
    std::mutex queue_mu_;
    const size_t queue_size_; // <=0 to indicate infinite

    IpcFunc ipc_callback_;
    ItcFunc itc_callback_;
    DeserFunc deserialize_f_;
    const bool single_t_;

    std::function<void()> destr_callback_; RUM_LOCK_BY(destr_mu_)
    std::mutex destr_mu_;
  public:
    const std::string topic_;
    const std::string protocol_;

  private:
  protected:
  public:
    /**
     * Constructor
     * @param topic Topic
     * @param tp Thread pool
     * @param queue_size Size of message queue
     * @param ipc_cb Ipc callback function, note that we allow to modify the message here
     * @param itc_cb Itc callback function
     */
    SubscriberBaseImpl(std::string topic, const std::shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
                       IpcFunc ipc_cb,
                       ItcFunc itc_cb,
                       DeserFunc deserialize_f,
                       std::string protocol = "");

    virtual ~SubscriberBaseImpl();

    /**
     * Enqueue a Msg to the threadpool
     * @param msg Msg to be enqueued
     */
    void enqueue(const std::shared_ptr<Msg> &msg); RUM_THREAD_SAFE

    void setDestrCallback(const std::function<void()> &destr_callback);
};

}

#endif //RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_
