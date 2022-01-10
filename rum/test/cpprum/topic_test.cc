/*
 * Created by Ivan B on 2022/1/9.
 */
#include <gtest/gtest.h>
#include <glog/logging.h>

#include <rum/cpprum/rum.h>
#include <rum/common/log.h>
#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_image_generated.h"

using namespace std;
using namespace rum;

string argv0;

struct SimpleNode : public ::testing::Test{
  public:
    static constexpr char kTopic[] = "TestTopic";
    PublisherHandler<SerializerFbs, flatbuffers::FlatBufferBuilder> pub;
    SubscriberHandler<SerializerFbs, test::msg::Image> sub;

    SimpleNode() {}
};


int main(int argc, char **argv){
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::w);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
