//
// Created by Ivan B on 2021/6/26.
//

/*

 slow join

 a, b connect to address_1
 c bound to address_1

 c dead

 d bound to address_1
 d publish on address_1

 will a,b receive it?
 how can d get notified when a,b connected to it?

 */
#include <gtest/gtest.h>

#include <rum/common/log.h>
#include <rum/common/zmq_helper.h>

using namespace std;
using namespace rum;

class MonitorTest : public testing::Test{
  public:
    // these have to be above monitor_
    vector<pair<int, string>> events;
    mutable mutex events_mu;

    unique_ptr<zmq::socket_t> socket_;  // monitored socket
    unique_ptr<rum::ZmqMonitor> monitor_;
    unique_ptr<zmq::socket_t> socket_test_;

    bool getEvent(int event, string &address){
        lock_guard<mutex> lock(events_mu);
        for (const auto &e : events){
            if (e.first == event){
                address = e.second;
                return true;
            }
        }
        return false;
    }

    void clearEvents(){
        lock_guard<mutex> lock(events_mu);
        events.clear();
    }

    void printEvents(){
        lock_guard<mutex> lock(events_mu);
        string str = "Have received " + to_string(events.size()) + " events: ";
        for (const auto &e : events){
            str += to_string(e.first) + "@" + e.second + "\t";
        }
        Log::V(__func__, str);
    }

    MonitorTest(){
        socket_ = make_unique<zmq::socket_t>(*rum::shared_context(), ZMQ_SUB);
        socket_->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
        // int reconnect_period = 500;
        // socket_->setsockopt(ZMQ_RECONNECT_IVL, &reconnect_period, sizeof(reconnect_period));

        socket_test_ = make_unique<zmq::socket_t>(*rum::shared_context(), ZMQ_PUB);
        monitor_ = make_unique<rum::ZmqMonitor>(*socket_);
        monitor_->start([this](const zmq_event_t &event, const char *address){
            {
                lock_guard<mutex> lock(events_mu);
                events.emplace_back(event.event, address);
            }
            constexpr char kTag[] = "MonitorCb";
            switch (event.event) {
                case ZMQ_EVENT_CONNECTED:
                    Log::V(kTag, "ZMQ_EVENT_CONNECTED %s", address);
                    break;
                case ZMQ_EVENT_CONNECT_DELAYED:
                    Log::V(kTag, "ZMQ_EVENT_CONNECT_DELAYED %s", address);
                    break;
                case ZMQ_EVENT_CONNECT_RETRIED:
                    Log::V(kTag, "ZMQ_EVENT_CONNECT_RETRIED %s", address);
                    break;
                case ZMQ_EVENT_LISTENING:
                    Log::V(kTag, "ZMQ_EVENT_LISTENING %s", address);
                    break;
                case ZMQ_EVENT_BIND_FAILED:
                    Log::V(kTag, "ZMQ_EVENT_BIND_FAILED %s", address);
                    break;
                case ZMQ_EVENT_ACCEPTED:
                    Log::V(kTag, "ZMQ_EVENT_ACCEPTED %s", address);
                    break;
                case ZMQ_EVENT_ACCEPT_FAILED:
                    Log::V(kTag, "ZMQ_EVENT_ACCEPT_FAILED %s", address);
                    break;
                case ZMQ_EVENT_CLOSED:
                    Log::V(kTag, "ZMQ_EVENT_CLOSED %s", address);
                    break;
                case ZMQ_EVENT_CLOSE_FAILED:
                    Log::V(kTag, "ZMQ_EVENT_CLOSE_FAILED %s", address);
                    break;
                case ZMQ_EVENT_DISCONNECTED:
                    Log::V(kTag, "ZMQ_EVENT_DISCONNECTED %s", address);
                    break;
                case ZMQ_EVENT_MONITOR_STOPPED:
                    Log::V(kTag, "ZMQ_EVENT_MONITOR_STOPPED %s", address);
                    break;
                default:
                    Log::V(kTag, "default on event 0X%X, addr: %s", event.event, address);
                    break;
            }
        });
    }
};

TEST_F(MonitorTest, Stop){
    Log::I(__func__, "stop");
    monitor_->stop();

    usleep(1e4);
    string event_addr;
    bool res = getEvent(ZMQ_EVENT_MONITOR_STOPPED, event_addr);
    EXPECT_TRUE(res);
}

TEST_F(MonitorTest, Accept){
    string addr = rum::BindTcp(*socket_);
    ASSERT_FALSE(addr.empty());
    usleep(1e5);

    Log::I(__func__, "connect");
    socket_test_->connect(addr);
    usleep(1e5);
    string event_addr;
    bool res = getEvent(ZMQ_EVENT_ACCEPTED, event_addr);
    EXPECT_TRUE(res);
    EXPECT_EQ(event_addr, addr);
}

TEST_F(MonitorTest, Connect){
    string addr = rum::BindTcp(*socket_test_);
    ASSERT_FALSE(addr.empty());

    Log::I(__func__, "connect");
    socket_->connect(addr);
    usleep(1e5);

    string event_addr;
    bool res = getEvent(ZMQ_EVENT_CONNECT_DELAYED, event_addr);
    EXPECT_TRUE(res);
    EXPECT_EQ(event_addr, addr);
    getEvent(ZMQ_EVENT_CONNECTED, event_addr);
    EXPECT_TRUE(res);
    EXPECT_EQ(event_addr, addr);
}

TEST_F(MonitorTest, Disconnect){
    string addr = rum::BindTcp(*socket_test_);
    ASSERT_FALSE(addr.empty());

    Log::I(__func__, "connect");
    socket_->connect(addr);
    usleep(1e5);
    clearEvents();

    socket_test_->close();
    usleep(1e5);
    printEvents();

    string event_addr;
    bool res = getEvent(ZMQ_EVENT_DISCONNECTED, event_addr);
    EXPECT_TRUE(res);
    EXPECT_EQ(event_addr, addr);
    getEvent(ZMQ_EVENT_CONNECT_RETRIED, event_addr);
    EXPECT_TRUE(res);
    EXPECT_EQ(event_addr, addr);
}

TEST_F(MonitorTest, DisconnectOnRemoteCrash){
    // todo ivan. this makes no guarantee that can be bound, retest if failure for this reason
    std::string addr = rum::GenTcpAddr();

    Log::V(__func__, "fork!");
    fflush(stdout);
    int pid = fork();
    if (pid == 0){
        Log::I("child", "start");
        // recreate the test socket
        socket_test_ = make_unique<zmq::socket_t>(*rum::shared_context(), ZMQ_PUB);
        string final_addr = rum::BindTcp(*socket_test_, addr);
        ASSERT_FALSE(final_addr.empty());
        Log::D("child", "bind");
        this_thread::sleep_for(500ms);
        Log::D("child", "abort");
        fflush(stdout);
        // prevent optimization
        pid = pid*2; if (pid == 0) abort();
    }
    else{
        Log::I("parent", "start");
        this_thread::sleep_for(100ms);
        socket_->connect(addr);
        this_thread::sleep_for(300ms);
        clearEvents();
        Log::D("parent", "cleared old events");

        this_thread::sleep_for(500ms);
        string event_addr;
        bool res = getEvent(ZMQ_EVENT_DISCONNECTED, event_addr);
        EXPECT_TRUE(res);
        EXPECT_EQ(event_addr, addr);
        getEvent(ZMQ_EVENT_CONNECT_RETRIED, event_addr);
        EXPECT_TRUE(res);
        EXPECT_EQ(event_addr, addr);
    }

}

TEST(MultiPartMsgTest, latency){

}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
