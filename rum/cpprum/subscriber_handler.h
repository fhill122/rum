//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_SUBSCRIBER_H_
#define RUM_RUM_SUBSCRIBER_H_

#include "rum/core/subscriber_base_handler.h"
#include "rum/serialization/serializer.h"

namespace rum {

template <class SerializerT>  // add PubT is serialization required
class SubscriberHandler : public SubscriberBaseHandler{
  private:
  public:
    inline static Serializer<SerializerT> s_{};

  private:

  public:
    explicit SubscriberHandler(SubscriberBaseHandler &&base) : SubscriberBaseHandler(std::move(base)) {}
    SubscriberHandler() : SubscriberBaseHandler(nullptr){};

    static const Serializer<SerializerT>& GetSerializer(){
        return s_;
    }
};

}
#endif //RUM_RUM_SUBSCRIBER_H_
