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
#include "srv_common.h"

// #define RUM_USE_MOODYCAMEL_Q
#ifdef RUM_USE_MOODYCAMEL_Q
#include "rum/extern/concurrentqueue/concurrentqueue.h"
#endif

namespace rum {

class SubscriberBaseImpl : public std::enable_shared_from_this<SubscriberBaseImpl>{
  public:
    struct SubMsg{
        virtual void processSelf(SubscriberBaseImpl* sub) = 0;
    };

    struct TopicIntraProcMsg : public SubMsg{
        const std::shared_ptr<const void> msg;

        explicit TopicIntraProcMsg(std::shared_ptr<const void> msg) : msg(std::move(msg)) {}
        void processSelf(SubscriberBaseImpl* sub) override {sub->intra_proc_callback_(msg);}
    };

    struct TopicInterProcMsg : public SubMsg{
        std::shared_ptr<const Message> msg;
        const std::string protocol;
        std::mutex deserialize_mu;
        std::shared_ptr<const void> deserialized_obj = nullptr;

        TopicInterProcMsg(std::shared_ptr<const Message> &&msg, std::string &&protocol)
                : msg(std::move(msg)), protocol(std::move(protocol)) {}
        void processSelf(SubscriberBaseImpl *sub) override;
    };

    struct TopicInterProcMsgOwned : public SubMsg{
        std::shared_ptr<const Message> msg;
        const std::string protocol;

        TopicInterProcMsgOwned(std::shared_ptr<const Message> &&msg, std::string &&protocol)
                : msg(std::move(msg)), protocol(std::move(protocol)) {}
        void processSelf(SubscriberBaseImpl *sub) override;
    };

    using SrvIntraProcRequest = TopicIntraProcMsg;

    struct SrvIterProcRequest : public SubMsg{
        struct Content{
            std::shared_ptr<Message> request = nullptr;
            std::string protocol;
            unsigned int id;
            std::string client_id;
            Content(const std::shared_ptr<Message> &request,const std::string &a_protocol,
                    unsigned int id,const std::string &client_id)
                    : request(request), protocol(a_protocol), id(id), client_id(client_id) {}
        };

        std::shared_ptr<const Content> msg;

        SrvIterProcRequest(const std::shared_ptr<Message> &request,
                           const std::string &protocol,
                           unsigned int id,
                           const std::string &pub_topic)
                : msg(std::make_shared<Content>(request, protocol, id, pub_topic)){}
        void processSelf(SubscriberBaseImpl *sub) override {
            // AssertLog(sub->msg_type_==msg::MsgType_ServiceRequest, "");
            sub->inter_proc_callback_(msg);
        }
    };

  private:
    std::shared_ptr<ThreadPool> tp_;
#ifdef RUM_USE_MOODYCAMEL_Q
    moodycamel::ConcurrentQueue<std::shared_ptr<SubMsg>> msg_q_;
#else
    std::queue<std::shared_ptr<SubMsg>> msg_q_;  RUM_LOCK_BY(queue_mu_)
    std::mutex queue_mu_;
#endif
    const size_t queue_size_; // <=0 to indicate infinite

    InterProcFunc inter_proc_callback_;
    IntraProcFunc intra_proc_callback_;
    DeserFunc<> deserialize_f_;
    const bool single_t_;

    std::function<void()> destr_callback_;  RUM_LOCK_BY(destr_mu_)
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
     * @param inter_cb Inter-proc callback function, note that we allow to modify the message here
     * @param intra_cb Intra-proc callback function
     */
    SubscriberBaseImpl(std::string topic, const std::shared_ptr<ThreadPool> &tp, size_t queue_size,
                       InterProcFunc inter_cb,
                       IntraProcFunc intra_cb,
                       DeserFunc<> deserialize_f,
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
