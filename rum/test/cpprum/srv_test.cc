/*
 * Created by Ivan B on 2022/3/10.
 */

#include <gtest/gtest.h>

#include <rum/cpprum/rum.h>
#include <rum/common/common.h>

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
};

TEST_F(SimpleFbNode, BasicIntraP){
    client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);
    server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv,
                bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0) );

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

    // todo ivan. close server

}


// todo ivan. sometimes remote is not ready
TEST_F(SimpleFbNode, BasicInterP){
    client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);
    string cmd = argv0 + "_companion " + to_string(static_cast<int>(CompanionCmd::BasicInterP));
    thread companion_t([&]{system(cmd.c_str());});
    this_thread::sleep_for((kNodeHbPeriod+150)*1ms);

    auto req = CreateReqeust(1);
    uintptr_t req_address = reinterpret_cast<std::uintptr_t>(req->GetBufferPointer());
    Log::W(__func__, "call");
    auto future_res = client->call(move(req), 50);
    auto result = future_res.future.get();
    ASSERT_EQ(result.status, SrvStatus::OK);
    auto rep = test::msg::GetNumber(result.response->data());
    ASSERT_EQ(rep->n2(), 1+1);
    ASSERT_NE(rep->l1(), req_address);

    Log::W(__func__, "call foreground");
    auto direct_res = client->callForeground(CreateReqeust(2), 50);
    ASSERT_EQ(direct_res.status, SrvStatus::OK);
    auto direct_rep = test::msg::GetNumber(direct_res.response->data());
    ASSERT_EQ(direct_rep->n2(), 2+1);

    companion_t.join();
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
