//
// Created by Ivan B on 2021/4/10.
//

#ifndef RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
#define RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_

// not necessary the rum included flatbuffers
#include "flatbuffers/flatbuffers.h"

#include "rum/cpprum/serialization/serializer.h"
#include "rum/common/log.h"

namespace rum {

using FbsBuilder = flatbuffers::FlatBufferBuilder;

class SerializerFbs : public Serializer<SerializerFbs> {
    template<class T>
    static void DestroyData(void *data, void *builder_p){
        // decrease shared_ptr count
        delete (std::shared_ptr<const T>*)builder_p;
    }

  public:

    // always publish the builder, no other types
    template<class T =  FbsBuilder>
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const FbsBuilder> &builder) const {
        static_assert(std::is_same<T,FbsBuilder>::value, "Publish type is FlatBufferBuilder");
        // to achieve zero-copy, we extend the life of builder beyond this function
        auto *builder_cpy = new std::shared_ptr<const FbsBuilder>(builder);
        return std::make_unique<Message>((*builder_cpy)->GetBufferPointer(),
            (*builder_cpy)->GetSize(), &SerializerFbs::DestroyData<FbsBuilder>, builder_cpy);
    }

    template<typename T = Message>
    std::unique_ptr<T> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &msg_protocol) const {
        static_assert(std::is_same<T,Message>::value, "Subscribe type is Message");
        if (msg_protocol!=Protocol()) return nullptr;
        // create a unique_ptr of the same msg_in data and prolong msg_in life
        auto *msg_in_cpy = new std::shared_ptr<const Message>(msg_in);
        return std::make_unique<Message>(
                const_cast<void*>(msg_in->data()),msg_in->size(),
                &SerializerFbs::DestroyData<Message>, msg_in_cpy);
    }

    // always subscribe with Message, as this is the only way to copy flatbuffers objects
    template<typename SubT = Message>
    std::shared_ptr<const SubT> intraProcTypeConvert(const std::shared_ptr<const void>& msg) const{
        AssertLog(msg, "");
        auto *builder_cpy = new std::shared_ptr<const FbsBuilder>(
                std::static_pointer_cast<const FbsBuilder>(msg) );
        // create a message from builder data and prolong builder life
        return std::make_shared<Message>(
                (*builder_cpy)->GetBufferPointer(), (*builder_cpy)->GetSize(),
                &SerializerFbs::DestroyData<FbsBuilder>, builder_cpy);
    }

    static std::string Protocol() {
        return "fbs";
    }
};

}

#endif //RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
