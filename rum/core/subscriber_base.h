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
    std::shared_ptr<SubscriberBaseImpl> pimpl_;
  public:

  private:
  public:
    explicit SubscriberBase(std::unique_ptr<SubscriberBaseImpl> pimpl);
    SubscriberBase(SubscriberBase&& rhs) noexcept ;
    SubscriberBase& operator=(SubscriberBase&& rhs) noexcept ;
    SubscriberBase(const SubscriberBase&) = delete;
    SubscriberBase& operator=(const SubscriberBase&) = delete;
    virtual ~SubscriberBase();

};

}

#endif //RUM_CORE_SUBSCRIBER_BASE_H_
