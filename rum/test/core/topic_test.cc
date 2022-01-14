//
// Created by Ivan B on 2021/8/7.
//

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <rum/common/log.h>
#include <rum/core/internal/node_base_impl.h>
#include <rum/common/common.h>

#include "rum/core/internal/itc_manager.h"

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

    string cmd = argv0 + "_companion " + "IpcBasic";
    system(cmd.c_str());
    EXPECT_EQ(itc_count.load(), 0);
    EXPECT_EQ(ipc_count.load(), 10);
}

TEST_F(ImplTest, TcpBasic) {
    init();

    string cmd = argv0 + "_companion " + "TcpBasic";
    system(cmd.c_str());
    EXPECT_EQ(itc_count.load(), 0);
    EXPECT_EQ(ipc_count.load(), 10);
}

TEST_F(ImplTest, TcpIcpMismatched) {
    NodeParam param;
    param.enable_tcp_socket = false;
    init(move(param));

    string cmd = argv0 + "_companion " + "TcpBasic";
    system(cmd.c_str());
    EXPECT_EQ(itc_count.load(), 0);
    EXPECT_EQ(ipc_count.load(), 0);
}

TEST_F(ImplTest, ItcIpcTcpBasic) {
    init();

    string cmd1 = argv0 + "_companion " + "IpcBasic";
    string cmd2 = argv0 + "_companion " + "TcpBasic";
    auto t1 = thread([cmd1]{system(cmd1.c_str());});
    auto t2 = thread([cmd2]{system(cmd2.c_str());});

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

TEST_F(ImplMultiTest, MultiItcIpcTcp){
    constexpr int kNTopics = 10;
    constexpr int kNSubs = 5;
    constexpr int kNPubs = 10;

    init(kNTopics, kNSubs, kNPubs);

    string cmd1 = argv0 + "_companion " + "MultiIpc";
    string cmd2 = argv0 + "_companion " + "MultiTcp";
    auto t1 = thread([cmd1]{system(cmd1.c_str());});
    auto t2 = thread([cmd2]{system(cmd2.c_str());});

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
    this_thread::sleep_for(50ms);

    t1.join();
    t2.join();
    EXPECT_EQ(shared_itc_count.load(), itc_n*kNSubs*kNTopics*kNPubs);
    EXPECT_EQ(shared_ipc_count.load(), 100*2*kNSubs*kNTopics*kNPubs);
}

// todo ivan. test publish connection judge

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
