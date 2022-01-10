//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_NODE_H_
#define RUM_RUM_NODE_H_

#include "rum/core/node_base.h"
#include "rum/serialization/flatbuffers/serializer_fbs.h"
#include "publisher_handler.h"
#include "subscriber_handler.h"

namespace rum {

template<class SerializerT = SerializerFbs>
class Node{
  public:

    static void Init(const NodeParam &param = NodeParam()){
        NodeBase::Init(param);
    }

    static std::unique_ptr<Node>& GlobalNode(){
        return NodeBase::GlobalNode();
    };

    template <class MsgT, class SubSerializerT = SerializerT>
    SubscriberHandler<SubSerializerT, MsgT> AddSubscriber(
                            const std::string &topic,
                            const std::function<void(const MsgT&)> &callback_f,
                            size_t queue_size = 1000,
                            const std::shared_ptr<ThreadPool> &tp = std::make_shared<ThreadPool>(1));

    template <class MsgT, class PubSerializerT = SerializerT>
    PublisherHandler<PubSerializerT, MsgT> AddPublisher(const std::string &topic);
};


///////////////////////////////////////////////////////////////////////////////////////////////////

template<class SerializerT>
template<class MsgT, class SubSerializerT>
SubscriberHandler<SubSerializerT, MsgT> Node<SerializerT>::AddSubscriber(
        const std::string &topic, const std::function<void(const MsgT&)> &callback_f,
        size_t queue_size, const std::shared_ptr<ThreadPool> &tp) {

    return NodeBase::GlobalNode()->addSubscriber(
            topic, tp, queue_size,
            SubscriberHandler<SubSerializerT, MsgT>::GenerateIpcCb(callback_f),
            SubscriberHandler<SubSerializerT, MsgT>::GenerateItcCb(callback_f),
            SubSerializerT::protocol());
}

// template<class SerializerT>
// template<class MsgT, class PubSerializerT>
// PublisherHandler<PubSerializerT, MsgT> Node<SerializerT>::AddPublisher(const std::string &topic) {
//     return PublisherHandler<PubSerializerT, MsgT>(NodeBase::addPublisher(topic, PubSerializerT::protocol()));
// }

}
#endif //RUM_RUM_NODE_H_
