/*
 * Created by Ivan B on 2022/1/20.
 */

#ifndef RUM_CORE_INTERNAL_SERVER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_SERVER_BASE_IMPL_H_

#include "client_base_impl.h"

namespace rum{

class ServerBaseImpl {

  private:
    // <topic, pub>
    std::unordered_map<std::string, PublisherBaseImpl*> pubs_;  RUM_LOCK_BY(pubs_mu_)
    std::mutex pubs_mu_;
    // we need a dummy response sub to setup syncs
    SubscriberBaseImpl* sub_;
    std::string service_name_;
  public:

  private:
  public:
    void ipcCallback();
    ItcFunc genSubItc(const SrvItcFunc& srv_func);
    IpcFunc genSubIpc(const SrvIpcFunc& srv_func);

};
}

#endif //RUM_CORE_INTERNAL_SERVER_BASE_IMPL_H_
