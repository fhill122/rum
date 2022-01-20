//
// Created by Ivan B on 2021/3/24.
//

#ifndef RUM_CORE_SUB_CONTAINER_H_
#define RUM_CORE_SUB_CONTAINER_H_

#include <unordered_map>
#include <unordered_set>

#include <rum/extern/zmq/zmq.hpp>
#include <rum/common/def.h>
#include "rum/core/internal/subscriber_base_impl.h"


namespace rum {

class NodeBaseImpl;

class SubContainer {
    friend NodeBaseImpl;

  private:
    // <topic, list of sub raw>
    using SubBook = std::unordered_map<std::string,
                    std::vector<std::unique_ptr<SubscriberBaseImpl>>>;

    struct InternalTask{
        enum TaskType {kAdd, kRemove, kStop};
        TaskType type;
        std::string content;
    };

  private:
    const std::shared_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> zmq_subscriber_;
    std::unique_ptr<zmq::socket_t> zmq_interrupter_;
    std::string tcp_addr_;
    std::string ipc_addr_;
    const bool to_bind_;

    // note: connections initiated by the other port is not included
    std::unordered_set<std::string> conn_list_;
    // consider shared_mutex when adapt c++17
    std::mutex subs_mu_;
    // std::mutex topics_mu_;
    SubBook subs_; RUM_LOCK_BY(subs_mu_)

    std::unique_ptr<std::thread> loop_t_;

  public:

  private:
    bool receive(zmq::message_t *msg);
    bool getMsg(zmq::message_t &header, zmq::message_t &body);
    bool loop();
    void interrupt();

  public:
    explicit SubContainer(std::shared_ptr<zmq::context_t> context, bool to_bind = true);
    virtual ~SubContainer();

    // unsafe raw operations, do these before connectRaw (update ivan. why?)
    bool bindTcpRaw(const std::string &addr= "");
    bool bindIpcRaw();
    bool connectRaw(const std::string &addr);

    bool start();
    virtual void stop();

    /**
     * Add a sub
     * @param sub sub
     * @return true if it creates a new topic
     */
    bool addSub(std::unique_ptr<SubscriberBaseImpl> sub);

    /**
     * Remove a sub
     * @param sub sub
     * @return true if it removes a topic
     */
    bool removeSub(SubscriberBaseImpl *sub);

    std::vector<SubscriberBaseImpl*> getSubs();

    void clearSubs();

    [[nodiscard]] inline const std::string &getTcpAddr() const { return tcp_addr_;}
    [[nodiscard]] inline const std::string &getIpcAddr() const { return ipc_addr_;}
};

}
#endif //RUM_CORE_SUB_CONTAINER_H_
