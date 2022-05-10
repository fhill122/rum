/*
 * Created by Ivan B on 2022/1/20.
 */

#ifndef RUM_CORE_INTERNAL_SERVER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_SERVER_BASE_IMPL_H_

#include "client_base_impl.h"

namespace rum{

class ServerBaseImpl {

  private:
  public:
    // <cli_id, pub>
    std::unordered_map<std::string, PublisherBaseImpl*> pubs_;  RUM_LOCK_BY(pubs_mu_)
    std::mutex pubs_mu_;
    SubscriberBaseImpl* sub_ = nullptr;
    // std::string service_name_;
    const std::string pub_protocol_;

  private:
  public:
    explicit ServerBaseImpl(const std::string &pub_protocol) : pub_protocol_(pub_protocol) {}

    inline void setSub(SubscriberBaseImpl *sub) {sub_ = sub;}

    IntraProcFunc genSubIntraProc(const SrvIntraProcFunc& srv_func);
    InterProcFunc genSubInterProc(const SrvInterProcFunc& srv_func);

    std::string_view srvName() const {return SrvFromReqTopic(sub_->topic_);}

    void addPub(PublisherBaseImpl* pub);
    PublisherBaseImpl* removePub(const std::string &cli_id);
};
}

#endif //RUM_CORE_INTERNAL_SERVER_BASE_IMPL_H_
