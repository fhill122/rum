
#include <rum/cpprum/rum.h>

#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "../test_msg/test_number_generated.h"


struct ServerFb{

};

bool ServerFbCallback(const std::shared_ptr<const rum::Message>& request,
                      std::shared_ptr<flatbuffers::FlatBufferBuilder>& response){
    const rum::test::msg::Number* req = rum::test::msg::GetNumber(request->data());
    // rum::Log::I(__func__, "received a num of n1 %d", req->n1());
    response->Finish(rum::test::msg::CreateNumber(*response, req->n1(), req->n1()+1,
                                                  reinterpret_cast<std::uintptr_t>(request->data())));
    return true;
}