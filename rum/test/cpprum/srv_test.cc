/*
 * Created by Ivan B on 2022/3/10.
 */

#include <gtest/gtest.h>

#include <rum/cpprum/rum.h>
#include <rum/common/common.h>
#include <rum/extern/ivtb/stopwatch.h>
#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_number_generated.h"
#include "srv_test_common.h"

using namespace std;
using namespace rum;

string argv0;

struct SimpleFbNode : public ::testing::Test{
    Client<FbsBuilder, Message>::UniquePtr  client = nullptr;
    Server::UniquePtr server = nullptr;

    static unique_ptr<FbsBuilder> CreateReqeust(int x){
        auto builder = make_unique<FbsBuilder>();
        auto point = test::msg::CreateNumber(*builder, x);
        builder->Finish(point);
        return builder;
    }

    void basicIntraP(){
        client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);
        server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
                             bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0, nullptr) );

        bool ping_ok = client->ping(0, 0);
        ASSERT_TRUE(ping_ok);

        auto req = CreateReqeust(1);
        uintptr_t req_address = reinterpret_cast<std::uintptr_t>(req->GetBufferPointer());
        auto future_res = client->call(move(req), 50);
        auto result = future_res.future.get();
        ASSERT_EQ(result.status, SrvStatus::OK);
        auto rep = test::msg::GetNumber(result.response->data());
        ASSERT_EQ(rep->n2(), 1+1);
        // itc takes place
        ASSERT_EQ(rep->l1(), req_address);

        auto direct_res = client->callForeground(CreateReqeust(2), 50);
        ASSERT_EQ(direct_res.status, SrvStatus::OK);
        auto direct_rep = test::msg::GetNumber(direct_res.response->data());
        ASSERT_EQ(direct_rep->n2(), 2+1);
    }

    void basicInterP(CompanionCmd companion_cmd){
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(companion_cmd));
        thread companion_t([&]{system(cmd.c_str());});

        client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);

        auto req = CreateReqeust(1);
        uintptr_t req_address = reinterpret_cast<std::uintptr_t>(req->GetBufferPointer());
        auto future_res = client->call(move(req), 50);
        auto result = future_res.future.get();
        ASSERT_EQ(result.status, SrvStatus::OK);
        auto rep = test::msg::GetNumber(result.response->data());
        ASSERT_EQ(rep->n2(), 1+1);
        ASSERT_NE(rep->l1(), req_address);

        auto direct_res = client->callForeground(CreateReqeust(2), 50);
        ASSERT_EQ(direct_res.status, SrvStatus::OK);
        auto direct_rep = test::msg::GetNumber(direct_res.response->data());
        ASSERT_EQ(direct_rep->n2(), 2+1);

        companion_t.join();
    }
};

TEST_F(SimpleFbNode, BasicIntraP){
    basicIntraP();
}

TEST_F(SimpleFbNode, BasicInterP){
    basicInterP(CompanionCmd::BasicInterP);
}

TEST_F(SimpleFbNode, BasicTcpInterP){
    basicInterP(CompanionCmd::BasicTcpInterP);
}

TEST_F(SimpleFbNode, BasicMixed){
    basicInterP(CompanionCmd::BasicInterP);
    basicIntraP();
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
        thread companion_t([&]{system(cmd.c_str());});

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);

        auto direct_res = client->callForeground(CreateReqeust(2), 15);
        EXPECT_EQ(direct_res.status, SrvStatus::OK);

        auto direct_res2 = client->callForeground(CreateReqeust(2), 1);
        EXPECT_EQ(direct_res2.status, SrvStatus::Timeout);

        companion_t.join();
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
        FutureResult<Message> future_res2 = client->call(CreateReqeust(2), 15);
        // make sure server starts to precess
        this_thread::sleep_for(1ms);

        // server destroy should wait for the current task to be finished
        ivtb::Stopwatch stopwatch;
        server.reset();
        EXPECT_NEAR(stopwatch.passedMs(), 10, 2);

        ASSERT_EQ(future_res1.future.get().status, SrvStatus::OK);
        ASSERT_EQ(future_res2.future.get().status, SrvStatus::Timeout);
    }

    // inter process
    {
        string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::SafeEnding));
        thread companion_t([&]{system(cmd.c_str());});

        bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
        ASSERT_TRUE(ping_ok);
        FutureResult<Message> future_res1 = client->call(CreateReqeust(2), 15);
        FutureResult<Message> future_res2 = client->call(CreateReqeust(2), 15);

        ASSERT_EQ(future_res1.future.get().status, SrvStatus::OK);
        ASSERT_EQ(future_res2.future.get().status, SrvStatus::Timeout);

        companion_t.join();
    }
}

//todo ivan.
// multiple servers: itc, ipc, mixed (ping function required)
// no connection, should return quickly (ping function required)
// different serializer for req and rep
// cancel
// server cb err, serialization err, crash, timeout
// threadpool


int main(int argc, char **argv){
    rum::Init();

    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
