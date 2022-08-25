/*
 * Created by Ivan B on 2022/1/7.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>
#include <rum/cpprum/serialization/protobuf/serializer_proto.h>
#include <rum/cpprum/rum.h>
#include <google/protobuf/util/message_differencer.h>

#include "../test_msg/protobuf/test_number.pb.h"

using namespace rum;
using namespace std;
using test::proto::Number;

string argv0;

TEST(SerProtoTest, FileIO){
    SerializerProto serializer;
    constexpr char kPath[] = "proto_number.data";

    Number number;
    number.set_n1(3);
    number.set_d2(3.14);

    bool export_ok = serializer.exportToFile(kPath, number);
    ASSERT_TRUE(export_ok);

    Number number_2;
    ASSERT_FALSE(google::protobuf::util::MessageDifferencer::Equals(number, number_2));

    bool import_ok = serializer.importFromFile(kPath, number_2);
    ASSERT_TRUE(import_ok);

    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, number_2));
}

struct SimplePbNode : public ::testing::Test{
    static constexpr char kTopic[] = "TestTopic";
    Publisher<Number>::UniquePtr pub;
    Subscriber::UniquePtr sub;
    atomic_int msg_count{0};
    shared_ptr<const Number> received;
    Number number;

    void init(){
        number.set_n1(9); number.set_d2(0.7);
        pub = rum::CreatePublisher<Number, SerializerProto>(kTopic);
        sub = rum::CreateSubscriber<Number, SerializerProto>(kTopic,
                            [this](const shared_ptr<const Number> &msg){ subCallback(msg);});
    }

    void subCallback(const shared_ptr<const Number> &msg){
        msg_count.fetch_add(1, std::memory_order_relaxed);
        received = msg;
    }
};

TEST_F(SimplePbNode, IntraProcMsg){
    init();
    int count = msg_count.load(std::memory_order_relaxed);

    pub->pub(make_unique<Number>(number));
    this_thread::sleep_for(50ms);
    EXPECT_EQ(msg_count.load(memory_order_relaxed),++count);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, *received));

    pub->pub(make_shared<Number>(number));
    this_thread::sleep_for(50ms);
    EXPECT_EQ(msg_count.load(memory_order_relaxed),++count);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, *received));

    pub->pub(make_shared<const Number>(number));
    this_thread::sleep_for(50ms);
    EXPECT_EQ(msg_count.load(memory_order_relaxed),++count);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, *received));

    pub->pub(Number(number));
    this_thread::sleep_for(50ms);
    EXPECT_EQ(msg_count.load(memory_order_relaxed),++count);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, *received));

    pub->pub(number);
    this_thread::sleep_for(50ms);
    EXPECT_EQ(msg_count.load(memory_order_relaxed),++count);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, *received));
}

TEST_F(SimplePbNode, InterProcMsg) {
    init();
    system(string(argv0 + "_companion").c_str());

    EXPECT_EQ(msg_count.load(memory_order_relaxed), 10);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(number, *received));
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}