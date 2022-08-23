//
// Created by Ivan B on 2021/4/1.
//

#ifndef RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_
#define RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_

#include "rum/cpprum/serialization/serializer.h"

class SerializerProto : public Serializer<SerializerProto>{
  public:
    template<class T>
    void serialize(const T& t, char **data_out, size_t &size_out) {
        // todo ivan.
        size_out = 1;
        *data_out = new char[size_out];
        t.SerializeToArray(*data_out);
    }

    template<typename T>
    std::unique_ptr<T> deserialize(const char* data, size_t size,
                                   const std::string &protocol=""){
        // todo ivan.
    }

    std::string protocol(){
        return "protobuf3";
    }

};

#endif //RUM_SERIALIZATION_PROTOBUF_SERIALIZERPROTO_H_
