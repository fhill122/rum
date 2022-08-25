/*
 * Created by Ivan B on 2022/1/7.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/cpprum/serialization/protobuf/serializer_proto.h>
#include <rum/cpprum/rum.h>

#include "../test_msg/protobuf/test_number.pb.h"

using namespace rum;
using namespace std;
using test::proto::Number;


int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::i);
    rum::Init();

    static constexpr char kTopic[] = "TestTopic";
    auto pub = rum::CreatePublisher<Number, SerializerProto>(kTopic);

    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
    Log::I(__func__, "to publish");
    for (int i = 0; i < 10; ++i) {
        Number number;
        number.set_n1(9); number.set_d2(0.7);
        pub->pub(move(number));
        this_thread::sleep_for(10ms);
    }

    this_thread::sleep_for(20ms);
    Log::I(__func__, "end");
}