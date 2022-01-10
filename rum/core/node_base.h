//
// Created by Ivan B on 2021/3/20.
//

#ifndef RUM_CORE_NODE_BASE_H_
#define RUM_CORE_NODE_BASE_H_

#include <utility>

#include "rum/common/thread_pool.h"
#include "rum/common/node_param.h"
#include "subscriber_base_handler.h"
#include "publisher_base_handler.h"

namespace rum {

class NodeBaseImpl;

class NodeBase {
  private:
    std::unique_ptr<NodeBaseImpl> pimpl_;

  public:

  protected:
    explicit NodeBase(const std::string &name = "", const NodeParam &param = NodeParam());

  public:
    static void Init(const NodeParam &param = NodeParam());

    static std::unique_ptr<NodeBase>& GlobalNode();


    virtual ~NodeBase();

    SubscriberBaseHandler addSubscriber(const std::string &topic,
        const std::shared_ptr<ThreadPool> &tp, size_t queue_size,
        const std::function<void(zmq::message_t&)> &ipc_cb,
        const std::function<void(const void *)> &itc_cb,
        const std::string &protocol);

    void removeSubscriber(SubscriberBaseHandler &subscriber_handler);

    PublisherBaseHandler addPublisher(const std::string &topic, const std::string &protocol);

    void removePublisher(PublisherBaseHandler &publisher_handler);

};

}
#endif //RUM_CORE_NODE_BASE_H_
