//
// Note: without introducing large amount of delay many test would fail if net interface is not 127.0.0.1,
// especially when running all test.
//
// Created by Ivan B on 2021/8/7.
//

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <rum/common/log.h>
#include <rum/core/internal/node_base_impl.h>
#include <rum/common/common.h>
#include <rum/core/internal/itc_manager.h>
#include <rum/extern/ivtb/stopwatch.h>

#include "../test_utils/process_utils.h"

using namespace std;
using namespace rum;
using namespace ivtb;

string argv0;

class ImplTest : public ::testing::Test{
  private:
  public:
    static constexpr char kTopic[] = "TestTopic";
    static constexpr char kProtocol[] = "protocol";
    unique_ptr<NodeBaseImpl> node_;
    PublisherBaseImpl *pub_ = nullptr;
    SubscriberBaseImpl *sub_ = nullptr;
    atomic_int ipc_count{0};
    atomic_int itc_count{0};

  private:

  public:
    ImplTest() {}

    void init(NodeParam param = NodeParam()){
        node_ = make_unique<NodeBaseImpl>("", move(param));
        node_->connect(GetMasterInAddr(), GetMasterOutAddr());
        // this_thread::sleep_for(50ms);
        pub_ = node_->addPublisher(kTopic, kProtocol);
        sub_ = node_->addSubscriber(kTopic, make_shared<ivtb::ThreadPool>(1), 100,
                             [&](const shared_ptr<const void> &){ipc_count.fetch_add(1,memory_order_relaxed);},
                             [&](const shared_ptr<const void> &){itc_count.fetch_add(1,memory_order_relaxed);},
                             [](shared_ptr<const Message> &msg, const string&){return move(msg);}, kProtocol);
    }

    virtual ~ImplTest() {
        // this is mandatory as ItcManager is global and contains all the history subs
        if (node_) node_->shutdown();
        this_thread::sleep_for(10ms);
    }
};

class ImplMultiTest : public ::testing::Test{
  private:
  public:
    const string kTopicPrefix = "TestTopic";
    const string kProtocol = "protocol";
    unique_ptr<NodeBaseImpl> node_;
    vector<vector<PublisherBaseImpl*>> pubs_;
    vector<vector<SubscriberBaseImpl*>> subs_;
    shared_ptr<ivtb::ThreadPool> tp = make_shared<ivtb::ThreadPool>(2);
    atomic_int shared_ipc_count{0};
    atomic_int shared_itc_count{0};

    void init(int n_topics, int n_subs, int n_pubs,
              NodeParam param = NodeParam()){
        node_ = make_unique<NodeBaseImpl>("", move(param));
        node_->connect(GetMasterInAddr(), GetMasterOutAddr());

        pubs_.resize(n_topics);
        subs_.resize(n_topics);
        for (int i=0; i<n_topics; ++i){
            string topic = kTopicPrefix + to_string(i);
            pubs_[i].resize(n_pubs);
            subs_[i].resize(n_subs);

            for(int j=0; j<n_pubs; ++j){
                pubs_[i][j] = node_->addPublisher(topic, kProtocol);
            }

            for(int j=0; j<n_subs; ++j){
                subs_[i][j] = node_->addSubscriber(topic, tp, 1000,
                        [&](const shared_ptr<const void> &){shared_ipc_count.fetch_add(1, memory_order_relaxed);},
                        [&](const shared_ptr<const void> &){shared_itc_count.fetch_add(1, memory_order_relaxed);},
                        [](shared_ptr<const Message> &msg, const string&){return move(msg);}, kProtocol);
            }
        }
    }

    virtual ~ImplMultiTest() {
        // this is mandatory as ItcManager is global and contains all the history subs
        if (node_) node_->shutdown();
        this_thread::sleep_for(10ms);
    }

};

TEST_F(ImplTest, ItcBasic){
    init();

    for (int i = 0; i < 10; ++i) {
        pub_->publishIpc(zmq::message_t(1));
        auto str_ptr = make_shared<string>("fdd");
        pub_->scheduleItc(str_ptr);
    }

    this_thread::sleep_for(50ms);
    EXPECT_EQ(itc_count.load(), 10);
    EXPECT_EQ(ipc_count.load(), 0);
}

TEST_F(ImplTest, IpcBasic) {
    init();

    string cmd = argv0 + "_companion " + "IpcBasic_both";
    system(cmd.c_str());
    EXPECT_EQ(itc_count.load(), 0);
    EXPECT_EQ(ipc_count.load(), 10);
}

TEST_F(ImplTest, TcpBasic) {
    init();

    string cmd = argv0 + "_companion " + "IpcBasic_tcp";
    system(cmd.c_str());
    EXPECT_EQ(itc_count.load(), 0);
    EXPECT_EQ(ipc_count.load(), 10);
}

TEST_F(ImplTest, TcpIcpMismatched) {
    NodeParam param;
    param.enable_ipc_txrx = false;
    init(move(param));

    string cmd = argv0 + "_companion " + "IpcBasic_ipc";
    system(cmd.c_str());
    EXPECT_EQ(itc_count.load(), 0);
    EXPECT_EQ(ipc_count.load(), 0);
}

