/*
 * Created by Ivan B on 2021/12/23.
 */

#ifndef RUM_SERIALIZATION_NATIVE_AUTOSERIALIZE_H_
#define RUM_SERIALIZATION_NATIVE_AUTOSERIALIZE_H_

#include "handwritten.h"

// if custom serialization is a template, they should be included before here

namespace rum {

// helper template to trigger compilation failure
template<typename T>
struct DependentFalse { static constexpr bool value = false; };

template<typename T>
size_t GetSerializationSize(const T &t) {
    static_assert(DependentFalse<T>::value, "No rules to get serialization size");
}

template<typename T>
bool Serialize(char *buffer, const T &t) {
    static_assert(DependentFalse<T>::value, "No rules to serialize");
}

template<typename T>
bool Deserialize(const char *buffer, T &t) {
    static_assert(DependentFalse<T>::value, "No rules to deserialize");
}

// note ivan. would it be better that swap the order of 2nd and 3rd options?
template<typename T>
size_t AutoGetSerializationSize(const T &t) {
    if constexpr(std::is_base_of<HandwrittenSerialization, T>::value) {
        return t.getSerializationSize();
    } else if constexpr(std::is_trivially_copyable<T>::value) {
        return sizeof(T);
    } else {
        return GetSerializationSize(t);
    }
}

template<typename T, typename... Args>
size_t AutoGetSerializationSize(const T &t, const Args &... args) {
    return AutoGetSerializationSize(t) + AutoGetSerializationSize(args...);
}

template<typename T>
void AutoSerialize(char *buffer, const T &t) {
    if constexpr(std::is_base_of<HandwrittenSerialization, T>::value) {
        t.serialize(buffer);
    } else if constexpr(std::is_trivially_copyable<T>::value) {
        char *t_ptr = (char *) (&t);
        std::copy(t_ptr, t_ptr + sizeof(T), buffer);
    } else {
        Serialize(buffer, t);
    }
}

template<typename T, typename... Args>
void AutoSerialize(char *buffer, const T &t, const Args &... args) {
    AutoSerialize(buffer, t);
    size_t size = AutoGetSerializationSize(t);
    AutoSerialize(buffer + size, args...);
}

template<typename T>
void AutoDeserialize(const char *buffer, T &t) {
    if constexpr(std::is_base_of<HandwrittenSerialization, T>::value) {
        t.deserialize(buffer);
    } else if constexpr(std::is_trivially_copyable<T>::value) {
        std::copy(buffer, buffer + sizeof(T), (char *) (&t));
    } else {
        Deserialize(buffer, t);
    }
}

template<typename T, typename... Args>
void AutoDeserialize(const char *buffer, T &t, Args &... args) {
    AutoDeserialize(buffer, t);
    size_t size = AutoGetSerializationSize(t);
    AutoDeserialize(buffer + size, args...);
}

#define AUTO_SERIALIZE_MEMBERS(...) \
    [[nodiscard]] size_t getSerializationSize() const override { \
        return rum::AutoGetSerializationSize(__VA_ARGS__);            \
    }                                \
                                     \
    void serialize(char *_buffer_data__) const override {        \
        rum::AutoSerialize(_buffer_data__, __VA_ARGS__);              \
    }                               \
                                    \
    void deserialize(const char *_buffer_data__) override {      \
        rum::AutoDeserialize(_buffer_data__, __VA_ARGS__);            \
    }


}
#endif //RUM_SERIALIZATION_NATIVE_AUTOSERIALIZE_H_
