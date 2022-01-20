//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_SUBSCRIBER_H_
#define RUM_RUM_SUBSCRIBER_H_

#include "rum/core/subscriber_base_handler.h"
#include "rum/serialization/serializer.h"

namespace rum {

class Subscriber : public SubscriberBaseHandler{
  public:
    using SharedPtr = std::shared_ptr<Subscriber>;
    using UniquePtr = std::unique_ptr<Subscriber>;


    explicit Subscriber(SubscriberBaseHandler &&base) : SubscriberBaseHandler(std::move(base)) {}
    Subscriber() : SubscriberBaseHandler(nullptr){};

    ~Subscriber() override {
        NodeBase::GlobalNode()->removeSubscriber(*this);
    }
};

}
#endif //RUM_RUM_SUBSCRIBER_H_
