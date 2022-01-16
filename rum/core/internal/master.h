//
// Created by Ivan B on 2021/4/5.
//

#ifndef RUM_CORE_MASTER_H_
#define RUM_CORE_MASTER_H_

#include "sub_container.h"
#include "publisher_base_impl.h"
#include <rum/common/zmq_helper.h>

namespace rum {
class Master {
    static std::unique_ptr<Master> master_;

  private:
    const std::shared_ptr<zmq::context_t> context_;
    std::unique_ptr<PublisherBaseImpl> pub_;
    std::unique_ptr<SubContainer> sub_container_;

    std::unique_ptr<std::thread> bind_t_;
    std::condition_variable bind_cv_;
    bool to_bind_ = true;
    std::mutex bind_mu_;

    std::unique_ptr<zmq::socket_t> monitor_socket_;
    std::unique_ptr<ZmqMonitor> monitor_;
  public:

  private:
    void syncForward(zmq::message_t& msg);
  public:
    explicit Master(std::shared_ptr<zmq::context_t> context);
    virtual ~Master();
    // yeah I'm lazy
    Master(const Master &) = delete;
    void operator=(const Master&) = delete;
    Master(Master &&) = delete;
    Master& operator=(Master &&) = delete;

    // debugs
    static std::unique_ptr<Master>& DbgGetGlobalMaster(){return master_;}
    std::atomic<bool> active{false};
};

}

#endif //RUM_CORE_MASTER_H_
