/*
 * Created by Ivan B on 2022/1/20.
 */

#ifndef RUM_CORE_INTERNAL_CLIENT_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_CLIENT_BASE_IMPL_H_

#include <unordered_map>

#include "publisher_base_impl.h"
#include "subscriber_base_impl.h"
#include "rum/common/srv_def.h"
#include "srv_common.h"

namespace rum{

class ClientBaseImpl {

  private:
  public:
    PublisherBaseImpl* pub_;
    // we need a dummy response sub to set up syncs
    SubscriberBaseImpl* sub_;

    static inline std::shared_ptr<ThreadPool> sub_dumb_tp_ = std::make_shared<ThreadPool>();

  private:
  public:
    // creation need: srv_name, pub_proto
    ClientBaseImpl(PublisherBaseImpl* pub, SubscriberBaseImpl* sub);

    std::shared_ptr<AwaitingResult> callItc(const std::shared_ptr<const void> &req_obj, unsigned int timeout_ms = 0);
    std::shared_ptr<AwaitingResult> sendItc(const std::shared_ptr<const void> &req_obj);
    void waitItc(AwaitingResult* awaiting, unsigned int timeout_ms = 0);

    std::unique_ptr<AwaitingResult> callIpc(std::unique_ptr<Message> req_msg, unsigned int timeout_ms = 0);
    std::unique_ptr<AwaitingResult> sendIpc(std::unique_ptr<Message> req_msg, bool ping=false);
    void waitIpc(AwaitingResult* awaiting, unsigned int timeout_ms = 0);

    bool ping(unsigned int timeout_ms, unsigned int retry_ms);

    static bool Cancel(AwaitingResult &awaiting);

};

}

#endif //RUM_CORE_INTERNAL_CLIENT_BASE_IMPL_H_
