/*
 * Created by Ivan B on 2021/12/23.
 */

#ifndef RUM_SERIALIZATION_NATIVE_COMMON_TYPES_H_
#define RUM_SERIALIZATION_NATIVE_COMMON_TYPES_H_

#include "handwritten.h"

namespace rum{

// std::string
size_t GetSerializationSize(const std::string &str){

}

// vector. store as [data byte size, data]
template<typename T>
size_t GetSerializationSize(const std::vector<T> &obj){
    size_t size = sizeof(size_t);
    for (const auto &o : obj){
        size += AutoSerializeGetSize(o);
    }
    return size;
}

}

#endif //RUM_SERIALIZATION_NATIVE_COMMON_TYPES_H_
