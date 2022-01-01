//
// Created by Ivan B on 2021/4/16.
//

#ifndef RUM_CORE_SUBSCRIBER_BASE_H_
#define RUM_CORE_SUBSCRIBER_BASE_H_

#include <memory>

namespace rum {

class SubscriberBaseImpl;

class SubscriberBase {
  private:
    SubscriberBaseImpl* pimpl_;
  public:

  private:
  public:
    explicit SubscriberBase(SubscriberBaseImpl* pimpl);
    SubscriberBase(SubscriberBase&& rhs) = default ;
    SubscriberBase& operator=(SubscriberBase&& rhs) = default ;
    SubscriberBase(const SubscriberBase&) = default;
    SubscriberBase& operator=(const SubscriberBase&) = default;
    virtual ~SubscriberBase();

};

}

#endif //RUM_CORE_SUBSCRIBER_BASE_H_
