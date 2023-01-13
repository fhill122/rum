//
// Created by Ivan B on 2021/4/1.
//

#ifndef RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_
#define RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_

#include "rum/cpprum/serialization/serializer.h"
#include <google/protobuf/message.h>

namespace rum{

class SerializerProto : public Serializer<SerializerProto>{
  public:
    template<class T>
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const T> &object) const{
        static_assert(std::is_base_of<google::protobuf::Message,T>::value, "must be a protobuf message type");
        auto msg = std::make_unique<Message>(object->ByteSizeLong());
        object->SerializeToArray(msg->data(), msg->size());
        return msg;
    }

    template<typename T>
    std::unique_ptr<T> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &protocol="") const{
        static_assert(std::is_base_of<google::protobuf::Message,T>::value, "must be a protobuf message type");
        if (protocol!=Protocol()) return nullptr;

        auto obj = std::make_unique<T>();
        bool ok = obj->ParseFromArray(msg_in->data(), msg_in->size());
        return ok? std::move(obj) : nullptr;
    }

    inline static std::string Protocol(){
        return "protobuf3";
    }

};

}
#endif //RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_
