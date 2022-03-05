/*
 * Created by Ivan B on 2021/12/29.
 */

#ifndef RUM_COMMON_MISC_H_
#define RUM_COMMON_MISC_H_

#include <unordered_map>
#include <vector>
#include "log.h"

namespace rum{

/**
 * Add an element to a map of vectors.
 * @tparam KeyT
 * @tparam VecT
 * @param map_container
 * @param key
 * @param obj
 * @return The iterator of the map
 */
template <typename KeyT, typename VecT>
typename std::unordered_map<KeyT,std::vector<VecT>>::iterator
MapVecAdd(std::unordered_map<KeyT,std::vector<VecT>> &map_container,
               const KeyT& key, VecT&& obj){
    auto itr = map_container.find(key);
    if (itr==map_container.end()){
        std::vector<VecT> vec;
        vec.push_back(std::forward<VecT>(obj));
        map_container[key] = std::move(vec);
    }
    else{
        AssertLog(!itr->second.empty(), "exist empty vector");
        itr->second.push_back(std::forward<VecT>(obj));
    }
    return itr;
}

/**
 * Remove an element from a map of vectors. Assert element exists.
 * If the element is the last one of the vector, the whole vector is removed.
 * @tparam KeyT
 * @tparam VecT
 * @tparam ObjT
 * @tparam CompFuncT
 * @param map_container
 * @param key
 * @param obj
 * @param comp_func
 * @return whether the whole vector is removed
 */
template <typename KeyT, typename VecT, typename ObjT, typename CompFuncT>
bool MapVecRemove(std::unordered_map<KeyT,std::vector<VecT>> &map_container,
             const KeyT& key, ObjT obj, CompFuncT&& comp_func){
    auto itr = map_container.find(key);
    AssertLog(itr!=map_container.end(), "not found");

    AssertLog(!itr->second.empty(), "exist empty vector");
    auto itr2 = find_if(itr->second.begin(), itr->second.end(), comp_func);
    AssertLog(itr2!=itr->second.end(), "not found");

    if (itr->second.size()==1){
        map_container.erase(itr);
        return true;
    }
    else{
        itr->second.erase(itr2);
        return false;
    }
}

/**
 * Similar to MapVecRemove, but returns the removed element
 * @tparam KeyT
 * @tparam VecT
 * @tparam ObjT
 * @tparam CompFuncT
 * @param map_container
 * @param key
 * @param obj
 * @param comp_func
 * @return <whether the whole vector is removed, removed element>
 */
template <typename KeyT, typename VecT, typename ObjT, typename CompFuncT>
std::tuple<bool,VecT> MapVecMove(std::unordered_map<KeyT,std::vector<VecT>> &map_container,
                  const KeyT& key, ObjT obj, CompFuncT&& comp_func){
    auto itr = map_container.find(key);
    AssertLog(itr!=map_container.end(), "not found");

    AssertLog(!itr->second.empty(), "exist empty vector");
    auto itr2 = find_if(itr->second.begin(), itr->second.end(), comp_func);
    AssertLog(itr2!=itr->second.end(), "not found");

    auto out = std::make_tuple(itr->second.size()==1, std::move(*itr2));
    if (itr->second.size()==1){
        map_container.erase(itr);
    }
    else{
        itr->second.erase(itr2);
    }
    return out;
}

// template <typename Key1, typename Key2, typename Value>
// struct UMap2Keys{
//     std::unordered_map<Key1, Value> map1;
//     std::unordered_map<Key2, Value> map2;
//     std::unordered_map<Value, std::tuple<Key1,Key2>> r_map;
//
// };
//
// template <typename Key1, typename Key2, typename Key3, typename Value>
// struct UMap3Keys{
//
// };


inline bool StrStartWith(const std::string &full, const std::string &head){
    return strncmp(full.c_str(), head.c_str(), head.size()) == 0;
}

inline bool StrStartWith(const char *full, const char *head){
    return strncmp(full, head, strlen(head)) == 0;
}

/*
 * string concatenation attempt.
 * weird result in mac m1, all these are just slower. also: reserve then append is slower than just append
 */

/*
// just slow as well
inline size_t StrViewCatGetSize(std::string_view s){
    return s.size();
}

template<typename... Args>
inline size_t StrViewCatGetSize(std::string_view s1, Args... args){
    return s1.size() + StrViewCatGetSize(args...);
}

inline void StrViewCatAppend(std::string &out, std::string_view s1){
    out.append(s1);
}

template<typename... Args>
inline void StrViewCatAppend(std::string &out, std::string_view s1, Args... args){
    out.append(s1);
    StrViewCatAppend(out, args...);
}

template<typename... Args>
inline std::string StrViewCat2(std::string_view s1, Args... args){
    std::string out;
    out.reserve(StrViewCatGetSize(s1, args...));
    StrViewCatAppend(out, s1, args...);
    return out;
}

// just slow as well
// https://stackoverflow.com/a/18899027
template<typename>
struct string_size_impl;

template<size_t N>
struct string_size_impl<const char[N]> {
    static constexpr size_t size(const char (&) [N]) { return N - 1; }
};

template<size_t N>
struct string_size_impl<char[N]> {
    static size_t size(char (&s) [N]) { return N ? strlen(s) : 0; }
};

template<>
struct string_size_impl<const char*> {
    static size_t size(const char* s) { return s ? strlen(s) : 0; }
};

template<>
struct string_size_impl<char*> {
    static size_t size(char* s) { return s ? strlen(s) : 0; }
};

template<>
struct string_size_impl<std::string> {
    static size_t size(const std::string& s) { return s.size(); }
};

template<typename String> size_t string_size(String&& s) {
    using noref_t = typename std::remove_reference<String>::type;
    using string_t = typename std::conditional<std::is_array<noref_t>::value,
                                               noref_t,
                                               typename std::remove_cv<noref_t>::type
    >::type;
    return string_size_impl<string_t>::size(s);
}

template<typename...>
struct concatenate_impl;

template<typename String>
struct concatenate_impl<String> {
    static size_t size(String&& s) { return string_size(s); }
    static void concatenate(std::string& result, String&& s) { result += s; }
};

template<typename String, typename... Rest>
struct concatenate_impl<String, Rest...> {
    static size_t size(String&& s, Rest&&... rest) {
        return string_size(s)
                + concatenate_impl<Rest...>::size(std::forward<Rest>(rest)...);
    }
    static void concatenate(std::string& result, String&& s, Rest&&... rest) {
        result += s;
        concatenate_impl<Rest...>::concatenate(result, std::forward<Rest>(rest)...);
    }
};

template<typename... Strings>
std::string concatenate(Strings&&... strings) {
    std::string result;
    result.reserve(concatenate_impl<Strings...>::size(std::forward<Strings>(strings)...));
    concatenate_impl<Strings...>::concatenate(result, std::forward<Strings>(strings)...);
    return result;
}
*/

}
#endif //RUM_COMMON_MISC_H_
