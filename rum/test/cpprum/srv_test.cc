/*
 * Created by Ivan B on 2022/3/10.
 */

#include <gtest/gtest.h>

#include <rum/cpprum/rum.h>
#include <rum/common/common.h>
#include <rum/extern/ivtb/stopwatch.h>
#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_number_generated.h"
#include "../test_utils/process_utils.h"
#include "srv_test_common.h"

using namespace std;
using namespace rum;

string argv0;

struct SimpleFbNode : public ::testing::Test{
    Client<FbsBuilder, Message>::UniquePtr  client = nullptr;
    Server::UniquePtr server = nullptr;

    void basicIntraP(unique_ptr<FbsBuilder> req_builder,
                     bool async = true,
                     SrvStatus expect = SrvStatus::OK){
        client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);
        server.reset();
        server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
                             bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0, nullptr) );

        bool ping_ok = client->ping(0, 0);
        ASSERT_TRUE(ping_ok);

        uintptr_t req_address = reinterpret_cast<std::uintptr_t>(req_builder->GetBufferPointer());
        const auto *req = rum::test::msg::GetNumber(req_builder->GetBufferPointer());
        const test::msg::Number *rep;

        // async call
        if (async){
            auto future_res = client->call(move(req_builder), 50);
            auto result = future_res.future.get();
            ASSERT_EQ(result.status, expect);
            if (result.status!=SrvStatus::OK) return;
            rep = test::msg::GetNumber(result.response->data());
        }
        // foreground call
        else {
            auto direct_res = client->callForeground(move(req_builder), 50);
            ASSERT_EQ(direct_res.status, expect);
            if (direct_res.status!=SrvStatus::OK) return;
            rep = test::msg::GetNumber(direct_res.response->data());
        }

        // check response
        ASSERT_EQ(rep->n2(), req->n1()+1);
        // same memory address
        ASSERT_EQ(rep->l1(), req_address);
    }

    void basicInterP(unique_ptr<FbsBuilder> req_builder,
                     CompanionCmd companion_cmd,
                     bool async = true,
                     SrvStatus expect = SrvStatus::OK){
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(companion_cmd));
        DaemonProcess daemon_process(cmd);

        server.reset();
        client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);

        uintptr_t req_address = reinterpret_cast<std::uintptr_t>(req_builder->GetBufferPointer());
        const auto *req = rum::test::msg::GetNumber(req_builder->GetBufferPointer());
        const test::msg::Number *rep;

        // async call
        if (async){
            auto future_res = client->call(move(req_builder), 50);
            auto result = future_res.future.get();
            ASSERT_EQ(result.status, expect);
            if (result.status!=SrvStatus::OK) return;
            rep = test::msg::GetNumber(result.response->data());
        }
        // foreground call
        else {
            auto direct_res = client->callForeground(move(req_builder), 50);
            ASSERT_EQ(direct_res.status, expect);
            if (direct_res.status!=SrvStatus::OK) return;
            rep = test::msg::GetNumber(direct_res.response->data());
        }

        // check response
        ASSERT_EQ(rep->n2(), req->n1()+1);
        ASSERT_NE(rep->l1(), req_address);
    }
};

TEST_F(SimpleFbNode, BasicIntraP){
    basicIntraP(CreateReqeust(1,0,1), true, SrvStatus::OK);
    basicIntraP(CreateReqeust(1,0,1), false, SrvStatus::OK);
    basicIntraP(CreateReqeust(1,1,0), true, SrvStatus::ServerErr);
    basicIntraP(CreateReqeust(1,1,0), false, SrvStatus::ServerErr);
}

TEST_F(SimpleFbNode, BasicInterP){
    // basicInterP(CreateReqeust(1,0,1), CompanionCmd::BasicInterP, true, SrvStatus::OK);
    // basicInterP(CreateReqeust(1,0,1), CompanionCmd::BasicInterP, false, SrvStatus::OK);
    // basicInterP(CreateReqeust(1,1,0), CompanionCmd::BasicInterP, true, SrvStatus::ServerErr);
    basicInterP(CreateReqeust(1,1,0), CompanionCmd::BasicInterP, false, SrvStatus::ServerErr);
}

