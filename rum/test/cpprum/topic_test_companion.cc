/*
 * Created by Ivan B on 2022/1/10.
 */
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/cpprum/rum.h>
#include "../test_msg/flatbuffers/test_image_generated.h"

#include <rum/cpprum/serialization/native/common_types.h>
#include <rum/cpprum/serialization/native/serializer_native.h>
#include "../test_msg/native/test_native_message.h"


using namespace std;
using namespace rum;

constexpr char kTag[] = "companion";

struct SimpleFbNode{
  public:
    static constexpr char kTopic[] = "TestTopic";
    Publisher<flatbuffers::FlatBufferBuilder>::UniquePtr pub;

    SimpleFbNode() {}
    ~SimpleFbNode(){
        // rum::RemovePublisher(pub);
    }

    void init(){
        pub = rum::CreatePublisher<flatbuffers::FlatBufferBuilder, SerializerFbs>(kTopic);
    }

    static unique_ptr<flatbuffers::FlatBufferBuilder> CreateImage(){
        static int id=0;

        vector<int8_t> img_data = {1,2,3,4};
        auto builder = make_unique<flatbuffers::FlatBufferBuilder>();
        auto img = test::msg::CreateImageDirect(*builder, id++,2,2, "fake", &img_data);
        builder->Finish(img);
        return builder;
    }
};

void SimpleFbNode_IpcTest(){
    Log::I(kTag, "start");
    SimpleFbNode fb_node;
    fb_node.init();
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);

    Log::I(kTag, "fire");
    constexpr int kNum = 10;
    for (int i = 0; i < kNum; ++i) {
        fb_node.pub->pub(SimpleFbNode::CreateImage());
        this_thread::sleep_for(10ms);
    }
    this_thread::sleep_for(20ms);

    Log::I(kTag, "end");
}

struct SimpleNativeNode {
  public:
    static constexpr char kTopic[] = "TestTopic";
    int id_pool = 0;
    Publisher<Predefinded>::UniquePtr pub;

    SimpleNativeNode() {}

    ~SimpleNativeNode(){
        // rum::RemovePublisher(pub);
    }

    void init(){
        pub = rum::CreatePublisher<Predefinded, SerializerNative>(kTopic);
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
};

void SimpleNativeNode_IpcTest(){
    Log::I(kTag, "start");
    SimpleNativeNode native_node;
    native_node.init();
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);

    Log::I(kTag, "fire");
    constexpr int kNum = 10;
    for (int i = 0; i < kNum; ++i) {
        native_node.pub->pub(native_node.createObject());
        this_thread::sleep_for(10ms);
    }
    this_thread::sleep_for(20ms);

    Log::I(kTag, "end");
}

int main(int argc, char* argv[]){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::i);
    rum::Init();

    AssertLog(argc>1, "input command");
    string cmd = argv[1];
    if (cmd == "SimpleFbNode/IpcTest"){
        SimpleFbNode_IpcTest();
    }else if (cmd == "SimpleNativeNode/IpcTest"){
        SimpleNativeNode_IpcTest();
    }
    else {
        AssertLog(false, "no command found");
    }
}