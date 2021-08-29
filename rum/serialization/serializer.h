//
// Created by Ivan B on 2021/3/30.
//

#ifndef RUM_SERIALIZATION_SERIALIZER_H_
#define RUM_SERIALIZATION_SERIALIZER_H_

#include <rum/common/buffer.h>

#include <rum/extern/zmq/zmq.hpp>



namespace rum {
// todo ivan. take unique_ptr instead to achieve zero copy

// template<typename S>
// class Serializer {
//     std::function<void(Buffer &buffer, void*)> destroy_f_;
//
//   public:
//     explicit Serializer(std::function<void(Buffer &buffer, void*)> destroy_f = nullptr){
//         if (!destroy_f)
//             destroy_f_ = [](Buffer &buffer, void*){buffer.clear();};
//     };
//
//     template<typename T>
//     void serialize(const T& t, Buffer &buffer_out) {
//         ((S*)this)-> template serialize<T>(t, buffer_out);
//     }
//
//     template<typename T>
//     std::unique_ptr<T> deserialize(const Buffer& buffer_in,
//                                    const std::string &protocol=""){
//         return ((S*)this)-> template deserialize<T>(buffer_in, protocol);
//     }
//
//     std::string protocol(){
//         return ((S*)this->protocol());
//     }
// };


template<typename S>
class Serializer {

  public:

    /**
     * serialization function
     * @tparam T Object type
     * @param t Object that is taken to serialize (it maybe moved).
     * shared as it may pass to itc sub at the same time, and we choose shared_ptr over raw pointer or reference as object life is managed here as well.
     * @return Message to be sent
     */
    template<typename T>
    std::unique_ptr<zmq::message_t> serialize(const std::shared_ptr<const T> &t) const {
        return ((S*)this)-> template serialize<T>(t);
    }

    // deserialization is done in each subscriber
    // todo ivan return const pointer maybe, as in memory serialization cannot get unique_ptr.
    //  maybe in this case return unique_ptr<T*>
    /**
     *
     * @tparam T
     * @param msg_in
     * @param protocol
     * @return
     */
    template<typename T>
    std::unique_ptr<T> deserialize(const zmq::message_t &msg_in,
                                   const std::string &protocol="") const{
        return ((S*)this)-> template deserialize<T>(msg_in, protocol);
    }

    // for serialization that have different type in pub and sub.
    // Useful with in memory serialization like flatbuffers, capnproto, etc.
    template<typename SubT, typename PubT = void>
    std::function<void(const void *)>
    itcFuncConvert(const std::function<void(const SubT&)> &callback_f) const {
        return ((S*)this)-> template itcFuncConvert<SubT, PubT>(callback_f);
    }

    static std::string protocol(){
        return S::rotocol();
    }
};

}

#endif //RUM_SERIALIZATION_SERIALIZER_H_
