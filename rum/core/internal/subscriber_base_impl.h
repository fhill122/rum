//
// Created by Ivan B on 2021/3/28.
//

#ifndef RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_

#include <utility>

#include "rum/common/def.h"
#include "rum/common/message.h"
#include "rum/common/thread_pool.h"
#include "rum/extern/zmq/zmq.hpp"
#include "rum/serialization/serializer.h"
#include "../msg/rum_header_generated.h"
#include "awaiting_result.h"

namespace rum {

class SubscriberBaseImpl : public std::enable_shared_from_this<SubscriberBaseImpl>{
  public:
    struct SubMsg{
        virtual void processSelf(SubscriberBaseImpl* sub) = 0;
    };

    struct TopicItcMsg : public SubMsg{
        const std::shared_ptr<const void> msg;

        explicit TopicItcMsg(std::shared_ptr<const void> msg) : msg(std::move(msg)) {}
        void processSelf(SubscriberBaseImpl* sub) override {sub->itc_callback_(msg);}
    };

    struct TopicIpcMsg : public SubMsg{
        std::shared_ptr<const Message> msg;
        const std::string protocol;
        std::mutex deserialize_mu;
        std::shared_ptr<const void> deserialized_obj = nullptr;

        TopicIpcMsg(std::shared_ptr<const Message> &&msg, std::string &&protocol)
                : msg(std::move(msg)), protocol(std::move(protocol)) {}
        void processSelf(SubscriberBaseImpl *sub) override;
    };

    struct TopicIpcMsgOwned : public SubMsg{
        std::shared_ptr<const Message> msg;
        const std::string protocol;

        TopicIpcMsgOwned(std::shared_ptr<const Message> &&msg, std::string &&protocol)
                : msg(std::move(msg)), protocol(std::move(protocol)) {}
        void processSelf(SubscriberBaseImpl *sub) override;
    };

    using SrvItcRequest = TopicItcMsg;

    struct SrvIpcRequest : public SubMsg{
        struct Content{
            std::shared_ptr<Message> request = nullptr;
            std::string protocol;
            unsigned int id;
            std::string pub_topic;
            Content(const std::shared_ptr<Message> &request,const std::string &a_protocol,
                    unsigned int id,const std::string &pub_topic)
                    : request(request), protocol(a_protocol), id(id), pub_topic(pub_topic) {}
        };

        std::shared_ptr<const Content> msg;

        SrvIpcRequest(const std::shared_ptr<Message> &request,
                      const std::string &protocol,
                      unsigned int id,
                      const std::string &pub_topic)
                : msg(std::make_shared<Content>(request, protocol, id, pub_topic)){}
        void processSelf(SubscriberBaseImpl *sub) override {
            // AssertLog(sub->msg_type_==msg::MsgType_ServiceRequest, "");
            sub->ipc_callback_(msg);
        }
    };

  private:
    std::shared_ptr<ThreadPool> tp_;
    // todo ivan. consider a lock free queue: https://github.com/cameron314/concurrentqueue
    // std::queue<std::shared_ptr<zmq::message_t>> msg_q_;
    std::queue<std::shared_ptr<SubMsg>> msg_q_;  RUM_LOCK_BY(queue_mu_)
    std::mutex queue_mu_;
    const size_t queue_size_; // <=0 to indicate infinite

    IpcFunc ipc_callback_;
    ItcFunc itc_callback_;
    DeserFunc deserialize_f_;
    const bool single_t_;

    std::function<void()> destr_callback_;  RUM_LOCK_BY(destr_mu_)
    std::mutex destr_mu_;
  public:
    const std::string topic_;
    const std::string protocol_;
    const msg::MsgType msg_type_;

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
    SubscriberBaseImpl(std::string topic, const std::shared_ptr<ThreadPool> &tp, size_t queue_size,
                       IpcFunc ipc_cb,
                       ItcFunc itc_cb,
                       DeserFunc deserialize_f,
                       std::string protocol = "",
                       msg::MsgType msg_type = msg::MsgType::MsgType_Message);

    virtual ~SubscriberBaseImpl();

    /**
     * Enqueue a Msg to the threadpool
     * @param msg Msg to be enqueued
     */
    void enqueue(const std::shared_ptr<SubMsg> &msg); RUM_THREAD_SAFE

    void clearAndReleaseTp();  RUM_THREAD_SAFE

    void setDestrCallback(const std::function<void()> &destr_callback);
};

}

#endif //RUM_CORE_INTERNAL_SUBSCRIBER_BASE_IMPL_H_
