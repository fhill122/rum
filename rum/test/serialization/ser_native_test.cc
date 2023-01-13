/*
 * Created by Ivan B on 2021/12/22.
 */
#include <gtest/gtest.h>
#include <rum/common/log.h>

#include <rum/cpprum/serialization/native/common_types.h>

namespace rum{
    // write additional template custom type serialization here
}

#include <rum/cpprum/serialization/native/serializer_native.h>

#include "../test_msg/native/test_native_message.h"

using namespace std;
using namespace rum;


TEST(Native, TrivialDataTest){
    SerializerNative serializer;

    auto obj = make_shared<TrivialData>();
    obj->l = obj->b;
    obj->x = 3; obj->y =4;
    obj->data[0] = 6; obj->data[1] = 6; obj->data[2] = 6;

    shared_ptr<const Message> msg = serializer.serialize(static_pointer_cast<const TrivialData>(obj));

    auto obj2 = serializer.deserialize<TrivialData>(msg, serializer.Protocol());

    ASSERT_EQ(*obj, *obj2);
}

TEST(Native, AutoDerivedTest){
    SerializerNative serializer;

    auto obj = make_shared<AutoDerived>();
    obj->trivial_data.x = 9;
    obj->id = 3;
    obj->name = "lala";

    shared_ptr<const Message> msg = serializer.serialize(static_pointer_cast<const AutoDerived>(obj));

    auto obj2 = serializer.deserialize<AutoDerived>(msg, serializer.Protocol());

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

    shared_ptr<const Message> msg = serializer.serialize(static_pointer_cast<const Predefinded>(obj));

    auto obj2 = serializer.deserialize<Predefinded>(msg, serializer.Protocol());

    ASSERT_EQ(*obj, *obj2);
}

TEST(Native, CustomDefined){
    SerializerNative serializer;

    auto obj = make_shared<CustomDefined>();
    obj->x = 13;

    shared_ptr<const Message> msg = serializer.serialize(static_pointer_cast<const CustomDefined>(obj));

    auto obj2 = serializer.deserialize<CustomDefined>(msg, serializer.Protocol());

    ASSERT_EQ(*obj, *obj2);
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
