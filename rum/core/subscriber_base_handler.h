//
// Created by Ivan B on 2021/4/16.
//

#ifndef RUM_CORE_SUBSCRIBER_BASE_HANDLER_H_
#define RUM_CORE_SUBSCRIBER_BASE_HANDLER_H_

#include <memory>

namespace rum {

class SubscriberBaseImpl;
class NodeBase;

class SubscriberBaseHandler{
  private:
    friend NodeBase;
    SubscriberBaseImpl* pimpl_ = nullptr;

  protected:
    explicit SubscriberBaseHandler(SubscriberBaseImpl* pimpl);

  public:
    inline virtual ~SubscriberBaseHandler() = default;
};

}

#endif //RUM_CORE_SUBSCRIBER_BASE_HANDLER_H_
