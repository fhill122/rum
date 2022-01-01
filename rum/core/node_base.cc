//
// Created by Ivan B on 2021/3/20.
//

#include "node_base.h"

#include "internal/node_base_impl.h"
#include "internal/subscriber_base_impl.h"


using namespace std;

namespace rum{

NodeBase::NodeBase(std::string name, std::string domain, std::string addr):
        pimpl_(make_unique<NodeBaseImpl>(name, domain, addr)){}

NodeBase::~NodeBase() =  default;

// unique_ptr<SubscriberBaseImpl> NodeBase::createSubscriberImpl(string topic,
//            const shared_ptr<ThreadPool> &tp, size_t queue_size,
//            const function<void(zmq::message_t&)> &ipc_cb,
//            const function<void(std::shared_ptr<void>&)> &itc_cb) {
//     return make_unique<SubscriberBaseImpl>(topic, tp, queue_size, ipc_cb, itc_cb);
// }

unique_ptr<SubscriberBase> NodeBase::createSubscriber(string topic,
           const shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
           const function<void(zmq::message_t &)> &ipc_cb,
           const function<void(const void *)> &itc_cb,
           std::string protocol) {
    // should it call pimpl method instead?
    // return make_unique<SubscriberBase>(
    //        make_unique<SubscriberBaseImpl>(move(topic), tp, queue_size, ipc_cb, itc_cb, move(protocol)));
    return nullptr;
}

unique_ptr<PublisherBase> NodeBase::createPublisher(std::string topic, std::string protocol) {
    return make_unique<PublisherBase>(
        make_unique<PublisherBaseImpl>(move(topic), move(protocol), pimpl_->context_, false));
}

}