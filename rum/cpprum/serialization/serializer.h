//
// Created by Ivan B on 2021/3/30.
//

#ifndef RUM_SERIALIZATION_SERIALIZER_H_
#define RUM_SERIALIZATION_SERIALIZER_H_

#include <fstream>

#include "rum/common/serialization.h"
#include "rum/common/log.h"

namespace rum {

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
    std::unique_ptr<T> deserialize(std::shared_ptr<const Message> &msg_in,
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


    /**
     * Get protocol. Name starts with "__" is reserved
     * @return Protocol
     */
    static std::string Protocol(){
        return S::Protocol();
    }

    /* io related */

    template<typename T>
    std::unique_ptr<T> importFromFile(const std::string &path) const {
        using namespace std;
        ifstream file(path, ios::in | ios::binary);
        if (!file) {
            log.e(__func__, "file operation failed");
            return nullptr;
        }

        file.seekg(0, ios::end);
        shared_ptr<Message> message = make_shared<Message>(file.tellg());
        file.seekg(0, ios::beg);
        file.read((char*)message->data(), message->size());
        if (!file){
            log.e(__func__, "file operation failed");
            return nullptr;
        }

        shared_ptr<const Message> message_const = move(message);
        unique_ptr<T> obj = deserialize<T>(message_const, Protocol());
        if (!obj){
            log.e(__func__, "failed to deserialize");
            return nullptr;
        }
        return obj;
    }

    template<typename T>
    bool importFromFile(const std::string &path, T& t) const {
        std::unique_ptr<T> obj = importFromFile<T>(path);
        if (!obj) return false;
        // possible to eliminate copy assign?
        static_assert(std::is_copy_assignable<T>::value);
        t = *obj;
        return true;
    }

    // TODO(ivan): taking input as shared_ptr as well
    template<typename T>
    bool exportToFile(const std::string &path, const T& t) const {
        using namespace std;
        ofstream file(path, ios::out | ios::binary);
        if (!file){
            log.e(__func__, "file operation failed");
            return false;
        }

        // todo ivan. eliminate copy
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
    std::shared_ptr<const ReqT> req_obj = req_serializer.template deserialize<ReqT>(request, req_protocol);
    // we require RepT default constructor exist
    auto rep_obj = std::make_shared<RepT>();
    bool ok = callback_f(req_obj, rep_obj);
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
