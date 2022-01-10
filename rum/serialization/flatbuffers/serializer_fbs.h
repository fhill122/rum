//
// Created by Ivan B on 2021/4/10.
//

#ifndef RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
#define RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_

#include "rum/serialization/serializer.h"
#include "flatbuffers/flatbuffers.h"

namespace rum {

class SerializerFbs : public Serializer<SerializerFbs> {

    static void DestroyData(void *data, void *builder_p){
        // decrease shared_ptr count
        delete (std::shared_ptr<const flatbuffers::FlatBufferBuilder>*)builder_p;
    }

  public:

    // always publish the builder, no other types
    template<class T =  flatbuffers::FlatBufferBuilder>
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const flatbuffers::FlatBufferBuilder> &builder) const {
        // to achieve zero-copy, we extend the life of builder beyond this function
        auto *builder_cpy = new std::shared_ptr<const flatbuffers::FlatBufferBuilder>(builder);
        return std::make_unique<Message>((*builder_cpy)->GetBufferPointer(),
            (*builder_cpy)->GetSize(), &SerializerFbs::DestroyData, builder_cpy);
    }

    // v1. this worked, but not elegant
    // note: T is a pointer to const: const RootType*
    // We can only produce a pointer instead of an object, luckily the msg_in outlives the callback
    // template<typename T>
    // inline std::unique_ptr<T> deserialize(const zmq::message_t &msg_in,
    //                                const std::string &protocol = "") {
    //     const auto *obj = flatbuffers::GetRoot<typename std::remove_pointer<T>::type>(msg_in.data());
    //     return std::make_unique<T>(obj);
    // }

    // v2. T is just the root type
    template<typename T>
    inline std::unique_ptr<T> deserialize(const Message &msg_in,
                                          const std::string &msg_protocol = "") const {
        if (msg_protocol!=protocol()) return nullptr;
        const auto *obj = flatbuffers::GetRoot<T>(msg_in.data());
        // note two things:
        // 1. the const cast is alright, as we will never modify it
        // 2. destructor will not actually do anything
        // update: test it, really unique_ptr? flatbuffers struct seems have no field, but access methods. but is it really safe to delete?
        return std::unique_ptr<T>(const_cast<T*>(obj));
    }

    // template<typename SubT, typename PubT = void>
    // std::unique_ptr<SubT> itcTypeConvert(const void *builder){
    //     auto builder_p = (const flatbuffers::FlatBufferBuilder*)builder;
    //     const auto *obj = flatbuffers::GetRoot<SubT>(builder_p->GetBufferPointer());
    //     // note two things:
    //     // 1. the const cast is alright, as we will never modify it
    //     // 2. destructor will not actually do anything
    //     return std::unique_ptr<SubT>(const_cast<SubT*>(obj));
    // }

    template<typename SubT, typename PubT = void>
    std::function<void(const void *)>
    ipcToItcCallback(const std::function<void(const SubT&)> &callback_f) const {
        return [&](const void *itc_msg){
            auto builder_p = (const flatbuffers::FlatBufferBuilder*)itc_msg;
            const SubT *obj = flatbuffers::GetRoot<SubT>(builder_p->GetBufferPointer());
            callback_f(*obj);
        };
    }
    //
    // const void* pubToSubType(const void* pub_obj) const{
    //     auto *builder_p = (const flatbuffers::FlatBufferBuilder*)pub_obj;
    //     return ((S*)this)-> template pubToSubType(pub_obj);
    // }

    static std::string protocol() {
        return "fbs";
    }

};

}

#endif //RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
