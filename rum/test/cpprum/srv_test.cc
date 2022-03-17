/*
 * Created by Ivan B on 2022/3/10.
 */

#include <gtest/gtest.h>

#include <rum/cpprum/rum.h>

#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_number_generated.h"
#include "srv_test_common.h"

using namespace std;
using namespace rum;

string argv0;

struct SimpleFbNode : public ::testing::Test{
    static constexpr char kSrv[] = "GetImage";
    Client<FbsBuilder, Message>::UniquePtr  client = nullptr;
    Server::UniquePtr server = nullptr;

    static unique_ptr<FbsBuilder> CreateReqeust(int x){
        auto builder = make_unique<FbsBuilder>();
        auto point = test::msg::CreateNumber(*builder, x);
        builder->Finish(point);
        return builder;
    }
};

// seg occasionally. itcTypeConvert on message with null ptr
TEST_F(SimpleFbNode, BasicItc){
    client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);
    server = CreateServer<Message, FbsBuilder, SerializerFbs>(kSrv, &ServerFbCallback);

    auto req = CreateReqeust(1);
    uintptr_t req_address = reinterpret_cast<std::uintptr_t>(req->GetBufferPointer());
    auto future_res = client->call(move(req));
    auto result = future_res.future.get();
    ASSERT_EQ(result.status, SrvStatus::OK);
    auto rep = test::msg::GetNumber(result.response->data());
    ASSERT_EQ(rep->n2(), 1+1);
    // itc takes place
    ASSERT_EQ(rep->l1(), req_address);

    auto direct_res = client->callForeground(CreateReqeust(2));
    ASSERT_EQ(direct_res.status, SrvStatus::OK);
    auto direct_rep = test::msg::GetNumber(direct_res.response->data());
    ASSERT_EQ(direct_rep->n2(), 2+1);
}

//todo ivan.
// multiple servers: itc, ipc, mixed (ping function required)
// no connection, should return quickly (ping function required)
// different serializer for req and rep
// cancel


int main(int argc, char **argv){
    rum::Init();

    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    argv0 = argv[0];

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
