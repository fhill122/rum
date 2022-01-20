/*
 * Created by Ivan B on 2022/1/10.
 */

#ifndef RUM_CPPRUM_RUM_H_
#define RUM_CPPRUM_RUM_H_

#include "rum/core/node_base.h"
#include "rum/serialization/flatbuffers/serializer_fbs.h"
#include "publisher.h"
#include "subscriber.h"

namespace rum {

static inline bool Init(const NodeParam &param = NodeParam()) {
    return NodeBase::Init(param);
}

template<class MsgT, class SubSerializerT>
[[nodiscard]] std::unique_ptr<Subscriber> CreateSubscriber(
        const std::string &topic,
        const SubFunc<MsgT> &callback_f,
        size_t queue_size = 1000,
        const std::shared_ptr<ThreadPool> &tp = std::make_shared<ThreadPool>(1));

template <class MsgT, class PubSerializerT>
[[nodiscard]] std::unique_ptr<Publisher<MsgT>> CreatePublisher(const std::string &topic);

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class MsgT, class SubSerializerT>
Subscriber::UniquePtr CreateSubscriber(
        const std::string &topic, const SubFunc<MsgT> &callback_f,
        size_t queue_size, const std::shared_ptr<ThreadPool> &tp) {
    Serializer<SubSerializerT> serializer;

    return std::make_unique<Subscriber>(
        NodeBase::GlobalNode()->addSubscriber(
            topic, tp, queue_size,
            serializer.generateIpcCallback(callback_f),
            serializer.generateItcCallback(callback_f),
            [serializer](std::shared_ptr<const Message> &msg, const std::string& protocol){
                return serializer.template deserialize<MsgT>(msg, protocol);
            },
            SubSerializerT::Protocol()
        ));
}

template<class MsgT, class PubSerializerT>
std::unique_ptr<Publisher<MsgT>> CreatePublisher(const std::string &topic) {
    Serializer<PubSerializerT> serializer;

    return std::make_unique<Publisher<MsgT>>(
            NodeBase::GlobalNode()->addPublisher(topic, PubSerializerT::Protocol()),
            [serializer](const std::shared_ptr<const MsgT>&obj){
                return serializer.template serialize<MsgT>(obj);} );
}

}
#endif //RUM_CPPRUM_RUM_H_
