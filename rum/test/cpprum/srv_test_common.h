
#include <rum/cpprum/rum.h>

#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_number_generated.h"

namespace rum {

constexpr char kSrv[] = "GetImage";

enum class CompanionCmd : int{
    BasicInterP = 0,
    BasicIpcInterP,
    BasicTcpInterP,
};

inline bool ServerFbCallback(const std::shared_ptr<const rum::Message> &request,
                             std::shared_ptr<flatbuffers::FlatBufferBuilder> &response,
                             int sleep_ms, std::atomic_int *count = nullptr) {
    using namespace std;
    if(count) count->fetch_add(1);
    const rum::test::msg::Number *req = rum::test::msg::GetNumber(request->data());
    rum::Log::I(__func__, "server at %s received a num of n1 %d",
                NodeBase::GlobalNode()->getStrId().c_str(), req->n1());
    response->Finish(rum::test::msg::CreateNumber(*response, req->n1(), req->n1() + 1,
                                                  reinterpret_cast<std::uintptr_t>(request->data())));
    if(sleep_ms>0) this_thread::sleep_for(sleep_ms * 1ms);
    return true;
}

}