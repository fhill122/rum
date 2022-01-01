//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_SUBSCRIBER_H_
#define RUM_RUM_SUBSCRIBER_H_

#include <rum/core/subscriber_base.h>
#include <rum/serialization/serializer.h>

namespace rum {

template <class SerializerT, class MsgT>  // add PubT is serialization required
class Subscriber : public SubscriberBase{
  private:
    static Serializer<SerializerT> s_;
  public:

  private:

  public:
    explicit Subscriber(SubscriberBase &&base) : SubscriberBase(std::move(base)) {}

    static std::function<void(zmq::message_t&)> GenerateIpcCb(
            const std::function<void(const MsgT&)> &callback_f){
        return [&](zmq::message_t &msg){
            callback_f(*s_.template deserialize<MsgT>(msg));
        };
    }

    static std::function<void(const void *)> GenerateItcCb(
            const std::function<void(const MsgT&)> &callback_f){
        return s_.template itcFuncConvert(callback_f);
    }

    static const Serializer<SerializerT>& GetSerializer(){
        return s_;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class SerializerT, class MsgT>
Serializer<SerializerT> Subscriber<SerializerT, MsgT>::s_{};

// template<class SerializerT, class MsgT>
// std::function<void(zmq::message_t &)> Subscriber<SerializerT, MsgT>::GenerateIpcCb(const std::function<void(
//         const MsgT &)> &callback_f) {
//     return [&](zmq::message_t &msg){
//         callback_f(*s_.template deserialize<MsgT>(msg));
//     };
// }

// template<class SerializerT, class MsgT>
// std::function<void(std::shared_ptr<void> &)> Subscriber<SerializerT, MsgT>::GenerateItcCb(const std::function<
//         void(const MsgT &)> &callback_f) {
//     return [&](std::shared_ptr<void> &itc_msg){
//         auto msg = s_.template itcTypeConvert<MsgT>(itc_msg);
//         callback_f(*msg);
//     };
// }

}
#endif //RUM_RUM_SUBSCRIBER_H_
