//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_NODE_H_
#define RUM_RUM_NODE_H_

#include <rum/core/node_base.h>
#include "publisher.h"
#include "subscriber.h"

namespace rum {

template<class SerializerT>
class Node : public NodeBase{

  private:
  public:

  protected:
    explicit Node(std::string name) : NodeBase(std::move(name)){}
  public:
    static std::unique_ptr<Node<SerializerT>> init(
        std::string name="", Serializer<SerializerT> s = Serializer<SerializerT>());

    template <class MsgT, class SubSerializerT = SerializerT>
    std::unique_ptr<Subscriber<SubSerializerT, MsgT>> createSubscriber(
            std::string topic, std::function<void(const MsgT&)> callback_f, size_t queue_size = 100,
            const std::shared_ptr<ThreadPool> &tp = std::make_shared<ThreadPool>(1) );

    template <class MsgT, class PubSerializerT = SerializerT>
    std::unique_ptr<Publisher<PubSerializerT, MsgT>> createPublisher(std::string topic);
};


///////////////////////////////////////////////////////////////////////////////////////////////////

template<class SerializerT>
std::unique_ptr<Node<SerializerT>> Node<SerializerT>::init(std::string name, Serializer<SerializerT> s) {
    // return std::make_unique<Node<SerializerT>>(name);
    return std::unique_ptr<Node<SerializerT>>(new Node<SerializerT>(name));
}

template<class SerializerT>
template<class MsgT, class SubSerializerT>
std::unique_ptr<Subscriber<SubSerializerT, MsgT>> Node<SerializerT>::createSubscriber(
        std::string topic, std::function<void(const MsgT&)> callback_f, size_t queue_size,
        const std::shared_ptr<ThreadPool> &tp) {

    return std::make_unique<Subscriber<SubSerializerT, MsgT>>(
        std::move( *NodeBase::createSubscriber(std::move(topic), tp, queue_size,
            Subscriber<SubSerializerT, MsgT>::GenerateIpcCb(callback_f),
            Subscriber<SubSerializerT, MsgT>::GenerateItcCb(callback_f),
            SerializerT::protocol()) ));
}

template<class SerializerT>
template<class MsgT, class PubSerializerT>
std::unique_ptr<Publisher<PubSerializerT, MsgT>> Node<SerializerT>::createPublisher(std::string topic) {
    return std::make_unique<Publisher<PubSerializerT, MsgT>>(
        std::move( *NodeBase::createPublisher(std::move(topic), SerializerT::protocol()) ));
}

}
#endif //RUM_RUM_NODE_H_
