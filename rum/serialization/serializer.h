//
// Created by Ivan B on 2021/3/30.
//

#ifndef RUM_SERIALIZATION_SERIALIZER_H_
#define RUM_SERIALIZATION_SERIALIZER_H_

#include <memory>

#include "rum/common/message.h"

namespace rum {

template<typename T>
using SubFunc = std::function<void(const std::shared_ptr<const T>&)>;
// void is actually the type that DeserFunc converts to
using IpcFunc = std::function<void(const std::shared_ptr<const void>&)>;
// void is actually the type of scheduled itc object
using ItcFunc = std::function<void(const std::shared_ptr<const void>&)>;
using DeserFunc = std::function<
        std::shared_ptr<const void> (std::shared_ptr<const Message>&, const std::string&)
                >;


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

    /**
     * Generate itc callback given user input callback, for most cases they are same
     * @tparam SubT User provided callback object type
     * @param callback_f User provided callback function
     * @return Itc callback function that is invoked on published objet
     */
    template<typename SubT>
    ItcFunc generateItcCallback(const SubFunc<SubT> &callback_f) const{
        return ((S*)this) -> template generateItcCallback<SubT>(callback_f);
    }

    /**
     * Generate ipc callback given user input callback, for most cases they are same
     * @tparam SubT User provided callback object type
     * @param callback_f User provided callback function
     * @return Ipc callback function that is invoked on deserialized object
     */
    template<typename SubT>
    IpcFunc generateIpcCallback(const SubFunc<SubT> &callback_f) const{
        return ((S*)this) -> template generateIpcCallback<SubT>(callback_f);
    }

    static std::string Protocol(){
        return S::Protocol();
    }
};

}

#endif //RUM_SERIALIZATION_SERIALIZER_H_