// local server, remote client
TEST_F(SimpleFbNode, BasicInterP2){
    string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::BasicInterP2));
    DaemonProcess daemon_process(cmd);

    server.reset();
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0, &count) );
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==1) break;
        this_thread::sleep_for(10ms);
    }
    EXPECT_EQ(count.load(), 1);
}

TEST_F(SimpleFbNode, BasicTcpInterP){
    basicInterP(CreateReqeust(1,0,1), CompanionCmd::BasicTcpInterP, true, SrvStatus::OK);
    basicInterP(CreateReqeust(1,0,1), CompanionCmd::BasicTcpInterP, false, SrvStatus::OK);
    basicInterP(CreateReqeust(1,1,0), CompanionCmd::BasicTcpInterP, true, SrvStatus::ServerErr);
    basicInterP(CreateReqeust(1,1,0), CompanionCmd::BasicTcpInterP, false, SrvStatus::ServerErr);
}

TEST_F(SimpleFbNode, BasicMixed){
    basicInterP(CreateReqeust(1,0,1), CompanionCmd::BasicInterP, true, SrvStatus::OK);

    // test intra process while exist remote server
    string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::BasicInterP));
    DaemonProcess daemon_process(cmd);
    basicIntraP(CreateReqeust(1,0,1), true, SrvStatus::OK);
}

TEST_F(SimpleFbNode, DuplicateServers){
    constexpr int kNServer = 3;
    vector<DaemonProcess> daemons;
    daemons.reserve(kNServer-1);
    string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::BasicTcpInterP));
    for (int i = 0; i < kNServer-1; ++i) {
        daemons.emplace_back(cmd);
    }

    // should work just like normal, taking the response only from the fastest server
    basicInterP(CreateReqeust(1,0,1), CompanionCmd::BasicInterP, true, SrvStatus::OK);
}

TEST_F(SimpleFbNode, Timeout){
    client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);

    // intra process
    {
        server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
                    bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, nullptr) );

        bool ping_ok = client->ping(0, 0);
        ASSERT_TRUE(ping_ok);

        auto direct_res = client->callForeground(CreateReqeust(2), 15);
        EXPECT_EQ(direct_res.status, SrvStatus::OK);

        auto direct_res2 = client->callForeground(CreateReqeust(2), 1);
        EXPECT_EQ(direct_res2.status, SrvStatus::Timeout);
    }

    // inter process
    {
        server.reset();
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::Timeout));
        DaemonProcess daemon_process(cmd);

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);

        auto direct_res = client->callForeground(CreateReqeust(2), 15);
        EXPECT_EQ(direct_res.status, SrvStatus::OK);

        auto direct_res2 = client->callForeground(CreateReqeust(2), 1);
        EXPECT_EQ(direct_res2.status, SrvStatus::Timeout);
    }
}

TEST_F(SimpleFbNode, SafeEnding){
    client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);

    // intra process
    {
        server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
            bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, nullptr) );

        bool ping_ok = client->ping(0, 0);
        ASSERT_TRUE(ping_ok);
        FutureResult<Message> future_res1 = client->call(CreateReqeust(2), 15);
        ivtb::Stopwatch stopwatch;
        std::this_thread::sleep_for(1ms);
        FutureResult<Message> future_res2 = client->call(CreateReqeust(2), 15);
        // make sure server starts to precess
        this_thread::sleep_for(1ms);

        // server destroy should wait for the current task to be finished
        server.reset();
        EXPECT_NEAR(stopwatch.passedMs(), 10, 4); // sleep is quite inaccurate

        ASSERT_EQ(future_res1.future.get().status, SrvStatus::OK);
        ASSERT_EQ(future_res2.future.get().status, SrvStatus::Timeout);
    }

    // inter process
    {
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::SafeEnding));
        DaemonProcess daemon_process(cmd);

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);
        FutureResult<Message> future_res1 = client->call(CreateReqeust(2), 15);
        std::this_thread::sleep_for(1ms);
        FutureResult<Message> future_res2 = client->call(CreateReqeust(2), 15);

        ASSERT_EQ(future_res1.future.get().status, SrvStatus::OK);
        ASSERT_EQ(future_res2.future.get().status, SrvStatus::Timeout);
    }
}

