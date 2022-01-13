/*
 * Created by Ivan B on 2022/1/10.
 */

#ifndef RUM_CPPRUM_RUM_H_
#define RUM_CPPRUM_RUM_H_

#include "rum/core/node_base.h"
#include "rum/serialization/flatbuffers/serializer_fbs.h"
#include "publisher_handler.h"
#include "subscriber_handler.h"

namespace rum {

static inline void Init(const NodeParam &param = NodeParam()) {
    NodeBase::Init(param);
}

template<class MsgT, class SubSerializerT>
SubscriberHandler<SubSerializerT> AddSubscriber(
        const std::string &topic,
        const SubFunc<MsgT> &callback_f,
        size_t queue_size = 1000,
        const std::shared_ptr<ThreadPool> &tp = std::make_shared<ThreadPool>(1));

template <class MsgT, class PubSerializerT>
PublisherHandler<MsgT, PubSerializerT> AddPublisher(const std::string &topic);

template<class MsgT, class SubSerializerT>
void RemoveSubscriber(SubscriberHandler<SubSerializerT> &sub_handler);

template <class MsgT, class PubSerializerT>
void RemovePublisher(PublisherHandler<PubSerializerT, MsgT> &pub_handler);

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class MsgT, class SubSerializerT>
SubscriberHandler<SubSerializerT> AddSubscriber(
        const std::string &topic, const SubFunc<MsgT> &callback_f,
        size_t queue_size, const std::shared_ptr<ThreadPool> &tp) {

    return SubscriberHandler<SubSerializerT>(
        NodeBase::GlobalNode()->addSubscriber(
            topic, tp, queue_size,
            SubscriberHandler<SubSerializerT>::s_.generateIpcCallback(callback_f),
            SubscriberHandler<SubSerializerT>::s_.generateItcCallback(callback_f),
            // nullptr,
            [](std::shared_ptr<const Message> &msg, const std::string& protocol){
                return SubscriberHandler<SubSerializerT>::s_.template deserialize<MsgT>(msg, protocol);
            },
            SubSerializerT::Protocol()
        ));
}

template<class MsgT, class PubSerializerT>
PublisherHandler<MsgT, PubSerializerT> AddPublisher(const std::string &topic) {
    return PublisherHandler<MsgT, PubSerializerT>(
            NodeBase::GlobalNode()->addPublisher(topic, PubSerializerT::Protocol()) );
}

template<class SubSerializerT>
void RemoveSubscriber(SubscriberHandler<SubSerializerT> &sub_handler){
    NodeBase::GlobalNode()->removeSubscriber(sub_handler);
}

template <class MsgT, class PubSerializerT>
void RemovePublisher(PublisherHandler<PubSerializerT, MsgT> &pub_handler){
    NodeBase::GlobalNode()->removePublisher(pub_handler);
}

}
#endif //RUM_CPPRUM_RUM_H_
