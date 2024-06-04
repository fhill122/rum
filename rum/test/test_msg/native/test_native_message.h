/*
 * Created by Ivan B on 2022/1/14.
 */

#ifndef RUM_TEST_TEST_MSG_TEST_NATIVE_MESSAGE_H_
#define RUM_TEST_TEST_MSG_TEST_NATIVE_MESSAGE_H_

#include "rum/cpprum/serialization/native/common_types.h"

namespace rum{

// example 1: trivial type
struct TrivialData{
    enum Level{
        a,b,c
    };
    Level l;
    int x, y;
    char data[3];
};

// example 2: derived from HandwrittenSerialization
struct AutoDerived : HandwrittenSerialization{
    // not trivial
    const static int default_val = 0;

    unsigned int id;
    std::string name;
    TrivialData trivial_data;

    AUTO_SERIALIZE_MEMBERS(name, trivial_data, id)
};


// example 3: common_types.h
struct Predefinded : HandwrittenSerialization{
    std::string name;
    int id;
    std::unique_ptr<TrivialData> trivial_data;
    std::array<double,3> xyz;
    std::vector<char> data;
    std::map<int,int> map_data;

    AUTO_SERIALIZE_MEMBERS(name, id, trivial_data, xyz, data, map_data)
};


// example 4: just like option 3, we define our own serialization rule
struct CustomDefined{
    int x;
    int y;
    std::mutex mu;
    CustomDefined(){ x=4;}
};

AUTO_SERIALIZE_CLASS(CustomDefined, obj, obj.x, obj.y)

// equal operator we use for this test
inline bool operator==(const TrivialData& lhs, const TrivialData& rhs){
    return lhs.l==rhs.l && lhs.x==rhs.x && lhs.y==rhs.y &&
            lhs.data[0]==rhs.data[0] &&
            lhs.data[1]==rhs.data[1] &&
            lhs.data[2]==rhs.data[2];
}

inline bool operator==(const AutoDerived& lhs, const AutoDerived& rhs){
    return lhs.name == rhs.name && lhs.id == rhs.id && lhs.trivial_data == rhs.trivial_data;
}

inline bool operator==(const Predefinded& lhs, const Predefinded& rhs){
    bool res = false;
    if (!lhs.trivial_data && !rhs.trivial_data){
        res = true;
    }
    else if (lhs.trivial_data && rhs.trivial_data){
        res = *lhs.trivial_data == *rhs.trivial_data;
    }

    return res && lhs.name == rhs.name && lhs.id == rhs.id && lhs.xyz == rhs.xyz &&
            lhs.data==rhs.data && lhs.map_data==rhs.map_data;
}

inline bool operator==(const CustomDefined& lhs, const CustomDefined& rhs){
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

}


#endif //RUM_TEST_TEST_MSG_TEST_NATIVE_MESSAGE_H_
