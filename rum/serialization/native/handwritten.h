/*
 * Created by Ivan B on 2021/12/22.
 */

#ifndef RUM_SERIALIZATION_NATIVE_HANDWRITTEN_H_
#define RUM_SERIALIZATION_NATIVE_HANDWRITTEN_H_

#include <vector>

// todo ivan. to test all these
namespace rum{

struct HandwrittenSerialization{
    virtual size_t getSerializationSize() const = 0;
    virtual void serialize(void* data) const = 0;
    virtual void deserialize(const void* data) = 0;
};

template<typename T>
size_t AutoSerializeGetSize(const T& t);

template<typename T, typename... Args>
size_t AutoSerializeGetSize(const T& t, Args&& ... args);


// handwritten serialization for some common types

// vector. store as [data byte size, data]
// template<typename T>
// size_t GetSerializationSize(const std::vector<T> &obj){
//     size_t size = sizeof(size_t);
//     for (const auto &o : obj){
//         size += AutoSerializeGetSize(o);
//     }
//     return size;
// }

}

#endif //RUM_SERIALIZATION_NATIVE_HANDWRITTEN_H_
