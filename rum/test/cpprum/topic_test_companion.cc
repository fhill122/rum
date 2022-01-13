/*
 * Created by Ivan B on 2022/1/10.
 */
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/cpprum/rum.h>
#include "../test_msg/test_image_generated.h"

using namespace std;
using namespace rum;

constexpr char kTag[] = "companion";

struct SimpleFbNode{
  public:
    static constexpr char kTopic[] = "TestTopic";
    PublisherHandler<flatbuffers::FlatBufferBuilder, SerializerFbs> pub;

    SimpleFbNode() {}
    ~SimpleFbNode(){
        rum::RemovePublisher(pub);
    }

    void init(){
        pub = rum::AddPublisher<flatbuffers::FlatBufferBuilder, SerializerFbs>(kTopic);
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
    rum::Init();
    fb_node.init();
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);

    Log::I(kTag, "fire");
    constexpr int kNum = 10;
    for (int i = 0; i < kNum; ++i) {
        fb_node.pub.pub(SimpleFbNode::CreateImage());
        this_thread::sleep_for(10ms);
    }
    // this_thread::sleep_for(100ms);

    Log::I(kTag, "end");
}

int main(int argc, char* argv[]){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::i);
    AssertLog(argc>1, "input command");
    string cmd = argv[1];
    if (cmd == "SimpleFbNode/IpcTest"){
        SimpleFbNode_IpcTest();
    }
    else {
        AssertLog(false, "no command found");
    }
}