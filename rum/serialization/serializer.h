//
// Created by Ivan B on 2021/3/30.
//

#ifndef RUM_SERIALIZATION_SERIALIZER_H_
#define RUM_SERIALIZATION_SERIALIZER_H_

#include <memory>

#include "rum/common/message.h"

namespace rum {

// todo ivan. allow other version of SubFunc and SrvFunc, e.g. taking reference

template<typename T>
using SubFunc = std::function<void(const std::shared_ptr<const T>&)>;
// void is actually the type that DeserFunc converts to
using IpcFunc = std::function<void(const std::shared_ptr<const void>&)>;
// void is actually the type of scheduled itc object
using ItcFunc = std::function<void(const std::shared_ptr<const void>&)>;
template<typename T=void>
using DeserFunc = std::function<
        std::shared_ptr<const T> (std::shared_ptr<const Message>&, const std::string&) >;
template<typename T=void>
using SerFunc = std::function<
        std::unique_ptr<Message> (const std::shared_ptr<const T>&) >;
template<typename T>
using ItcFactoryFunc = std::function<std::shared_ptr<const T>(const std::shared_ptr<const void>&)>;

// SubT, PubT
template<typename Q, typename P>
using SrvFunc = std::function<bool(const std::shared_ptr<const Q>& request, std::shared_ptr<P>& response)>;
using SrvItcFunc = std::function<bool(std::shared_ptr<const void>& request,
                                      std::shared_ptr<void>& response)>;
using SrvIpcFunc = std::function<bool(std::shared_ptr<const Message>& request,
                   const std::string& req_protocol, std::shared_ptr<Message>& response) >;

template<typename S>
class Serializer {

  public:

    /**
     * Serialization function
     * @tparam T Object type
     * @param t Object that is taken to serialize (it maybe moved).
     * shared as it may pass to itc sub at the same time, and we choose shared_ptr over raw pointer or
     * reference as object life is managed here as well.
     * @return Message to be sent
     */
    template<typename T>
    std::unique_ptr<Message> serialize(const std::shared_ptr<const T> &t) const {
        return ((S*)this)-> template serialize<T>(t);
    }

    /**
     * Deserialization function
     * @tparam T Deserialized object type
     * @param msg_in Income Message object
     * @param protocol Protocol
     * @return
     */
    template<typename T>
    std::shared_ptr<const void> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &protocol) const{
        return ((S*)this)-> template deserialize<T>(msg_in, protocol);
    }

    // convert from pub type to sub type
    template<typename SubT>
    std::shared_ptr<const SubT> itcTypeConvert(const std::shared_ptr<const void>& msg) const{
        // check if this function is overridden
        if constexpr (&S::template itcTypeConvert<SubT> == &Serializer<S>::itcTypeConvert<SubT>)
            return std::static_pointer_cast<const SubT>(msg);
        else
            return ((S*)this)-> template itcTypeConvert<SubT>(msg);
    }

    // convert from deserialized type to sub type. would ever override it?
    template<typename SubT>
    std::shared_ptr<const SubT> ipcTypeConvert(const std::shared_ptr<const void>& msg) const{
        // check if this function is overridden
        if constexpr (&S::template ipcTypeConvert<SubT> == &Serializer<S>::ipcTypeConvert<SubT>)
            return std::static_pointer_cast<const SubT>(msg);
        else
            return ((S*)this)-> template ipcTypeConvert<SubT>(msg);
    }

    /**
     * Get protocol
     * @return Protocol
     */
    static std::string Protocol(){
        return S::Protocol();
    }
};

template<typename ReqT, typename RepT, class ReqSerializerT, class RepSerializerT>
bool SrvIpcCallback(const Serializer<ReqSerializerT> &req_serializer,
                    const Serializer<RepSerializerT> &rep_serializer,
                    const SrvFunc<ReqT, RepT> &callback_f,
                    std::shared_ptr<const Message>& request, const std::string& req_protocol,
                    std::shared_ptr<Message>& response){
    auto req_void = req_serializer.template deserialize<ReqT>(request, req_protocol);
    // we require RepT default constructor exist
    auto rep_obj = std::make_shared<RepT>();
    bool ok = callback_f(req_serializer.template ipcTypeConvert<ReqT>(req_void), rep_obj);
    if(ok){
        response = rep_serializer.template serialize<RepT>(std::const_pointer_cast<const RepT>(rep_obj));
    }
    return ok;
}

template<typename ReqT, typename RepT, class ReqSerializerT>
bool SrvItcCallback(const Serializer<ReqSerializerT> &req_serializer,
                    const SrvFunc<ReqT, RepT> &callback_f,
                    std::shared_ptr<const void>& request, std::shared_ptr<void>& response){
    // we require RepT default constructor exist
    auto rep_obj = std::make_shared<RepT>();
    bool ok = callback_f(req_serializer.template itcTypeConvert<ReqT>(request), rep_obj);
    response = std::move(rep_obj);
    return ok;
}

}

#endif //RUM_SERIALIZATION_SERIALIZER_H_
