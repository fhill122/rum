//
// Created by Ivan B on 2021/3/20.
//

#ifndef RUM_CORE_NODE_BASE_H_
#define RUM_CORE_NODE_BASE_H_

#include <utility>

#include "rum/common/thread_pool.h"
#include "rum/common/node_param.h"
#include "rum/common/serialization.h"
#include "subscriber_base_handler.h"
#include "publisher_base_handler.h"
#include "server_base_handler.h"
#include "client_base_handler.h"

namespace rum {

class NodeBaseImpl;

class NodeBase {
  private:
    std::unique_ptr<NodeBaseImpl> pimpl_;

  public:

  protected:
    explicit NodeBase(const std::string &name = "", const NodeParam &param = NodeParam());

  public:
    static bool Init(const NodeParam &param = NodeParam());
    static std::unique_ptr<NodeBase>& GlobalNode(bool auto_init = false);

    virtual ~NodeBase();

    std::string getStrId();

    SubscriberBaseHandler addSubscriber(const std::string &topic,
        const std::shared_ptr<ThreadPool> &tp, size_t queue_size,
        const InterProcFunc &inter_cb,
        const IntraProcFunc &intra_cb,
        const DeserFunc<> &deserialize_f,
        const std::string &protocol);

    void removeSubscriber(SubscriberBaseHandler &subscriber_handler);

    PublisherBaseHandler addPublisher(const std::string &topic, const std::string &protocol);

    void removePublisher(PublisherBaseHandler &publisher_handler);

    ClientBaseHandler addClient(const std::string &srv_name, const std::string &req_protocol);

    void removeClient(ClientBaseHandler &client_handler);

    ServerBaseHandler addServer(const std::string &srv_name,
                                const std::shared_ptr<ThreadPool> &tp, size_t queue_size,
                                const SrvInterProcFunc &inter_f,
                                const SrvIntraProcFunc &intra_f,
                                const std::string &req_protocol,
                                const std::string &rep_protocol);

    void removeServer(ServerBaseHandler &server_handler);
};

}
#endif //RUM_CORE_NODE_BASE_H_
