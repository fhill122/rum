//
// Created by Ivan B on 2021/3/20.
//

#include "node_base.h"

#include "internal/node_base_impl.h"
#include "internal/subscriber_base_impl.h"
#include "rum/common/log.h"


using namespace std;

namespace rum{

void NodeBase::Init(const NodeParam &param) {
    static std::atomic_flag inited = ATOMIC_FLAG_INIT;
    if (inited.test_and_set())
        AssertLog(false, "double init");
    GlobalNode() = unique_ptr<NodeBase>(new NodeBase("__global", param));
}

std::unique_ptr<NodeBase> &NodeBase::GlobalNode() {
    static unique_ptr<NodeBase> node;
    return node;
}

NodeBase::NodeBase(const string &name, const NodeParam &param):
        pimpl_(make_unique<NodeBaseImpl>(name, param)){}

NodeBase::~NodeBase() =  default;

SubscriberBaseHandler NodeBase::addSubscriber(const std::string &topic,
                                              const shared_ptr<ThreadPool> &tp,
                                              size_t queue_size,
                                              const function<void(zmq::message_t &)> &ipc_cb,
                                              const function<void(const void *)> &itc_cb,
                                              const std::string &protocol) {
    return SubscriberBaseHandler(pimpl_->addSubscriber(topic, tp, queue_size, ipc_cb, itc_cb, protocol));
}

void NodeBase::removeSubscriber(SubscriberBaseHandler &subscriber_handler) {
    pimpl_->removeSubscriber(subscriber_handler.pimpl_);
}

PublisherBaseHandler NodeBase::addPublisher(const std::string &topic,
                                            const std::string &protocol) {
    return PublisherBaseHandler(pimpl_->addPublisher(topic, protocol));
}

void NodeBase::removePublisher(PublisherBaseHandler &publisher_handler) {
    pimpl_->removePublisher(publisher_handler.pimpl_);
}

}