TEST_F(ImplTest, ItcIpcTcpBasic) {
    init();

    string cmd1 = argv0 + "_companion " + "IpcBasic_both";
    string cmd2 = argv0 + "_companion " + "IpcBasic_tcp";
    auto t1 = thread([cmd1]{ LaunchProcess(cmd1);});
    auto t2 = thread([cmd2]{ LaunchProcess(cmd2);});

    int itc_n = kNodeHbPeriod+150;
    for (int i = 0; i < itc_n; ++i) {
        pub_->publishIpc(zmq::message_t(1));
        auto str_ptr = make_shared<string>("fdd");
        pub_->scheduleItc(str_ptr);
        this_thread::sleep_for(1ms);
    }

    t1.join();
    t2.join();
    EXPECT_EQ(itc_count.load(), itc_n);
    EXPECT_EQ(ipc_count.load(), 20);
}

// just easy to fail when run all, but pass when run single test or on 127.0.0.1 or sleep long enough
TEST_F(ImplTest, RemoteCrash){
    init();
    string cmd = argv0 + "_companion " + "RemoteCrash";
    thread companion_t([&]{system(cmd.c_str());});

    Stopwatch stopwatch;
    while(stopwatch.passedMs() < 5*(kNodeHbPeriod+50)){
        if (pub_->isConnected()){
            break;
        }
        std::this_thread::sleep_for(5ms);
    }
    EXPECT_TRUE(pub_->isConnected());

    companion_t.join();

    // sleep until the node is removed
    stopwatch.start();
    while(stopwatch.passedMs() < kNodeOfflineCriteria+kNodeOfflineCheckCounts*kNodeOfflineCheckPeriod){
        if (!pub_->isConnected()){
            break;
        }
        std::this_thread::sleep_for(5ms);
    }
    EXPECT_FALSE(pub_->isConnected());
}

// just easy to fail when run all, but pass when run single test or on 127.0.0.1 or sleep long enough
TEST_F(ImplTest, RemoteSubRemoval){
    init();
    string cmd = argv0 + "_companion " + "RemoteSubRemoval";
    thread companion_t([&]{system(cmd.c_str());});

    Stopwatch stopwatch;
    while(stopwatch.passedMs() < 4*(kNodeHbPeriod+50)){
        if (pub_->isConnected()){
            break;
        }
        std::this_thread::sleep_for(5ms);
    }
    EXPECT_TRUE(pub_->isConnected());

    companion_t.join();
    EXPECT_FALSE(pub_->isConnected());
}

// when launch all test, got err: Resource temporarily unavailable
// just easy to fail when run all, but pass when run single test or on 127.0.0.1 or sleep long enough
TEST_F(ImplMultiTest, MultiItcIpcTcp){
    constexpr int kNTopics = 10;
    constexpr int kNSubs = 5;
    constexpr int kNPubs = 10;

    init(kNTopics, kNSubs, kNPubs);

    string cmd1 = argv0 + "_companion " + "MultiIpc";
    string cmd2 = argv0 + "_companion " + "MultiTcp";
    auto t1 = thread([cmd1]{LaunchProcess(cmd1);});
    auto t2 = thread([cmd2]{LaunchProcess(cmd2);});

    int itc_n = kNodeHbPeriod+150;
    for (int i = 0; i < itc_n; ++i) {
        for (int j=0; j<kNTopics; ++j) {
            for (int k=0; k<kNPubs; ++k){
                auto str_ptr = make_shared<string>("fdd");
                pubs_[j][k]->scheduleItc(str_ptr);
            }
        }
        this_thread::sleep_for(1ms);
    }

    t1.join();
    t2.join();
    this_thread::sleep_for(10ms);
    EXPECT_EQ(shared_itc_count.load(), itc_n*kNSubs*kNTopics*kNPubs);
    EXPECT_EQ(shared_ipc_count.load(), 100*2*kNSubs*kNTopics*kNPubs);
}

struct LongSubTask  : public ::testing::Test {
    unique_ptr<NodeBaseImpl> node = make_unique<NodeBaseImpl>();
    PublisherBaseImpl *pub;
    vector<SubscriberBaseImpl*> subs;
    atomic_int count{0};
    static constexpr char kTopic[] = "SlowJob";
    static constexpr char kProtocol[] = "";

    LongSubTask() {
        node->connect(GetMasterInAddr(), GetMasterOutAddr());
        pub = node->addPublisher(kTopic, kProtocol);
    }

    virtual ~LongSubTask() {
        if(node) node->shutdown();
    }

    void initSubs(int n){
        shared_ptr<ThreadPool> tp = make_shared<ThreadPool>(1);
        subs.resize(n);
        for (int i = 0; i < n; ++i) {
            subs[i] = node->addSubscriber(kTopic, tp, 100, nullptr,
                    [&](const shared_ptr<const void> &){
                        Log::I(__func__, "start job");
                        this_thread::sleep_for(10ms);
                        count.fetch_add(1);
                        Log::I(__func__, "finish job");
                    },
                    nullptr, kProtocol);
        }
    }
};

TEST_F(LongSubTask, SingleTp){
    initSubs(1);

    for (int i = 0; i < 10; ++i) {
        pub->scheduleItc(make_shared<string>("fdd"));
    }
    this_thread::sleep_for(1ms);

    node->removeSubscriber(subs[0]);
    EXPECT_EQ(count.load(), 1);
}

TEST_F(LongSubTask, SharedTp){
    initSubs(2);

    for (int i = 0; i < 10; ++i) {
        pub->scheduleItc(make_shared<string>("fdd"));
    }
    this_thread::sleep_for(1ms);

    node->removeSubscriber(subs[0]);
    EXPECT_EQ(count.load(), 1);

    node->removeSubscriber(subs[1]);
    EXPECT_LE(count.load(), 2);
}

int main(int argc, char **argv){
    // zmq_force_delay = 50;
    // todo ivan. wait until master is up and pingable
    this_thread::sleep_for(10ms);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
