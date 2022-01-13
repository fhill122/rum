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


struct SimpleFbNode : public ::testing::Test{
  public:
    static constexpr char kTopic[] = "TestTopic";
    PublisherHandler<flatbuffers::FlatBufferBuilder, SerializerFbs> pub;
    SubscriberHandler<SerializerFbs> sub;
    vector<shared_ptr<const Message>> received_msgs;

    SimpleFbNode() {}
    ~SimpleFbNode(){
        rum::RemovePublisher(pub);
        rum::RemoveSubscriber(sub);
    }

    void init(){
        pub = rum::AddPublisher<flatbuffers::FlatBufferBuilder, SerializerFbs>(kTopic);
        sub = rum::AddSubscriber<Message, SerializerFbs>(kTopic,
                [this](const shared_ptr<const Message> &msg){ subCallback(msg);});
    }

    void subCallback(const shared_ptr<const Message> &msg){
        received_msgs.push_back(msg);
    }

    static unique_ptr<flatbuffers::FlatBufferBuilder> CreateImage(){
        static int id=0;

        vector<int8_t> img_data = {1,2,3,4};
        auto builder = make_unique<flatbuffers::FlatBufferBuilder>();
        auto img = test::msg::CreateImageDirect(*builder, id++,2,2, "fake", &img_data);
        builder->Finish(img);
        return builder;
    }

     void checkImage(){
        for (int i = 0; i < received_msgs.size(); ++i) {
            const test::msg::Image* img = test::msg::GetImage(received_msgs[i]->data());
            EXPECT_EQ(img->frame_id(), i);
            EXPECT_EQ(img->w(), 2);
            EXPECT_EQ(img->h(), 2);
            EXPECT_EQ(img->data()->Get(0), int8_t(1));
            EXPECT_EQ(img->data()->Get(1), int8_t(2));
            EXPECT_EQ(img->data()->Get(2), int8_t(3));
            EXPECT_EQ(img->data()->Get(3), int8_t(4));
        }
    }
};

TEST_F(SimpleFbNode, ItcTest){
    init();

    constexpr int kNum = 10;

    for (int i = 0; i < kNum; ++i) {
        pub.pub(CreateImage());
        this_thread::sleep_for(10ms);
    }

    EXPECT_EQ(received_msgs.size(), kNum);
    checkImage();
}

TEST_F(SimpleFbNode, IpcTest){
    init();

    string cmd = argv0 + "_companion " + "SimpleFbNode/IpcTest";
    system(cmd.c_str());
    // this_thread::sleep_for(100ms);

    EXPECT_EQ(received_msgs.size(), 10);
    checkImage();
}

int main(int argc, char **argv){
    rum::Init();

    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
