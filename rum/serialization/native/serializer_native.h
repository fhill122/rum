/*
 * Created by Ivan B on 2021/12/21.
 */

#ifndef RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
#define RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_

#include <rum/serialization/serializer.h>
#include <rum/serialization/native/autoserialize.h>

namespace rum{

/**
 * Native struct serialization.
 * Limitation:
 *  - machine dependent, may not work on devices over network that have different arch/system
 */
class SerializerNative : public Serializer<SerializerNative>{

  public:
    template <class T>
    std::unique_ptr<zmq::message_t>
    serialize(const std::shared_ptr<const T> &object) const {
        size_t size = AutoGetSerializationSize(*object);
        auto msg = std::make_unique<zmq::message_t>(size);
        AutoSerialize((char*)msg->data(), *object);
        return msg;
    }

    template<typename T>
    std::unique_ptr<T> deserialize(const zmq::message_t &msg_in,
                                   const std::string &msg_protocol="") const{
        if (msg_protocol!=protocol()) return nullptr;
        // T must have default constructor
        auto t = std::make_unique<T>();
        AutoDeserialize((char*)msg_in.data(), *t);
        return t;
    }

    inline static std::string protocol(){
        return "native";
    }
};

}
#endif //RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
