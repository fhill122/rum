//
// Created by Ivan B on 2021/4/16.
//

#ifndef RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_

#include "subscriber_base_impl.h"
#include "publisher_base_impl.h"
#include "client_base_impl.h"
#include "server_base_impl.h"
#include "master.h"
#include "remote_manager.h"
#include "itc_manager.h"
#include "rum/common/node_param.h"
#include "rum/extern/ivtb/scheduler.h"
#include "rum/core/msg/rum_sync_generated.h"

namespace rum {

class NodeBaseImpl {
  public:

  private:
    const std::string name_;
    const int nid_;

    std::shared_ptr<ItcManager> itc_manager_ = ItcManager::GlobalManager();
    std::shared_ptr<RemoteManager> remote_manager_ = RemoteManager::GlobalManager();
    std::unique_ptr<SubContainer> sub_container_;
    std::unique_ptr<SubContainer> syncsub_container_;
    std::unique_ptr<PublisherBaseImpl> sync_pub_;
    std::unordered_map<std::string, std::vector<std::unique_ptr<PublisherBaseImpl>>> pubs_;
    std::unordered_map<std::string, std::unique_ptr<ServerBaseImpl>> servers_;
    // all sync work is enqueued on this single thread tp to make sure thread safe
    std::shared_ptr<ivtb::ThreadPool> sync_tp_ = std::make_shared<ivtb::ThreadPool>(1);

    // thread to broadcast sync, include heartbeat
    ivtb::Scheduler sync_scheduler_{0};
    std::shared_ptr<ivtb::Scheduler::Task> sync_task_;
    std::shared_ptr<flatbuffers::FlatBufferBuilder> sync_fb_builder_ = nullptr;
    unsigned long sync_fb_version_ = 0;
    std::atomic<bool> is_down_{false};

    std::vector<std::string> connections_;
    std::atomic_flag connected = ATOMIC_FLAG_INIT;

    static std::atomic_int id_pool_;
  public:
    const std::shared_ptr<zmq::context_t> context_;
    const NodeParam param_;

  private:
    void updatePubConnection(const msg::NodeId *remote_node, RemoteManager::NodeUpdate &update);
    void syncCb(const zmq::message_t& msg);
    // broadcast self sync info
    void syncFunc();
    void checkRemote();
    bool shouldConnectIpc(const msg::NodeId *sync) const;
    bool shouldConnectTcp(const msg::NodeId *sync) const;
    PublisherBaseImpl* internalAddPublisher(
            const std::string &topic, const std::string &protocol, msg::MsgType msg_type); RUM_THREAD_UNSAFE


  public:
    explicit NodeBaseImpl(std::string name = "", NodeParam param = NodeParam());
    virtual ~NodeBaseImpl();

    SubscriberBaseImpl*
    addSubscriber(const std::string &topic,
                  const std::shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
                  const IpcFunc &ipc_cb,
                  const ItcFunc &itc_cb,
                  const DeserFunc<> &deserialize_f,
                  const std::string &protocol = ""); RUM_THREAD_SAFE

    void removeSubscriber(SubscriberBaseImpl* &sub); RUM_THREAD_SAFE

    PublisherBaseImpl* addPublisher(const std::string &topic, const std::string &protocol,
                                    msg::MsgType msg_type = msg::MsgType_Message); RUM_THREAD_SAFE

    void removePublisher(PublisherBaseImpl* &pub); RUM_THREAD_SAFE

    ClientBaseImpl* addClient(const std::string &srv_name, const std::string &pub_protocol); RUM_THREAD_SAFE

    void removeClient(ClientBaseImpl* &client);  RUM_THREAD_SAFE

    ServerBaseImpl* addServer(const std::string &srv_name,
                              const std::shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
                              const SrvIpcFunc &ipc_func,
                              const SrvItcFunc &itc_func,
                              const std::string &sub_protocol = "",
                              const std::string &pub_protocol = "");  RUM_THREAD_SAFE

    void removeServer(ServerBaseImpl* &server);  RUM_THREAD_SAFE

    // todo ivan. should we ever call it? especially for rumassembly modules
    void shutdown();

    /**
     * connect to master. call this only once
     * @param addr_in income address
     * @param addr_out outgoing address
     */
    void connect(const std::string &addr_in, const std::string &addr_out); RUM_THREAD_UNSAFE

    inline std::unique_ptr<SubContainer>& dbgGetSubContainer(){return sub_container_;};
};

}
#endif //RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_
