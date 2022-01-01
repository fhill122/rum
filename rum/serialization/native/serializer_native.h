/*
 * Created by Ivan B on 2021/12/21.
 */

#ifndef RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
#define RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_

#include <rum/serialization/serializer.h>
#include <type_traits>

namespace rum{

/**
 * Native struct serialization.
 * Limitation:
 *  - trivial copyable
 *  - machine dependent, may only works on local machine
 */
class SerializerNative : public Serializer<SerializerNative>{
    static constexpr size_t kCopyThresh = 64*10;

    template <class T>
    static void DestroyData(void *data, void *builder_p){
        // decrease shared_ptr count
        delete (std::shared_ptr<const T>*)builder_p;
    }

  public:

    template <class T>
    std::unique_ptr<zmq::message_t>
    serialize(const std::shared_ptr<const T> &object){
        // todo ivan. check trivial, if not check if user provided serialization

        static_assert(std::is_trivial<T>::value);

        // worth the trouble? if on heap yes
        if constexpr(sizeof(T)>kCopyThresh){
            return std::make_unique<zmq::message_t>(object.get(), sizeof(T));
        }

        // hacky way to prolong shared_ptr lifetime
        auto *sptr_cpy = new std::shared_ptr<const T>(object);
        return std::make_unique<zmq::message_t>(object.get(), sizeof(T)
              &SerializerNative::DestroyData<T>, sptr_cpy);


        // v2 use auto handwritten and auto serialize
        size_t size = AutoSerializeGetSize(*object);
    }

    static std::string protocol(){
        return "native";
    }
};

}
#endif //RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
