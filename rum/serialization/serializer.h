//
// Created by Ivan B on 2021/3/30.
//

#ifndef RUM_SERIALIZATION_SERIALIZER_H_
#define RUM_SERIALIZATION_SERIALIZER_H_

#include <memory>
#include <fstream>

#include "rum/common/message.h"
#include "rum/common/log.h"

namespace rum {

// todo ivan. allow other version of SubFunc and SrvFunc, e.g. taking reference

template<typename T>
using SubFunc = std::function<void(const std::shared_ptr<const T>&)>;
// void is actually the type that DeserFunc converts to
using InterProcFunc = std::function<void(const std::shared_ptr<const void>&)>;
// void is actually the type of scheduled intra-proc object
using IntraProcFunc = std::function<void(const std::shared_ptr<const void>&)>;
template<typename T=void>
using DeserFunc = std::function<
        std::shared_ptr<const T> (std::shared_ptr<const Message>&, const std::string&) >;
template<typename T=void>
using SerFunc = std::function<
        std::unique_ptr<Message> (const std::shared_ptr<const T>&) >;
template<typename T>
using IntraProcFactoryFunc = std::function<std::shared_ptr<const T>(const std::shared_ptr<const void>&)>;

// SubT, PubT
template<typename Q, typename P>
using SrvFunc = std::function<bool(const std::shared_ptr<const Q>& request, std::shared_ptr<P>& response)>;
using SrvIntraProcFunc = std::function<bool(std::shared_ptr<const void>& request,
                                            std::shared_ptr<void>& response)>;
using SrvInterProcFunc = std::function<bool(std::shared_ptr<const Message>& request,
                                            const std::string& req_protocol, std::shared_ptr<Message>& response) >;

template<typename S>
class Serializer {

  public:

    // todo ivan. maybe multiple forms of serialization and deserialization

    /**
     * Serialization function
     * @tparam T Object type
     * @param t Object that is taken to serialize (it maybe moved).
     * shared as it may pass to intra-proc sub at the same time, and we choose shared_ptr over raw pointer or
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
    std::shared_ptr<const SubT> intraProcTypeConvert(const std::shared_ptr<const void>& msg) const{
        // check if this function is overridden
        // todo ivan. linux gcc failed while mac clang passed
        // if constexpr (&S::template intraProcTypeConvert<SubT> == &Serializer<S>::intraProcTypeConvert<SubT>)
        if (&S::template intraProcTypeConvert<SubT> == &Serializer<S>::intraProcTypeConvert<SubT>)
            return std::static_pointer_cast<const SubT>(msg);
        else
            return ((S *) this)->template intraProcTypeConvert<SubT>(msg);
    }

    // convert from deserialized type to sub type. would ever override it?
    template<typename SubT>
    std::shared_ptr<const SubT> interProcTypeConvert(const std::shared_ptr<const void>& msg) const{
        // check if this function is overridden
        // todo ivan. linux gcc failed while mac clang passed
        // if constexpr (&S::template interProcTypeConvert<SubT> == &Serializer<S>::interProcTypeConvert<SubT>)
        if (&S::template interProcTypeConvert<SubT> == &Serializer<S>::interProcTypeConvert<SubT>)
            return std::static_pointer_cast<const SubT>(msg);
        else
            return ((S *) this)->template interProcTypeConvert<SubT>(msg);
    }

    /**
     * Get protocol. Name starts with "__" is reserved
     * @return Protocol
     */
    static std::string Protocol(){
        return S::Protocol();
    }

    /* io related */

    template<typename T>
    bool importFromFile(const std::string &path, T& t) const {
        using namespace std;
        ifstream file(path, ios::in | ios::binary);
        if (!file) {
            log.e(__func__, "file operation failed");
            return false;
        }

        file.seekg(0, ios::end);
        shared_ptr<Message> message = make_shared<Message>(file.tellg());
        file.seekg(0, ios::beg);
        file.read((char*)message->data(), message->size());
        if (!file){
            log.e(__func__, "file operation failed");
            return false;
        }

        shared_ptr<const Message> message_const = move(message);
        shared_ptr<const void> obj_void = deserialize<T>(message_const, Protocol());
        if (!obj_void){
            log.e(__func__, "failed to deserialize");
            return false;
        }
        shared_ptr<const T> obj = interProcTypeConvert<T>(obj_void);
        static_assert(is_copy_assignable<T>::value);
        t = *obj;
        return true;
    }

    template<typename T>
    std::unique_ptr<T> importFromFile(const std::string &path) const {
        // todo ivan. implement this and add multiple input forms of serialization and deserialization
    }

    template<typename T>
    bool exportToFile(const std::string &path, const T& t) const {
        using namespace std;
        ofstream file(path, ios::out | ios::binary);
        if (!file){
            log.e(__func__, "file operation failed");
            return false;
        }

        static_assert(is_copy_constructible<T>::value);
        shared_ptr<T> obj = make_shared<T>(t);
        unique_ptr<Message> message = serialize<T>(obj);
        if (!message){
            log.e(__func__, "failed to serialize");
            return false;
        }

        file.write((char*)message->data(), message->size());
        return true;
    }

};

template<typename ReqT, typename RepT, class ReqSerializerT, class RepSerializerT>
bool SrvInterProcCallback(const Serializer<ReqSerializerT> &req_serializer,
                          const Serializer<RepSerializerT> &rep_serializer,
                          const SrvFunc<ReqT, RepT> &callback_f,
                          std::shared_ptr<const Message>& request, const std::string& req_protocol,
                          std::shared_ptr<Message>& response){
    auto req_void = req_serializer.template deserialize<ReqT>(request, req_protocol);
    // we require RepT default constructor exist
    auto rep_obj = std::make_shared<RepT>();
    bool ok = callback_f(req_serializer.template interProcTypeConvert<ReqT>(req_void), rep_obj);
    if(ok){
        response = rep_serializer.template serialize<RepT>(std::const_pointer_cast<const RepT>(rep_obj));
    }
    return ok;
}

template<typename ReqT, typename RepT, class ReqSerializerT>
bool SrvIntraProcCallback(const Serializer<ReqSerializerT> &req_serializer,
                          const SrvFunc<ReqT, RepT> &callback_f,
                          std::shared_ptr<const void>& request, std::shared_ptr<void>& response){
    // we require RepT default constructor exist
    auto rep_obj = std::make_shared<RepT>();
    bool ok = callback_f(req_serializer.template intraProcTypeConvert<ReqT>(request), rep_obj);
    response = std::move(rep_obj);
    return ok;
}

}

#endif //RUM_SERIALIZATION_SERIALIZER_H_
