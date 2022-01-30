/*
 * Created by Ivan B on 2022/1/20.
 */

#include "client_base_impl.h"

#include "rum/common/log.h"
#include "rum/common/common.h"

#define TAG (service_name_+"_cli")

using namespace std;

namespace rum{

ClientBaseImpl::ClientBaseImpl(PublisherBaseImpl* pub, SubscriberBaseImpl* sub):
        pub_(pub), sub_(sub){}

unique_ptr<AwaitingResult>
ClientBaseImpl::callIpc(unique_ptr<Message> req_msg, unsigned int timeout_ms) {
    auto awaiting_result = make_unique<AwaitingResult>();
    {
        lock_guard lock(wait_list_mu_);
        wait_list_.emplace(awaiting_result->id, awaiting_result.get());
    }

    pub_->publishReqIpc(awaiting_result->id, sub_->topic_, *req_msg);

    // wait until got result or timeout
    bool got_result = true;
    unique_lock result_lock(awaiting_result->mu);
    if (timeout_ms){
        awaiting_result->cv.wait(result_lock, [&awaiting_result](){return awaiting_result->response != nullptr;});
    } else{
        // gcc bug before gcc10? https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41861
        got_result = awaiting_result->cv.wait_for(result_lock, chrono::milliseconds(timeout_ms),
                                    [&awaiting_result](){return awaiting_result->response != nullptr;});
    }

    // erase only if timeout to reduce lock
    if (!got_result){
        {
            lock_guard lock(wait_list_mu_);
            size_t n_removed = wait_list_.erase(awaiting_result->id);
        }
        // n_removed could be 0 if server just responded after wait_for finished
        awaiting_result->status = SrvStatus::Timeout;
    } else{
        // awaiting_result is set by server
        if (awaiting_result->status == SrvStatus::OK)
            AssertLog(awaiting_result->response, "ok but null");
    }

    return awaiting_result;
}

shared_ptr<AwaitingResult>
ClientBaseImpl::callItc(const shared_ptr<const void> &req_obj, unsigned int timeout_ms) {
    auto awaiting_result = make_shared<AwaitingResult>(0);
    awaiting_result->request = req_obj;
    // no need to add to wait_list
    pub_->scheduleItc(awaiting_result);

    // wait until got result or timeout
    bool got_result = true;
    unique_lock result_lock(awaiting_result->mu);
    if (timeout_ms){
        awaiting_result->cv.wait(result_lock, [&awaiting_result](){return awaiting_result->response != nullptr;});
    } else{
        // gcc bug before gcc10? https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41861
        got_result = awaiting_result->cv.wait_for(result_lock, chrono::milliseconds(timeout_ms),
                                                  [&awaiting_result](){return awaiting_result->response != nullptr;});
    }

    if (!got_result){
        awaiting_result->status = SrvStatus::Timeout;
    } else{
        // awaiting_result is set by server
        if (awaiting_result->status == SrvStatus::OK)
            AssertLog(awaiting_result->response, "ok but null");
    }

    return awaiting_result;
}

}

#undef TAG