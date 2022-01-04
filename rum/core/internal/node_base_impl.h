//
// Created by Ivan B on 2021/4/16.
//

#ifndef RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_
#define RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_

#include "subscriber_base_impl.h"
#include "publisher_base_impl.h"
#include "master.h"
#include <rum/extern/ivtb/scheduler.h>

namespace rum {

class NodeBaseImpl {
  public:
    struct Param{
        bool enable_ipc_socket = true;
        inline Param() {};
    };

  private:
    const std::string name_;
    std::pair<std::string, std::string> domain_;
    const int nid_;

    std::unique_ptr<SubContainer> sub_container_;
    std::unique_ptr<SubContainer> syncsub_container_;
    std::unique_ptr<PublisherBaseImpl> sync_pub_;
    std::unordered_map<std::string, std::vector<std::unique_ptr<PublisherBaseImpl>>> pubs_;
    std::shared_ptr<ivtb::ThreadPool> sync_tp_ = std::make_shared<ivtb::ThreadPool>(1);

    // thread to broadcast sync, include heartbeat
    ivtb::Scheduler sync_scheduler_{0};
    std::shared_ptr<ivtb::Scheduler::Task> sync_task_;
    std::atomic<unsigned long> sync_version_{0};
    std::atomic<bool> is_down_{false};

    std::vector<std::string> connections_;
    std::atomic_flag connected = ATOMIC_FLAG_INIT;

    static std::atomic_int id_pool_;
  public:
    const std::shared_ptr<zmq::context_t> context_;
    const Param param_;

  private:
    void syncCb(zmq::message_t& msg);
    void syncF();


  public:
    explicit NodeBaseImpl(std::string name = "", Param param = Param());

    SubscriberBaseImpl* addSubscriber(const std::string &topic,
              const std::shared_ptr<ivtb::ThreadPool> &tp, size_t queue_size,
              const std::function<void(zmq::message_t&)> &ipc_cb,
              const std::function<void(const void *)> &itc_cb,
              std::string protocol = "");

    void removeSubscriber(SubscriberBaseImpl* &sub);

    PublisherBaseImpl * addPublisher(const std::string &topic, const std::string &protocol); RUM_THREAD_UNSAFE

    void removePublisher(PublisherBaseImpl *pub); RUM_THREAD_UNSAFE

    void shutdown();

    /**
     * connect to master. call this only once
     * @param addr_in income address
     * @param addr_out outgoing address
     */
    void connect(const std::string &addr_in, const std::string &addr_out);
};

}
#endif //RUM_CORE_INTERNAL_NODE_BASE_IMPL_H_
