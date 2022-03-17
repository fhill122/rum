//
// Created by Ivan B on 2021/4/10.
//

#ifndef RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
#define RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_

// not necessary the rum included flatbuffers
#include <flatbuffers/flatbuffers.h>

#include "rum/serialization/serializer.h"
#include "rum/common/log.h"

namespace rum {

using FbsBuilder = flatbuffers::FlatBufferBuilder;

class SerializerFbs : public Serializer<SerializerFbs> {

    static void DestroyData(void *data, void *builder_p){
        // decrease shared_ptr count
        delete (std::shared_ptr<const FbsBuilder>*)builder_p;
    }

  public:

    // always publish the builder, no other types
    template<class T =  FbsBuilder>
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const FbsBuilder> &builder) const {
        // to achieve zero-copy, we extend the life of builder beyond this function
        auto *builder_cpy = new std::shared_ptr<const FbsBuilder>(builder);
        return std::make_unique<Message>((*builder_cpy)->GetBufferPointer(),
            (*builder_cpy)->GetSize(), &SerializerFbs::DestroyData, builder_cpy);
    }

    template<typename T = Message>
    std::shared_ptr<const void> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &msg_protocol) const {
        if (msg_protocol!=Protocol()) return nullptr;
        return msg_in;
    }

    // always subscribe with Message, as this is the only way to copy flatbuffers objects
    template<typename SubT = Message>
    std::shared_ptr<const SubT> itcTypeConvert(const std::shared_ptr<const void>& msg) const{
        AssertLog(msg, "");
        auto *builder_cpy = new std::shared_ptr<const FbsBuilder>(
                std::static_pointer_cast<const FbsBuilder>(msg) );
        // create a message from builder data and prolong builder life
        return std::make_shared<Message>(
                (*builder_cpy)->GetBufferPointer(), (*builder_cpy)->GetSize(),
                &SerializerFbs::DestroyData, builder_cpy);
    }

    static std::string Protocol() {
        return "fbs";
    }
};

}

#endif //RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
