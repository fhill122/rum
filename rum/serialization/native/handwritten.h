/*
 * Created by Ivan B on 2021/12/22.
 */

#ifndef RUM_SERIALIZATION_NATIVE_HANDWRITTEN_H_
#define RUM_SERIALIZATION_NATIVE_HANDWRITTEN_H_

#include <cstddef>

// todo ivan. to test all these
namespace rum{

struct HandwrittenSerialization{
    [[nodiscard]] virtual size_t getSerializationSize() const = 0;
    virtual void serialize(char* buffer) const = 0;
    virtual void deserialize(const char* buffer) = 0;
};

template<typename T>
size_t AutoGetSerializationSize(const T& t);

template<typename T, typename... Args>
size_t AutoGetSerializationSize(const T& t, Args&& ... args);

template<typename T>
void AutoSerialize(char* buffer, const T& t);

template<typename T, typename... Args>
void AutoSerialize(char* buffer, const T& t, const Args&... args);

template<typename T>
void AutoDeserialize(const char* buffer, T& t);

template<typename T, typename... Args>
void AutoDeserialize(const char* buffer, T& t, Args&... args);

}

#endif //RUM_SERIALIZATION_NATIVE_HANDWRITTEN_H_
