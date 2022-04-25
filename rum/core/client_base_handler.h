/*
 * Created by Ivan B on 2022/2/6.
 */

#ifndef RUM_CORE_CLIENT_BASE_HANDLER_H_
#define RUM_CORE_CLIENT_BASE_HANDLER_H_

#include "rum/common/srv_def.h"
#include "rum/common/message.h"

namespace rum{

class ClientBaseImpl;
class AwaitingResult;
class NodeBase;

template <class RepT>
struct Result{
    SrvStatus status = SrvStatus::OK;
    std::shared_ptr<RepT> response =  nullptr;
};

/*
 * class used to control a single service call
 */
struct CallHandler{
    // internal implementation
    std::shared_ptr<AwaitingResult> pimpl_ = nullptr;

    /**
     * Get current SrvStatus of the service call.
     * todo ivan. this is kind of ugly that we expose this internal api
     * @return status
     */
    SrvStatus getCurrentStatus() const;

    /**
     * cancel waiting, unblock the call
     * @return whether is cancelled, false if call already finished
     */
    bool cancel() const;
};


class ClientBaseHandler {
  private:
    using MessageWithProto = std::pair<std::shared_ptr<Message>, std::string>;
    friend NodeBase;
    ClientBaseImpl* pimpl_ = nullptr;
    inline ClientBaseHandler() = default;

  public:
    explicit ClientBaseHandler(ClientBaseImpl *pimpl);

    inline virtual ~ClientBaseHandler() = default;

    bool isConnected() const;

    CallHandler sendItc(const std::shared_ptr<const void> &request);
    Result<void> waitItc(CallHandler &&call_handler, unsigned int timeout_ms = 0);
    Result<void> callItc(const std::shared_ptr<const void> &request, unsigned int timeout_ms = 0);

    CallHandler sendIpc(std::unique_ptr<Message> request);
    Result<MessageWithProto> waitIpc(CallHandler &&call_handler, unsigned int timeout_ms = 0);
    Result<MessageWithProto> callIpc(std::unique_ptr<Message> request, unsigned int timeout_ms = 0);

    /**
     * Ping to check server connectivity
     * @param timeout_ms total timeout in ms, considered connected if response received within this time
     * @param retry_ms ping period
     * @return whether connected
     */
    bool ping(unsigned int timeout_ms, unsigned int retry_ms);
};

}
#endif //RUM_CORE_CLIENT_BASE_HANDLER_H_
