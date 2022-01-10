//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_SUBSCRIBER_H_
#define RUM_RUM_SUBSCRIBER_H_

#include "rum/core/subscriber_base_handler.h"
#include "rum/serialization/serializer.h"

namespace rum {

template <class SerializerT, class MsgT>  // add PubT is serialization required
class SubscriberHandler : public SubscriberBaseHandler{
  private:
    static Serializer<SerializerT> s_;
  public:

  private:

  public:
    explicit SubscriberHandler(SubscriberBaseHandler &&base) : SubscriberBaseHandler(std::move(base)) {}
    SubscriberHandler() : SubscriberBaseHandler(nullptr){};

    static std::function<void(Message &)> GenerateIpcCb(
            const std::function<void(const MsgT&)> &callback_f){
        return [&](zmq::message_t &msg){
            callback_f(*s_.template deserialize<MsgT>(msg));
        };
    }

    static std::function<void(const void *)> GenerateItcCb(
            const std::function<void(const MsgT&)> &callback_f){
        return s_.template ipcToItcCallback(callback_f);
    }

    static const Serializer<SerializerT>& GetSerializer(){
        return s_;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// template<class SerializerT, class MsgT>
// Serializer<SerializerT> SubscriberHandler<SerializerT, MsgT>::s_{};
//
// template<class SerializerT, class MsgT>
// std::function<void(Message &)> SubscriberHandler<SerializerT, MsgT>::GenerateIpcCb(const std::function<void(
//         const MsgT &)> &callback_f) {
//     return [&](zmq::message_t &msg){
//         callback_f(*s_.template deserialize<MsgT>(msg));
//     };
// }
//
// template<class SerializerT, class MsgT>
// std::function<void(std::shared_ptr<void> &)> SubscriberHandler<SerializerT, MsgT>::GenerateItcCb(const std::function<
//         void(const MsgT &)> &callback_f) {
//     return [&](std::shared_ptr<void> &itc_msg){
//         auto msg = s_.template itcTypeConvert<MsgT>(itc_msg);
//         callback_f(*msg);
//     };
// }

}
#endif //RUM_RUM_SUBSCRIBER_H_
