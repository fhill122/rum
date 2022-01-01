/*
 * Created by Ivan B on 2021/12/23.
 */

#ifndef RUM_SERIALIZATION_NATIVE_AUTOSERIALIZE_H_
#define RUM_SERIALIZATION_NATIVE_AUTOSERIALIZE_H_

#include "handwritten.h"
#include "common_types.h"

// if custom serialization is a template, they should be included before here

namespace rum{

// helper template to trigger compilation failure
template <typename T>
struct DependentFalse {static constexpr bool value=false;};

template<typename T>
size_t GetSerializationSize(const T& t){
    static_assert(DependentFalse<T>::value, "No definition to get serialization size");
}

template<typename T>
size_t AutoSerializeGetSize(const T& t){
    if constexpr(std::is_base_of<T, HandwrittenSerialization>::value){
        return t.getSerializationSize();
    }
    else if constexpr(std::is_trivial<T>::value){
        return sizeof(T);
    }
    else{
        return GetSerializationSize(t);
    }
}

template<typename T, typename... Args>
size_t AutoSerializeGetSize(const T& t, Args&& ... args){
    return AutoSerializeGetSize(t) + AutoSerializeGetSize(args...);
}

// todo write other functions

}

#endif //RUM_SERIALIZATION_NATIVE_AUTOSERIALIZE_H_
