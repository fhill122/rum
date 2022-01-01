//
// Created by Ivan B on 2021/3/28.
//

#ifndef RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_

#include <rum/common/def.h>
#include <rum/extern/ivtb/thread_pool.h>
#include <rum/extern/zmq/zmq.hpp>

namespace rum {

class SubscriberBaseImpl {
  public:
    struct Msg{
        std::shared_ptr<const void> msg;
        // if own, zmq_msg could be modified directly without copy
        bool own = false;
        bool itc = false;

        // todo ivan. do deserialization here
    };

  private:
    std::shared_ptr<ivtb::ThreadPool> tp_;
    // todo ivan. consider a lock free queue: https://github.com/cameron314/concurrentqueue
    // std::queue<std::shared_ptr<zmq::message_t>> msg_q_;
    std::queue<Msg> msg_q_; RUM_LOCK_BY(queue_mu_)
    std::mutex queue_mu_;
    const size_t queue_size_; // <=0 to indicate infinite

    // todo ivan. instead takes a serialization function and a callback function
    std::function<void(zmq::message_t&)> ipc_callback_;
    std::function<void(const void *)> itc_callback_;
    std::function<void*(zmq::message_t&)> deserialize_f_;
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
    SubscriberBaseImpl(std::string topic, const std::shared_ptr<ivtb::ThreadPool> &tp,
                       size_t queue_size, std::function<void(zmq::message_t&)> ipc_cb,
                       std::function<void(const void *)> itc_cb, std::string protocol = "");

    virtual ~SubscriberBaseImpl();

    /**
     * Enqueue a Msg to the threadpool
     * @param msg Msg to be enqueued
     */
    void enqueue(const Msg &msg); RUM_THREAD_SAFE

    void enqueu(const std::shared_ptr<const void> &msg);

    void setDestrCallback(const std::function<void()> &destr_callback);
};

}

#endif //RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_
