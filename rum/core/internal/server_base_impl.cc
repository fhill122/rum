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

        // handle ping
        if (request->protocol==kPingProtocol){
            AssertLog(!request->client_id.empty(), "");
            lock_guard lock(pubs_mu_);
            auto itr = pubs_.find(request->client_id);
            if (itr==pubs_.end()){
                // request is faster than sync, pub not created yet
                // todo ivan. wait? quick end instead of letting client wait?
                log.w(string(srvName()), "request is faster than sync, pub %s not created yet",
                      request->client_id.c_str());
            } else{
                itr->second->publishPingRep(request->id);
            }
            return;
        }

        // todo ivan. doing here. how to make sure safe removal? sub removed, long ongoing task is still here, when pub called it could be a null
        std::shared_ptr<Message> rep_message;
        auto request_message = static_pointer_cast<const Message>(request->request);
        bool ok = srv_func(request_message, request->protocol, rep_message);
        AssertLog(!request->client_id.empty(), "");
        lock_guard lock(pubs_mu_);
        auto itr = pubs_.find(request->client_id);
        if (itr==pubs_.end()){
            // request is faster than sync, pub not created yet
            // todo ivan. wait? quick end instead of letting client wait?
            log.w(string(srvName()), "request is faster than sync, pub %s not created yet",
                  request->client_id.c_str());
            return;
        }
        if (ok){
            itr->second->publishRepIpc(request->id, 0, *rep_message);
        } else{
            itr->second->publishRepIpc(request->id, 1, *rep_message);
        }
    };
}

void ServerBaseImpl::addPub(PublisherBaseImpl* pub) {
    lock_guard lock(pubs_mu_);
    auto itr = pubs_.find(pub->cli_id());
    AssertLog(itr==pubs_.end(), "");
    pubs_[pub->topic_] = pub;
}

PublisherBaseImpl *ServerBaseImpl::removePub(const std::string &cli_id) {
    lock_guard lock(pubs_mu_);
    auto itr = pubs_.find(cli_id);
    AssertLog(itr!=pubs_.end(), "");
    auto *pub = itr->second;
    pubs_.erase(itr);
    return pub;
}

}