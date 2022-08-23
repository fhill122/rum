/*
 * Created by Ivan B on 2022/1/10.
 */

#include "rum/common/common.h"
#include "rum/common/misc.h"
#include "rum/extern/ivtb/stopwatch.h"
#include "rum/cpprum/client.h"
#include "rum/cpprum/serialization/flatbuffers/serializer_fbs.h"
#include "rum/cpprum/rum.h"
#include "rum/common/log.h"
#include "../test_msg/test_point_generated.h"

using namespace rum;
using namespace std;

bool SrvFuncFbs(const shared_ptr<const Message>& request, shared_ptr<FbsBuilder>& response){
    const test::msg::PointTable* point = test::msg::GetPointTable(request->data());
    Log::I(__func__, "received a point of %f %f %f", point->x(), point->y(), point->z());

    this_thread::sleep_for(500ms);
    // response = make_shared<FbsBuilder>();
    auto point_rep = test::msg::CreatePointTable(*response, 3,4,5);
    response->Finish(point_rep);
    return true;
}

int main(){
    auto server = rum::CreateServer<Message, FbsBuilder, SerializerFbs>("srvName", &SrvFuncFbs);

    auto client = rum::CreateClient<FbsBuilder, Message, SerializerFbs>("srvName");

    auto builder = make_unique<FbsBuilder>();
    auto point = test::msg::CreatePointTable(*builder, 1, 2, 3);
    builder->Finish(point);

    Log::I(__func__, "call started");
    auto future_result = client->call(move(builder), 600);
    Log::I(__func__, "call finished");

    auto result = future_result.future.get();
    const auto *point_rep = test::msg::GetPointTable(result.response->data());
    Log::I(__func__, "result retrieved: status %d, point %f %f %f",
           result.status, point_rep->x(), point_rep->y(), point_rep->z());
}