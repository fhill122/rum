/*
 * Created by Ivan B on 2022/1/7.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>
#include <rum/cpprum/serialization/protobuf/serializer_proto.h>
#include <google/protobuf/util/message_differencer.h>
#include "../test_msg/protobuf/test_number.pb.h"

using namespace rum;

// todo ivan.
//  1. file io (done)
//  2. itc pub/sub
//  3. ipc pub/sub

TEST(SerProtoTest, FileIO){
    SerializerProto serializer;
    constexpr char kPath[] = "proto_number.data";

    test::proto::Number number;
    number.set_n1(3);
    number.set_d2(3.14);

    bool export_ok = serializer.exportToFile(kPath, number);
    ASSERT_TRUE(export_ok);

    test::proto::Number number_2;
    ASSERT_FALSE(google::protobuf::util::MessageDifferencer::Equals(number, number_2));

    bool import_ok = serializer.importFromFile(kPath, number_2);
    ASSERT_TRUE(import_ok);

    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, number_2));
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}