//
// Created by Ivan B on 2021/8/7.
//

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <rum/common/log.h>
#include <rum/core/internal/node_base_impl.h>
#include <rum/common/common.h>

using namespace std;
using namespace rum;

string argv0;

class ImplTest : public ::testing::Test{
  private:
  public:
    static constexpr char kTopic[] = "TestTopic";
    static constexpr char kProtocol[] = "protocol";
    unique_ptr<NodeBaseImpl> node_;
    PublisherBaseImpl *pub_;
    SubscriberBaseImpl *sub_ = nullptr;

  private:

  public:
    ImplTest() {}

    void init(NodeBaseImpl::Param param = NodeBaseImpl::Param()){
        node_ = make_unique<NodeBaseImpl>("", move(param));
        node_->connect(GetMasterInAddr(), GetMasterOutAddr());
        this_thread::sleep_for(50ms);
        pub_ = node_->addPublisher(kTopic, kProtocol);
    }

};

TEST_F(ImplTest, ItcBasic){
    init();
    int ipc_count = 0;
    int itc_count = 0;
    node_->addSubscriber(kTopic, make_shared<ivtb::ThreadPool>(1), 100,
                                  [&](zmq::message_t&){ipc_count++;},
                                  [&](const void*){itc_count++;}, kProtocol);

    this_thread::sleep_for(50ms);
    for (int i = 0; i < 10; ++i) {
        pub_->publishIpc(zmq::message_t(1));
        auto str_ptr = make_shared<string>("fdd");
        pub_->scheduleItc(str_ptr);
    }

    this_thread::sleep_for(50ms);
    EXPECT_EQ(itc_count, 10);
    EXPECT_EQ(ipc_count, 0);
}

TEST_F(ImplTest, IpcBasic) {
    init();
    const testing::TestInfo* const test_info =
            testing::UnitTest::GetInstance()->current_test_info();

    int ipc_count = 0;
    int itc_count = 0;
    node_->addSubscriber(kTopic, make_shared<ivtb::ThreadPool>(1), 100,
                        [&](zmq::message_t&){ipc_count++;},
                        [&](const void*){itc_count++;}, kProtocol);
    printf("start companion!\n");
    string cmd = argv0 + "_companion " + test_info->name();
    // this_thread::sleep_for(100ms);
    system(cmd.c_str());
    EXPECT_EQ(itc_count, 0);
    EXPECT_EQ(ipc_count, 10);
    // this_thread::sleep_for(100ms);
}

TEST_F(ImplTest, TcpBasic) {
    NodeBaseImpl::Param param;
    param.enable_ipc_socket = false;
    init(param);
    const testing::TestInfo* const test_info =
            testing::UnitTest::GetInstance()->current_test_info();

    int ipc_count = 0;
    int itc_count = 0;
    node_->addSubscriber(kTopic, make_shared<ivtb::ThreadPool>(1), 100,
                         [&](zmq::message_t&){ipc_count++;},
                         [&](const void*){itc_count++;}, kProtocol);
    printf("start companion!\n");
    string cmd = argv0 + "_companion " + test_info->name();
    // this_thread::sleep_for(100ms);
    system(cmd.c_str());
    EXPECT_EQ(itc_count, 0);
    EXPECT_EQ(ipc_count, 10);
    // this_thread::sleep_for(100ms);
}

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
