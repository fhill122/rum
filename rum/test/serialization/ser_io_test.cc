/*
 * Created by Ivan B on 2022/7/10.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>

#include <rum/serialization/native/handwritten.h>
#include <rum/serialization/native/common_types.h>
#include <rum/serialization/native/serializer_native.h>

#include "../test_msg/test_native_message.h"

using namespace std;
using namespace rum;

TEST(NativeIO, SimpleIO){
    constexpr char kPath[] = "TrivialData.data";

    SerializerNative serializer;
    TrivialData obj;
    obj.l = obj.b;
    obj.x = 3; obj.y =4;
    obj.data[0] = 6; obj.data[1] = 6; obj.data[2] = 6;

    bool export_ok = serializer.exportToFile(kPath, obj);
    ASSERT_TRUE(export_ok);

    TrivialData obj2;
    bool import_ok = serializer.importFromFile(kPath, obj2);
    ASSERT_TRUE(import_ok);

    ASSERT_TRUE(obj==obj2);
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
