//
// Created by Ivan B on 2021/3/24.
//

#ifndef RUM_CORE_INTERNAL_PUBLISHER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_PUBLISHER_BASE_IMPL_H_

#include <unordered_set>

#include <rum/extern/zmq/zmq.hpp>
#include <rum/common/def.h>
#include "subscriber_base_impl.h"

namespace rum {

class PublisherBaseImpl {
  private:
    const std::shared_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> zmq_publisher_; RUM_LOCK_BY(zmq_mu_)
    std::string tcp_addr_;
    std::string ipc_addr_;
    const bool to_bind_;
    std::mutex zmq_mu_;
    zmq::message_t msg_header_;

    // the reason conn_list_ is not guarded by zmq_mu_ is to speed up frequent sync connection check
    std::unordered_set<std::string> conn_list_;

    std::function<void()> destr_callback_; RUM_LOCK_BY(destr_mu_)
    std::mutex destr_mu_;

  public:
    const std::string topic_;
    const std::string protocol_;

  private:
  protected:

  public:
    PublisherBaseImpl(std::string topic, std::string protocol, std::shared_ptr<zmq::context_t> context,
                      bool to_bind = false);

    virtual ~PublisherBaseImpl();

    // unsafe raw binding
    bool bindTcpRaw(const std::string &addr= "");
    bool bindIpcRaw();

    bool connect(const std::string &addr);
    bool disconnect(const std::string &addr);
    inline std::mutex& getPubMutex(){return zmq_mu_;}
    bool isConnected();

    // void addItcSub(SubscriberBaseImpl *sub_wp);

    bool publishIpc(zmq::message_t &header, zmq::message_t &body);
    bool publishIpc(zmq::message_t &body);
    bool publishIpc(zmq::message_t &&body){ return publishIpc(body);}

    bool scheduleItc(const std::shared_ptr<const void> &msg);

    static int send(zmq::socket_t &socket, zmq::message_t &message, bool wait);

    void setDestrCallback(const std::function<void()> &destr_callback);
};

}
#endif //RUM_CORE_INTERNAL_PUBLISHER_BASE_IMPL_H_
