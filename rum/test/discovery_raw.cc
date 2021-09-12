//
// Created by Ivan B on 2021/4/4.
//

#include <gtest/gtest.h>

#include <rum/core/internal/master.h>
#include <rum/core/internal/sub_container.h>
#include <rum/core/internal/publisher_base_impl.h>
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/core/msg/rum_header_generated.h>

using namespace std;
using namespace rum;

struct SimpleNode{
    unique_ptr<SubContainer> sub_container;
    unique_ptr<PublisherBaseImpl> sync_pub;
    unique_ptr<SubscriberBaseImpl> sync_sub;
    shared_ptr<ivtb::ThreadPool> tp = make_shared<ivtb::ThreadPool>(1);
    atomic<int> sync_count{0};

    explicit SimpleNode(const shared_ptr<zmq::context_t>& context){
        sub_container = make_unique<SubContainer>(context, false);
        sub_container->connectRaw(GetMasterOutAddr());
        sub_container->start();

        sync_sub = make_unique<SubscriberBaseImpl>(kSyncTopic, tp, 100,
            bind(&SimpleNode::syncCb, this, placeholders::_1), nullptr);
        sub_container->addSub(sync_sub.get());

        sync_pub = make_unique<PublisherBaseImpl>(kSyncTopic, "", context, false);
        sync_pub->connect(GetMasterInAddr());
    }

    ~SimpleNode(){
        // sub_container->stop();
    }

    void syncCb(zmq::message_t& msg){
        string str((char*)msg.data(), msg.size());
        rum::log.v(__func__, "receive %s", str.c_str());
        sync_count++;
    }

    void pubString(const string& str) const{
        flatbuffers::FlatBufferBuilder header_builder;
        auto header_fb = msg::CreateMsgHeaderDirect(header_builder,
                        msg::MsgType_Message, kSyncTopic, "string");
        header_builder.Finish(header_fb);

        zmq::message_t header_msg(header_builder.GetBufferPointer(), header_builder.GetSize());
        zmq::message_t body_msg(str.data(), str.size());
        sync_pub->publishIpc(header_msg, body_msg);
    }
};

TEST(DicoveryTest, Discovery){
    auto zmq_context = make_shared<zmq::context_t>(1);

    // master
    //  Master master1(zmq_context);
    //  Master master2(zmq_context);
    // nodes
    SimpleNode node_a(zmq_context);
    SimpleNode node_b(zmq_context);
    usleep(1e6);

    // simulate sync pub
    node_a.pubString("sync broadcast from a");
    usleep(1e5);

    ASSERT_EQ(node_a.sync_count.load(), 1);
    ASSERT_EQ(node_b.sync_count.load(), 1);
    rum::log.v(__func__, "test end");
    SUCCEED();
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Levels::v);

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}
