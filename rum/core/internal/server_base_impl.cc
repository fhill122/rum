/*
 * Created by Ivan B on 2022/1/20.
 */

#include "server_base_impl.h"
#include "rum/common/log.h"

using namespace std;

namespace rum{

ItcFunc ServerBaseImpl::genSubItc(const SrvItcFunc& srv_func) {
    return [srv_func](const shared_ptr<const void> &awaiting_result_void){
        // const cast is ok as client build it in callItc without const
        shared_ptr<AwaitingResult> awaiting_result =
                const_pointer_cast<AwaitingResult>(
                    static_pointer_cast<const AwaitingResult>(awaiting_result_void)
                );

        bool ok = srv_func(awaiting_result->request, awaiting_result->response);

        lock_guard lock(awaiting_result->mu);
        // too late, client already moved on
        if (awaiting_result->status!= SrvStatus::OK) return;
        if (ok){
            awaiting_result->status = SrvStatus::OK;
            AssertLog(awaiting_result->response, "ok but null");
        }else{
            awaiting_result->status = SrvStatus::ServerErr;
        }
        // for itc case we directly set and notify, skipped pub and sub
        awaiting_result->cv.notify_one();
    };
}

IpcFunc ServerBaseImpl::genSubIpc(const SrvIpcFunc &srv_func) {
    return [this, srv_func](const std::shared_ptr<const void> &request_void){
        shared_ptr<SubscriberBaseImpl::SrvIpcRequest::Content> request =
                const_pointer_cast<SubscriberBaseImpl::SrvIpcRequest::Content>(
                        static_pointer_cast<const SubscriberBaseImpl::SrvIpcRequest::Content>(request_void)
                );

        // todo ivan. doing here. how to make sure safe removal? sub removed, long ongoing task is still here, when pub called it could be a null
        std::shared_ptr<Message> rep_message;
        bool ok = srv_func(static_pointer_cast<const Message>(request->request), request->protocol, rep_message);
        AssertLog(!request->pub_topic.empty(), "");
        lock_guard lock(pubs_mu_);
        auto itr = pubs_.find(request->pub_topic);
        if (itr==pubs_.end()){
            // request is faster than sync, pub not created yet
            return;
        }
        if (ok){
            itr->second->publishRepIpc(request->id, 0, *rep_message);
        } else{
            itr->second->publishRepIpc(request->id, 1, *rep_message);
        }
    };
}

}