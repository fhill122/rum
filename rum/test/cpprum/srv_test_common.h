
#include <rum/cpprum/rum.h>

#include <rum/cpprum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/flatbuffers/test_number_generated.h"

namespace rum {

constexpr char kSrv[] = "GetImage";

enum class CompanionCmd : int{
    BasicInterP = 0,
    BasicInterP2,
    BasicIpcInterP,
    BasicTcpInterP,
    Timeout,
    SafeEnding,
    Cancelling,
    Overflow,
};

std::unique_ptr<FbsBuilder> CreateReqeust(int n1, double d1 = 0, double d2 = 1){
    auto builder = std::make_unique<FbsBuilder>();
    auto point = test::msg::CreateNumber(*builder, n1, 0, 0, 0, d1, d2);
    builder->Finish(point);
    return builder;
}

inline bool ServerFbCallback(const std::shared_ptr<const rum::Message> &request,
                             std::shared_ptr<flatbuffers::FlatBufferBuilder> &response,
                             int sleep_ms, std::atomic_int *count = nullptr) {
    using namespace std;
    if(count) count->fetch_add(1);
    const rum::test::msg::Number *req = rum::test::msg::GetNumber(request->data());
    rum::Log::I(__func__, "server at %s received a num of n1 %d, d1 %f, d2 %f",
                NodeBase::GlobalNode()->getStrId().c_str(), req->n1(), req->d1(), req->d2());

    if(sleep_ms>0) {
        // this could be very inaccurate
        this_thread::sleep_for(sleep_ms * 1ms);
        rum::Log::D(__func__, "sleep finished");
    }

    // n1 = n1, n2 = n1 + 1, l1 = memory address, l2 = 0, d1 = d1/d2, d2 = 0
    if (req->d2()==0.0) {
        return false;
    }
    response->Finish(rum::test::msg::CreateNumber(*response, req->n1(), req->n1() + 1,
                                                  reinterpret_cast<std::uintptr_t>(request->data()), 0,
                                                  req->d1()/req->d2(), 0.0));
    return true;
}

}