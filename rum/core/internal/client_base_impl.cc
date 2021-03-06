/*
 * Created by Ivan B on 2022/1/20.
 */

#include "client_base_impl.h"

#include "rum/common/log.h"
#include "rum/common/common.h"
#include "rum/extern/ivtb/stopwatch.h"

#define TAG (service_name_+"_cli")

using namespace std;

namespace rum{

ClientBaseImpl::ClientBaseImpl(PublisherBaseImpl* pub, SubscriberBaseImpl* sub):
        pub_(pub), sub_(sub){}

unique_ptr<AwaitingResult>
ClientBaseImpl::callInterProc(unique_ptr<Message> req_msg, unsigned int timeout_ms) {
    auto awaiting_result = sendInterProc(move(req_msg));
    waitIterProc(awaiting_result.get(), timeout_ms);
    return awaiting_result;
}

std::unique_ptr<AwaitingResult> ClientBaseImpl::sendInterProc(std::unique_ptr<Message> req_msg, bool ping) {
    unique_ptr<AwaitingResult> awaiting_result = AwaitingResult::CreateInterP();

    // unlike intra process, we do not check pub's connection here, as it would be checked before this call to prevent
    // unnecessary serialization
    if (ping){
        pub_->publishPingReq(awaiting_result->id);
    } else{
        pub_->publishReq(awaiting_result->id, *req_msg);
    }
    return awaiting_result;
}

void ClientBaseImpl::waitIterProc(AwaitingResult *awaiting_result, unsigned int timeout_ms) {
    // wait until got result or timeout
    bool got_result = true;
    unique_lock result_lock(awaiting_result->mu);
    auto stop_waiting = [&awaiting_result](){
        return awaiting_result->response!=nullptr || awaiting_result->status==SrvStatus::Cancelled;};
    if (timeout_ms){
        // gcc bug before gcc10? https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41861
        got_result = awaiting_result->cv.wait_for(result_lock, chrono::milliseconds(timeout_ms), stop_waiting);
    } else{
        awaiting_result->cv.wait(result_lock, stop_waiting);
    }

    // erase only if timeout to reduce lock
    if (!got_result){
        {
            lock_guard lock(AwaitingResult::wait_list_mu_);
            size_t n_removed = AwaitingResult::wait_list_.erase(awaiting_result->id);
        }
        // n_removed could be 0 if server just responded after wait_for finished
        awaiting_result->status = SrvStatus::Timeout;
    } else{
        // awaiting_result is set by server
        if (awaiting_result->status == SrvStatus::OK)
            AssertLog(awaiting_result->response, "ok but null");
    }
}

shared_ptr<AwaitingResult>
ClientBaseImpl::callIntraProc(const shared_ptr<const void> &req_obj, unsigned int timeout_ms) {
    auto awaiting_result = sendIntraProc(req_obj);
    waitIntraProc(awaiting_result.get(), timeout_ms);
    return awaiting_result;
}

std::shared_ptr<AwaitingResult> ClientBaseImpl::sendIntraProc(const shared_ptr<const void> &req_obj) {
    shared_ptr<AwaitingResult> awaiting_result = AwaitingResult::CreateIntraP();
    awaiting_result->request = req_obj;
    // no need to add to wait_list
    bool scheduled = pub_->scheduleIntraProc(awaiting_result);
    if (!scheduled){
        awaiting_result->status = SrvStatus::NoConnections;
    }
    return awaiting_result;
}

void ClientBaseImpl::waitIntraProc(AwaitingResult* awaiting, unsigned int timeout_ms) {
    if (awaiting->status == SrvStatus::NoConnections) return;

    // wait until got result or timeout
    bool got_result = true;
    unique_lock result_lock(awaiting->mu);
    auto stop_waiting = [&awaiting](){
        return awaiting->response!=nullptr || awaiting->status==SrvStatus::Cancelled;};
    if (timeout_ms){
        // gcc bug before gcc10? https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41861
        got_result = awaiting->cv.wait_for(result_lock, chrono::milliseconds(timeout_ms), stop_waiting);
    } else{
        awaiting->cv.wait(result_lock, stop_waiting);
    }

    if (!got_result){
        awaiting->status = SrvStatus::Timeout;
    } else{
        // awaiting_result is set by server
        if (awaiting->status == SrvStatus::OK)
            AssertLog(awaiting->response, "ok but null");
    }
}

bool ClientBaseImpl::ping(unsigned int timeout_ms, unsigned int retry_ms) {
    ivtb::StopwatchMono stopwatch;
    do{
        // check intraP
        if (pub_->connectedIntraProc()) return true;

        // ping interP
        if (pub_->isConnected()){
            auto awaiting_result = sendInterProc(nullptr, true);
            waitIterProc(awaiting_result.get(), retry_ms);
            if (awaiting_result->status==SrvStatus::OK) return true;
        } else {
            this_thread::sleep_for(retry_ms*1ms);
        }
    } while(stopwatch.passedMs() < timeout_ms);
    return false;
}

bool ClientBaseImpl::Cancel(AwaitingResult &awaiting) {
    lock_guard lock(awaiting.mu);
    if (awaiting.status!=SrvStatus::OK) return false;
    awaiting.status = SrvStatus::Cancelled;
    awaiting.cv.notify_one();
    return true;
}

}

#undef TAG