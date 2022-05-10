//
// Created by Ivan B on 2021/3/24.
//

#ifndef RUM_CORE_INTERNAL_PUBLISHER_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_PUBLISHER_BASE_IMPL_H_

#include <unordered_set>

#include "rum/extern/zmq/zmq.hpp"
#include "rum/common/def.h"
#include "../msg/rum_header_generated.h"
#include "subscriber_base_impl.h"

namespace rum {

// todo ivan. should we create derived classes for req and rep publisher?
class PublisherBaseImpl {
  private:
    const std::shared_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> zmq_publisher_; RUM_LOCK_BY(zmq_mu_)
    std::string tcp_addr_;
    std::string ipc_addr_;
    const bool to_bind_;
    std::mutex zmq_mu_;
    zmq::message_t topic_header_;
    std::string cli_id_;

    // the reason conn_list_ is not guarded by zmq_mu_ is to speed up frequent sync connection check
    std::unordered_set<std::string> conn_list_;

    std::function<void()> destr_callback_; RUM_LOCK_BY(destr_mu_)
    std::mutex destr_mu_;

  public:
    const std::string topic_;
    const std::string protocol_;
    const msg::MsgType msg_type_;

  private:
    // flatbuffers::FlatBufferBuilder* createRepFbs(unsigned int id, char status, bool ping);

  public:
    PublisherBaseImpl(std::string topic, std::string protocol, std::shared_ptr<zmq::context_t> context,
                      bool to_bind = false, msg::MsgType msg_type = msg::MsgType::MsgType_Message);

    virtual ~PublisherBaseImpl();

    // unsafe raw binding
    bool bindTcpRaw(const std::string &addr= "");
    bool bindIpcRaw();

    bool connect(const std::string &addr); RUM_THREAD_UNSAFE
    bool disconnect(const std::string &addr); RUM_THREAD_UNSAFE
    inline std::mutex& getPubMutex(){return zmq_mu_;}
    bool isConnected();

    // topic. inter-proc send
    bool publish(zmq::message_t &header, zmq::message_t &body);
    bool publish(zmq::message_t &body);
    bool publish(zmq::message_t &&body){ return publish(body);}

    // srv
    bool publishReq(unsigned int id, zmq::message_t &body, const char* protocol= nullptr);
    bool publishPingReq(unsigned int id){
        zmq::message_t msg{0};
        return publishReq(id, msg, kPingProtocol);
    }

    bool publishRep(unsigned int id, char status, zmq::message_t &body, const char* protocol= nullptr);
    bool publishPingRep(unsigned int id){
        zmq::message_t msg{0};
        return publishRep(id, 0, msg, kPingProtocol);
    }

    // intra-proc send
    bool scheduleIntraProc(const std::shared_ptr<const void> &msg);
    bool connectedIntraProc();

    static int send(zmq::socket_t &socket, zmq::message_t &message, bool wait);

    void setDestrCallback(const std::function<void()> &destr_callback);

    inline const std::string& cli_id() {return cli_id_;};

    inline void set_cli_id(const std::string &cli_id) {cli_id_ = cli_id;}

};

}
#endif //RUM_CORE_INTERNAL_PUBLISHER_BASE_IMPL_H_
