//
// Created by Ivan B on 2021/4/1.
//

#ifndef RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_
#define RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_

#include "rum/cpprum/serialization/serializer.h"

namespace rum{

class SerializerProto : public Serializer<SerializerProto>{
  public:
    template<class T>
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const T> &object) const{
        // todo ivan.
        size_out = 1;
        *data_out = new char[size_out];
        t.SerializeToArray(*data_out);
    }

    template<typename T>
    std::unique_ptr<const void> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &protocol=""){
        // todo ivan.
    }

    std::string protocol(){
        return "protobuf3";
    }

};

}
#endif //RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_
