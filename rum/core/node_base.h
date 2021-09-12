//
// Created by Ivan B on 2021/3/20.
//

#ifndef RUM_CORE_NODE_BASE_H_
#define RUM_CORE_NODE_BASE_H_

#include <rum/extern/ivtb/thread_pool.h>

#include <utility>
#include "subscriber_base.h"
#include "publisher_base.h"

namespace rum {

class NodeBaseImpl;

class NodeBase {
  private:
    std::unique_ptr<NodeBaseImpl> pimpl_;

  public:

  private:

  public:
    NodeBase(std::string name = "", std::string domain = "", std::string addr = "");

    virtual ~NodeBase();

    // std::unique_ptr<SubscriberBaseImpl> createSubscriberImpl(std::string topic,
    //     const std::shared_ptr<ThreadPool> &tp, size_t queue_size,
    //     const std::function<void(zmq::message_t&)> &ipc_cb,
    //     const std::function<void(std::shared_ptr<void>&)> &itc_cb);

    std::unique_ptr<SubscriberBase> createSubscriber(std::string topic,
        const std::shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
        const std::function<void(zmq::message_t&)> &ipc_cb,
        const std::function<void(const void *)> &itc_cb, std::string protocol = "");

    std::unique_ptr<PublisherBase> createPublisher(std::string topic, std::string protocol);


    // todo ivan. should we switch to this api?
    PublisherBase* addPublisher();

    SubscriberBase* addSubscriber();

};

}
#endif //RUM_CORE_NODE_BASE_H_
