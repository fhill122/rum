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

}
#endif //RUM_COMMON_MISC_H_
