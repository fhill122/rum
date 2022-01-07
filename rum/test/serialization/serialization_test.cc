/*
 * Created by Ivan B on 2021/12/22.
 */
#include <gtest/gtest.h>
#include <rum/common/log.h>

#include <rum/serialization/native/handwritten.h>
#include <rum/serialization/native/common_types.h>

namespace rum{
    // write additional template custom type serialization here
    // what if it depends on common_types.h ?
}

#include <rum/serialization/native/serializer_native.h>

using namespace std;
using namespace rum;

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
    string name;
    TrivialData trivial_data;

    AUTO_SERIALIZE_MEMBERS(name, trivial_data, id)
};


// example 3: contains either trivial data or types with predefined rules (provided in common_types.h)
struct Predefinded : HandwrittenSerialization{
    string name;
    int id;
    array<double,3> xyz;
    vector<char> data;
    unique_ptr<TrivialData> trivial_data;

    AUTO_SERIALIZE_MEMBERS(name, id, xyz, data, trivial_data)
};


// example 4: just like option 3, we define our own serialization rule
struct CustomDefined{
    int x;
    mutex mu;
    CustomDefined(){ x=4;}
};

size_t GetSerializationSize(const CustomDefined &s) { return sizeof(s.x);}

void Serialize(char* buffer, const CustomDefined &s){
    *(int*)buffer = s.x;
}

void Deserialize(const char* buffer, CustomDefined &s){
    s.x = *(int*)buffer;
}

// equal operator we use for this test
bool operator==(const TrivialData& lhs, const TrivialData& rhs){
    return lhs.l==rhs.l && lhs.x==rhs.x && lhs.y==rhs.y &&
            lhs.data[0]==rhs.data[0] &&
            lhs.data[1]==rhs.data[1] &&
            lhs.data[2]==rhs.data[2];
}

bool operator==(const AutoDerived& lhs, const AutoDerived& rhs){
    return lhs.name == rhs.name && lhs.id == rhs.id && lhs.trivial_data == rhs.trivial_data;
}

bool operator==(const Predefinded& lhs, const Predefinded& rhs){
    bool res = false;
    if (!lhs.trivial_data && !rhs.trivial_data){
        res = true;
    }
    else if (lhs.trivial_data && rhs.trivial_data){
        res = *lhs.trivial_data == *rhs.trivial_data;
    }

    return res && lhs.name == rhs.name && lhs.id == rhs.id && lhs.xyz == rhs.xyz && lhs.data==rhs.data;
}

bool operator==(const CustomDefined& lhs, const CustomDefined& rhs){
    return lhs.x == rhs.x;
}


TEST(Native, TrivialDataTest){
    SerializerNative serializer;

    auto obj = make_shared<TrivialData>();
    obj->l = obj->b;
    obj->x = 3; obj->y =4;
    obj->data[0] = 6; obj->data[1] = 6; obj->data[2] = 6;

    auto msg = serializer.serialize(static_pointer_cast<const TrivialData>(obj));

    auto obj2 = serializer.deserialize<TrivialData>(*msg, serializer.protocol());

    ASSERT_EQ(*obj, *obj2);
}

TEST(Native, AutoDerivedTest){
    SerializerNative serializer;

    auto obj = make_shared<AutoDerived>();
    obj->trivial_data.x = 9;
    obj->id = 3;
    obj->name = "lala";

    auto msg = serializer.serialize(static_pointer_cast<const AutoDerived>(obj));

    auto obj2 = serializer.deserialize<AutoDerived>(*msg, serializer.protocol());

    ASSERT_EQ(*obj, *obj2);
}

TEST(Native, PredefindedTest){
    SerializerNative serializer;

    auto obj = make_shared<Predefinded>();
    obj->trivial_data = make_unique<TrivialData>();
    obj->trivial_data->x = 8;
    obj->name = "yo";
    obj->data = {'a','b', 'c'};
    obj->xyz = {1.5, 6.66, 3.14159};

    auto msg = serializer.serialize(static_pointer_cast<const Predefinded>(obj));

    auto obj2 = serializer.deserialize<Predefinded>(*msg, serializer.protocol());

    ASSERT_EQ(*obj, *obj2);
}

TEST(Native, CustomDefined){
    SerializerNative serializer;

    auto obj = make_shared<CustomDefined>();
    obj->x = 13;

    auto msg = serializer.serialize(static_pointer_cast<const CustomDefined>(obj));

    auto obj2 = serializer.deserialize<CustomDefined>(*msg, serializer.protocol());

    ASSERT_EQ(*obj, *obj2);
}
// todo ivan. doing here, sanitiser test, ...

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
