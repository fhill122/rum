/*
 * Created by Ivan B on 2022/2/6.
 */

#include "client_base_handler.h"
#include "internal/client_base_impl.h"

using namespace std;

namespace rum{

rum::ClientBaseHandler::ClientBaseHandler(rum::ClientBaseImpl *pimpl) : pimpl_(pimpl) {}

bool ClientBaseHandler::isConnected() const {
    return pimpl_->pub_->isConnected();
}

SrvStatus CallHandler::getCurrentStatus() const {
    if (!pimpl_) return SrvStatus::Unknown;
    return pimpl_->status;
}

bool CallHandler::cancel() const {
    if (!pimpl_) return false;
    return ClientBaseImpl::Cancel(*pimpl_);
}

CallHandler ClientBaseHandler::sendItc(const shared_ptr<const void> &request) {
    return CallHandler{pimpl_->sendItc(request)};
}

Result<void> ClientBaseHandler::waitItc(CallHandler &&call_handler, unsigned int timeout_ms) {
    pimpl_->waitItc(call_handler.pimpl_.get(), timeout_ms);
    return Result<void>{call_handler.pimpl_->status, move(call_handler.pimpl_->response)};
}

Result<void> ClientBaseHandler::callItc(const shared_ptr<const void> &request, unsigned int timeout_ms) {
    return waitItc(sendItc(request), timeout_ms);
}

CallHandler ClientBaseHandler::sendIpc(std::unique_ptr<Message> request) {
    return CallHandler{pimpl_->sendIpc(move(request))};
}

Result<ClientBaseHandler::MessageWithProto>
ClientBaseHandler::waitIpc(CallHandler &&call_handler, unsigned int timeout_ms) {
    pimpl_->waitIpc(call_handler.pimpl_.get(), timeout_ms);
    return Result<MessageWithProto>{call_handler.pimpl_->status,
                                    static_pointer_cast<MessageWithProto>(call_handler.pimpl_->response)};
}

Result<ClientBaseHandler::MessageWithProto>
ClientBaseHandler::callIpc(std::unique_ptr<Message> request, unsigned int timeout_ms) {
    return waitIpc(sendIpc(move(request)), timeout_ms);
}

bool ClientBaseHandler::ping(unsigned int timeout_ms, unsigned int retry_ms) {
    return pimpl_->ping(timeout_ms, retry_ms);
}

}