TEST_F(SimpleFbNode, NoConnection){
    client = CreateClient<FbsBuilder, Message, SerializerFbs>(kSrv);
    FutureResult<Message> future_res = client->call(CreateReqeust(2), 15);
    ivtb::Stopwatch stopwatch;
    auto res = future_res.future.get();
    // should have quick ending
    ASSERT_LE(stopwatch.passedMs(), 2);
    ASSERT_EQ(res.status, SrvStatus::NoConnections);

    Result<Message> direct_res = client->callForeground(CreateReqeust(2), 15);
    ASSERT_LE(stopwatch.passedMs(), 2);
    ASSERT_EQ(direct_res.status, SrvStatus::NoConnections);
}

TEST_F(SimpleFbNode, Cancelling) {
    client = CreateClient<FbsBuilder, Message, SerializerFbs>(kSrv);

    // intra process
    {
        server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
                 bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, nullptr) );
        ivtb::Stopwatch stopwatch;
        FutureResult<Message> future_res = client->call(CreateReqeust(2), 15);
        bool cancelled = future_res.cancel();
        // cancel should be instant
        ASSERT_LE(stopwatch.passedMs(), 2);
        ASSERT_TRUE(cancelled);
        // repeated cancel should return false
        cancelled = future_res.cancel();
        ASSERT_FALSE(cancelled);
        server.reset();
    }

    // inter process
    {
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::Cancelling));
        DaemonProcess daemon_process(cmd);

        bool ping_ok = client->ping(kNodeHbPeriod + 1000, 100);
        ASSERT_TRUE(ping_ok);

        ivtb::Stopwatch stopwatch;
        FutureResult<Message> future_res = client->call(CreateReqeust(2), 15);
        bool cancelled = future_res.cancel();
        // cancel should be instant
        ASSERT_LE(stopwatch.passedMs(), 2);
        ASSERT_TRUE(cancelled);
        // repeated cancel should return false
        cancelled = future_res.cancel();
        ASSERT_FALSE(cancelled);
    }
}

TEST_F(SimpleFbNode, Overflow) {
    client = CreateClient<FbsBuilder, Message, SerializerFbs>(kSrv);

    // intra process
    {
        server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
                 bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, nullptr), 1);
        ivtb::Stopwatch stopwatch;
        FutureResult<Message> future_res1 = client->call(CreateReqeust(2), 15);
        std::this_thread::sleep_for(1ms);
        FutureResult<Message> future_res2 = client->call(CreateReqeust(2), 15);
        FutureResult<Message> future_res3 = client->call(CreateReqeust(2), 15);

        EXPECT_EQ(future_res1.future.get().status, SrvStatus::OK);
        EXPECT_EQ(future_res2.future.get().status, SrvStatus::Timeout);
        EXPECT_EQ(future_res3.future.get().status, SrvStatus::Timeout);
        server.reset();
    }

    // inter process
    {
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::Overflow));
        DaemonProcess daemon_process(cmd);

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);

        ivtb::Stopwatch stopwatch;
        FutureResult<Message> future_res1 = client->call(CreateReqeust(2), 15);
        std::this_thread::sleep_for(1ms);
        FutureResult<Message> future_res2 = client->call(CreateReqeust(2), 15);
        FutureResult<Message> future_res3 = client->call(CreateReqeust(2), 15);

        EXPECT_EQ(future_res1.future.get().status, SrvStatus::OK);
        EXPECT_EQ(future_res2.future.get().status, SrvStatus::Timeout);
        EXPECT_EQ(future_res3.future.get().status, SrvStatus::Timeout);
    }
}

//todo ivan.
// different serializer for req and rep
// threadpool


int main(int argc, char **argv){
    rum::Init();

    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::GTEST_FLAG(filter) = "*Cancelling";
    ::testing::GTEST_FLAG(filter) = "*BasicInterP2";
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
