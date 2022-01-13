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
        const test::msg::Image* img = test::msg::GetImage(msg->data());
        log.w(__func__, "id %d, %dx%d, %s", img->frame_id(), img->w(), img->h(),
              img->format()->c_str());
        log.i(__func__, "shared count: %d", msg.use_count());
        received_msgs.push_back(msg);
        log.i(__func__, "shared count: %d", msg.use_count());
    }

    unique_ptr<flatbuffers::FlatBufferBuilder> createImage(){
        static int id=0;

        vector<int8_t> img_data = {1,2,3,4};
        auto builder = make_unique<flatbuffers::FlatBufferBuilder>();
        auto img = test::msg::CreateImageDirect(*builder, id++,2,2, "fake", &img_data);
        builder->Finish(img);
        return builder;
    }
};


TEST_F(SimpleFbNode, ItcTest){
    rum::Init();
    init();

    constexpr int kNum = 10;

    for (int i = 0; i < kNum; ++i) {
        pub.pub(createImage());
        this_thread::sleep_for(10ms);
    }

    EXPECT_EQ(received_msgs.size(), kNum);
    // todo ivan. doing here
    for (int i = 0; i < kNum; ++i) {
        log.i(__func__, "shared count: %d", received_msgs[i].use_count());

        const test::msg::Image* img = test::msg::GetImage(received_msgs[i]->data());
        log.w(__func__, "received msg of size %zu", received_msgs[i]->size());
        // EXPECT_EQ(i, img->frame_id());
        log.w(__func__, "%d: %d, %dx%d, %s", i, img->frame_id(), img->w(), img->h(),
              img->format()->c_str());
    }
}


int main(int argc, char **argv){
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
