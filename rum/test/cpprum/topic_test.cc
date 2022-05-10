/*
 * Created by Ivan B on 2022/1/9.
 */
#include <gtest/gtest.h>

#include <rum/cpprum/rum.h>
#include <rum/common/log.h>

#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_image_generated.h"

#include <rum/serialization/native/common_types.h>
#include <rum/serialization/native/serializer_native.h>
#include "../test_msg/test_native_message.h"

using namespace std;
using namespace rum;

string argv0;


struct SimpleFbNode : public ::testing::Test{
  public:
    static constexpr char kTopic[] = "TestTopic";
    Publisher<flatbuffers::FlatBufferBuilder>::UniquePtr pub;
    Subscriber::UniquePtr sub;
    vector<shared_ptr<const Message>> received_msgs;

    SimpleFbNode() {}
    ~SimpleFbNode(){
        Log::I(__func__, __func__ );
    }

    void init(){
        pub = rum::CreatePublisher<flatbuffers::FlatBufferBuilder, SerializerFbs>(kTopic);
        sub = rum::CreateSubscriber<Message, SerializerFbs>(kTopic,
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

TEST_F(SimpleFbNode, IntraProcTest){
    init();

    constexpr int kNum = 10;

    for (int i = 0; i < kNum; ++i) {
        pub->pub(CreateImage());
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

struct SimpleNativeNode : public ::testing::Test{
  public:
    static constexpr char kTopic[] = "TestTopic";
    int id_pool = 0;
    Publisher<Predefinded>::UniquePtr pub;
    Subscriber::UniquePtr sub;
    vector<shared_ptr<const Predefinded>> received_msgs;
    vector<shared_ptr<const Predefinded>> published_msgs;

    SimpleNativeNode() {}
    ~SimpleNativeNode(){
        // rum::RemovePublisher(pub);
        // rum::RemoveSubscriber(sub);
    }

    void init(){
        pub = rum::CreatePublisher<Predefinded, SerializerNative>(kTopic);
        sub = rum::CreateSubscriber<Predefinded, SerializerNative>(kTopic,
                [this](const shared_ptr<const Predefinded> &msg) {
                   subCallback(msg);
                });
    }

    void subCallback(const shared_ptr<const Predefinded> &msg){
        received_msgs.push_back(msg);
    }

    unique_ptr<Predefinded> createObject(){
        auto obj = make_unique<Predefinded>();
        obj->name = "lalala";
        obj->id = id_pool++;
        obj->xyz = {1.5, 6.66, 3.14159};
        obj->data = {'a','b', 'c'};
        obj->trivial_data = make_unique<TrivialData>();
        obj->trivial_data->l = TrivialData::Level::b;
        obj->trivial_data->x = 5;
        obj->trivial_data->y = 6;
        obj->data[0] = 'y';
        obj->data[1] = 'u';
        obj->data[2] = 'v';

        return obj;
    }

    void checkObject(){
        for (int i = 0; i < received_msgs.size(); ++i) {
            EXPECT_EQ(*published_msgs[i], *received_msgs[i]);
        }
    }
};

TEST_F(SimpleNativeNode, IntraProcTest){
    init();

    constexpr int kNum = 10;

    for (int i = 0; i < kNum; ++i) {
        shared_ptr<const Predefinded> obj = createObject();
        pub->pub(obj);
        published_msgs.push_back(move(obj));
        this_thread::sleep_for(10ms);
    }

    EXPECT_EQ(received_msgs.size(), kNum);
    checkObject();
}

TEST_F(SimpleNativeNode, IpcTest){
    init();

    constexpr int kNum = 10;
    for (int i = 0; i < kNum; ++i) {
        published_msgs.push_back(createObject());
    }

    string cmd = argv0 + "_companion " + "SimpleNativeNode/IpcTest";
    system(cmd.c_str());
    // this_thread::sleep_for(100ms);

    EXPECT_EQ(received_msgs.size(), 10);
    checkObject();
}

int main(int argc, char **argv){
    rum::Init();
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
