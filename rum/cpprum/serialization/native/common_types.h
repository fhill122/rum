/*
 * Common types that do not depend ton 3rd party libraries go here
 *
 * Created by Ivan B on 2021/12/23.
 */

#ifndef RUM_SERIALIZATION_NATIVE_COMMON_TYPES_H_
#define RUM_SERIALIZATION_NATIVE_COMMON_TYPES_H_

#include <memory>
#include <vector>
#include <map>
#include "handwritten.h"

namespace rum{

// std::string. store as [string.size(), data]
inline size_t GetSerializationSize(const std::string &str){
    return sizeof(size_t) + str.size();
}

inline void Serialize(char* buffer, const std::string &str){
    AutoSerialize(buffer, str.size());
    buffer += AutoGetSerializationSize(str.size());
    std::copy(str.data(), str.data()+str.size(), buffer);
}

inline void Deserialize(const char* buffer, std::string &str){
    size_t size;
    AutoDeserialize(buffer, size);
    buffer += AutoGetSerializationSize(size);
    str = std::string(buffer, size);
}

// std::unique_ptr. store as [if null, obj]. we assume default constructor exists
template<typename T>
size_t GetSerializationSize(const std::unique_ptr<T> &ptr){
    return ptr? sizeof(char) + AutoGetSerializationSize(*ptr) : sizeof(char);
}

template<typename T>
void Serialize(char* buffer, const std::unique_ptr<T> &ptr){
    char valid = (bool) ptr;
    AutoSerialize(buffer, valid);
    if(ptr){
        buffer += AutoGetSerializationSize(valid);
        AutoSerialize(buffer, *ptr);
    }
}

template<typename T>
void Deserialize(const char* buffer, std::unique_ptr<T> &ptr){
    char valid;
    AutoDeserialize(buffer, valid);
    if (valid){
        // assume default constructor exists
        ptr = std::make_unique<T>();
        buffer += AutoGetSerializationSize(valid);
        AutoDeserialize(buffer, *ptr);
    }
    else{
        ptr = nullptr;
    }
}

// std::vector. store as [vector.size(), object 0, object 1, ...]
template<typename T, typename Allocator>
size_t GetSerializationSize(const std::vector<T,Allocator> &obj){
    size_t size = sizeof(size_t);
    for (const auto &o : obj){
        size += AutoGetSerializationSize(o);
    }
    return size;
}

template<typename T, typename Allocator>
void Serialize(char* buffer, const std::vector<T,Allocator> &obj){
    AutoSerialize(buffer, obj.size());
    buffer += AutoGetSerializationSize(obj.size());

    for (const auto &o : obj){
        AutoSerialize(buffer, o);
        buffer += AutoGetSerializationSize(o);
        // should we try to accelerate for trivial objects?
        //  also make sure serial operation is not override
    }
}

template<typename T, typename Allocator>
void Deserialize(const char* buffer, std::vector<T,Allocator> &obj){
    size_t size;
    AutoDeserialize(buffer, size);
    buffer += AutoGetSerializationSize(size);

    obj.resize(size);
    // vector<bool>
    if constexpr(std::is_same<bool,T>::value){
        for (size_t i = 0; i < obj.size(); ++i) {
            bool b;
            AutoDeserialize(buffer, b);
            buffer += AutoGetSerializationSize(b);
            obj[i] = b;
        }
    } else {
        for (T &o : obj){
            AutoDeserialize(buffer, o);
            buffer += AutoGetSerializationSize(o);
        }
    }
}

// std::pair
template<typename F,typename S>
size_t GetSerializationSize(const std::pair<F,S> &obj){
    return AutoGetSerializationSize(obj.first) + AutoGetSerializationSize(obj.second);
}

template<typename F,typename S>
void Serialize(char* buffer, const std::pair<F,S> &obj){
    AutoSerialize(buffer, obj.first);
    buffer += AutoGetSerializationSize(obj.first);
    AutoSerialize(buffer, obj.second);
}

template<typename F,typename S>
void Deserialize(const char* buffer, std::pair<F,S> &obj){
    AutoDeserialize(buffer, obj.first);
    buffer += AutoGetSerializationSize(obj.first);
    AutoDeserialize(buffer, obj.second);
}

// std::map
template<typename K,typename V, typename C, typename A>
size_t GetSerializationSize(const std::map<K,V,C,A> &obj){
    size_t size = sizeof(size_t);
    for (const auto &o : obj){
        size += AutoGetSerializationSize(o);
    }
    return size;
}

template<typename K,typename V, typename C, typename A>
void Serialize(char* buffer, const std::map<K,V,C,A> &obj){
    AutoSerialize(buffer, obj.size());
    buffer += AutoGetSerializationSize(obj.size());

    for (const auto &o : obj){
        AutoSerialize(buffer, o);
        buffer += AutoGetSerializationSize(o);
    }
}

template<typename K,typename V, typename C, typename A>
void Deserialize(const char* buffer, std::map<K,V,C,A> &obj){
    size_t size;
    AutoDeserialize(buffer, size);
    buffer += AutoGetSerializationSize(size);

    for (int i = 0; i < size; ++i) {
        std::pair<K,V> p;
        AutoDeserialize(buffer, p);
        obj.insert(obj.end(), std::move(p));
        buffer += AutoGetSerializationSize(p);
    }
}

}

#endif //RUM_SERIALIZATION_NATIVE_COMMON_TYPES_H_
