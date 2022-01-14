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
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const T> &object) const {
        size_t size = AutoGetSerializationSize(*object);
        auto msg = std::make_unique<Message>(size);
        AutoSerialize((char*)msg->data(), *object);
        return msg;
    }

    template<typename T>
    std::shared_ptr<const void> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &msg_protocol="") const{
        if (msg_protocol!=Protocol()) return nullptr;
        // T must have default constructor
        auto t = std::make_unique<T>();
        AutoDeserialize((char*)msg_in->data(), *t);
        return t;
    }

    template<typename SubT>
    ItcFunc generateItcCallback(const SubFunc<SubT> &callback_f) const{
        return [callback_f](const std::shared_ptr<const void>& msg){
            callback_f(std::static_pointer_cast<const SubT>(msg));
        };
    }

    template<typename SubT>
    IpcFunc generateIpcCallback(const SubFunc<SubT> &callback_f) const{
        return [callback_f](const std::shared_ptr<const void>& msg){
            callback_f(std::static_pointer_cast<const SubT>(msg));
        };
    }

    inline static std::string Protocol(){
        return "native";
    }
};

}
#endif //RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
