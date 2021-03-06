//
// Created by Ivan B on 2021/3/17.
//

#ifndef RUM_CORE_ZMQ_ZMQ_HELPER_H_
#define RUM_CORE_ZMQ_ZMQ_HELPER_H_

#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include <rum/extern/zmq/zmq.hpp>

namespace rum{

struct NetInterface{
    std::string name;
    std::string ip;
    // bool ipv6 = false;
};

struct TcpAddress{
    unsigned char ip[4];
    int port;

    inline bool SameIp(unsigned char ip1[4], unsigned char ip2[4]){
        return ip1[0]==ip2[0] &&
               ip1[1]==ip2[1] &&
               ip1[2]==ip2[2] &&
               ip1[3]==ip2[3];
    }
};

// get global shared context. function instead of variable to ensure static initialization order
const std::shared_ptr<zmq::context_t>& shared_context();

std::vector<NetInterface> GetNetInterfaces();

std::string IpToStr(unsigned char ip[4]);

std::string IpFromTcp(const std::string &tcp_addr);

std::string GuessIp(const std::vector<NetInterface> &interfaces);

std::string GenTcpAddr();

std::string GenIpcAddr();

std::string GenIntraProcAddr();

std::string BindRandTcp(zmq::socket_t &socket, int trial = 100);

std::string BindIpc(zmq::socket_t &socket);

std::string BindTcp(zmq::socket_t &socket, const std::string &addr = "");

inline static int zmq_force_delay = 0; // ms

enum class ZmqOpType{
    Bind, Unbind, Connect, Disconnect, Close
};

void ZmqSyncedOp(zmq::socket_t &socket, ZmqOpType op, const std::string &addr);

// modified from cppzmq, not really my style
class ZmqMonitor{
    // using CallbackFunc = void (const zmq_event_t &event_, const char *addr_);
    using CallbackFunc = std::function<void(const zmq_event_t &event_, const char *addr_)>;
  private:
    // name taken from zmq.hpp
    zmq::socket_t _monitor_socket;
    zmq::socket_ref _socket;

    std::unique_ptr<std::thread> monitor_t_;
    std::atomic<bool> to_monitor_{false};

  private:
    bool monitorOnce(CallbackFunc *f_ptr, int timeout=0);
  public:
    // taking a reference, so make sure socket outlives this monitor.
    explicit ZmqMonitor(zmq::socket_t &socket, int events = ZMQ_EVENT_ALL);
    ~ZmqMonitor();
    // restart not supported
    void start(CallbackFunc f_ptr);
    void stop();

    ZmqMonitor(const ZmqMonitor &) = delete;
    void operator=(const ZmqMonitor &) = delete;
    // well we can have move, but I'm lazy
    ZmqMonitor(ZmqMonitor &&rhs) = delete;
    ZmqMonitor &operator=(ZmqMonitor &&rhs) = delete;
};

}

#endif //RUM_CORE_ZMQ_ZMQ_HELPER_H_
