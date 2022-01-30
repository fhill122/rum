/*
 * Created by Ivan B on 2022/1/20.
 */

#ifndef RUM_CORE_INTERNAL_CLIENT_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_CLIENT_BASE_IMPL_H_

#include <unordered_map>

#include "publisher_base_impl.h"
#include "subscriber_base_impl.h"
#include "rum/common/srv_def.h"
#include "awaiting_result.h"

namespace rum{

class ClientBaseImpl {

  private:
  public:
    PublisherBaseImpl* pub_;
    // we need a dummy response sub to set up syncs
    SubscriberBaseImpl* sub_;
    std::string service_name_;

    // if this is static, we could eliminate sub_
    static inline std::unordered_map<unsigned int, AwaitingResult*> wait_list_{};  RUM_LOCK_BY(wait_list_mu_)
    static inline std::mutex wait_list_mu_{};
    static inline std::shared_ptr<ThreadPool> sub_dumb_tp_ = std::make_shared<ThreadPool>();

  private:
    void subCallback(const std::shared_ptr<const void> &obj);
  public:
    // creation need: srv_name, pub_proto
    ClientBaseImpl(PublisherBaseImpl* pub, SubscriberBaseImpl* sub);

    // should return a future?
    std::shared_ptr<AwaitingResult> callItc(const std::shared_ptr<const void> &req_obj, unsigned int timeout_ms = 0);
    std::unique_ptr<AwaitingResult> callIpc(std::unique_ptr<Message> req_msg, unsigned int timeout_ms = 0);

    // void subItc(const std::shared_ptr<const void>& awaiting_result);
};

}

#endif //RUM_CORE_INTERNAL_CLIENT_BASE_IMPL_H_
