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

    template<typename T = Message>
    std::shared_ptr<const void> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &msg_protocol) const {
        if (msg_protocol!=Protocol()) return nullptr;
        return msg_in;
    }

    // always subscribe with Message, as this is the only way to copy flatbuffers objects

    template<typename SubT = Message>
    ItcFunc generateItcCallback(const SubFunc<SubT> &callback_f) const{
        return [callback_f](const std::shared_ptr<const void>& msg){
            auto builder_p = (const flatbuffers::FlatBufferBuilder*)msg.get();
            // create a dummy message from builder data and handles no destruction
            auto message = std::make_shared<Message>(
                    builder_p->GetBufferPointer(), builder_p->GetSize(),
                    [](void*,void*){printf("destroy\n");} );
            callback_f(message);
        };
    }

    template<typename SubT = Message>
    IpcFunc generateIpcCallback(const SubFunc<SubT> &callback_f) const{
        return [callback_f](const std::shared_ptr<const void>& msg){
            callback_f(std::static_pointer_cast<const Message>(msg));
        };
    }


    static std::string Protocol() {
        return "fbs";
    }

    static bool InMemorySerialization() {return true;}

};

}

#endif //RUM_SERIALIZATION_FLATBUFFERS_SERIALIZER_FBS_H_
