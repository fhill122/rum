//
// Created by Ivan B on 2021/4/16.
//

#ifndef RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_

#include "subscriber_base_impl.h"
#include "publisher_base_impl.h"
#include "master.h"
#include "itc_manager.h"

namespace rum {

class NodeBaseImpl {
  private:
    const std::string name_;
    std::vector<std::pair<std::string, std::string>> domains_;
    const int nid_;

    std::unique_ptr<SubContainer> sub_container_;
    std::unique_ptr<SubContainer> syncsub_container_;
    std::unique_ptr<PublisherBaseImpl> sync_pub_;
    std::unique_ptr<SubscriberBaseImpl> sync_sub_;
    std::vector<PublisherBaseImpl*> pub_list_;


    // thread to broadcast sync, include heartbeat
    std::unique_ptr<std::thread> sync_t_;
    std::unordered_map<std::string, std::string> topics_; RUM_LOCK_BY(sync_mu_)
    std::mutex sync_mu_;
    std::condition_variable hb_cv_;
    unsigned long sync_version_ = 0;
    std::atomic<bool> is_down_{false};

    std::vector<std::string> connections_;
    std::atomic_flag connected = ATOMIC_FLAG_INIT;

    static ItcManager itc_manager_;
    static std::atomic_int id_pool_;
  public:
    const std::shared_ptr<zmq::context_t> context_;

  private:
    void syncCb(zmq::message_t& msg);
    void syncF();

  public:
    NodeBaseImpl(std::string name = "", std::string domain = "",
                 std::string addr = "");
    std::unique_ptr<SubscriberBaseImpl> createSubscriber();

    void registerSubscriber(SubscriberBaseImpl *sub);

    void registerPublisher(PublisherBaseImpl *pub);

    void unregisterSubscriber(SubscriberBaseImpl *sub);

    void unregisterPublisher(PublisherBaseImpl *pub);

    void shutdown();

    /**
     * connect to masters. call this only once
     * @param addrs vector of <master addr in, master addr out>
     */
    void connect(const std::vector<std::pair<std::string, std::string>> &addrs);
};

}
#endif //RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_
