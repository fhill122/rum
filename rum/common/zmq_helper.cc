//
// Created by Ivan B on 2021/3/17.
//

#include "zmq_helper.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <random>

#include <rum/common/common.h>
#include <rum/common/log.h>
#include <rum/extern/ThreadUtil.h>

using namespace std;

bool StrStartWith(string full, string head){
    return strncmp(full.c_str(), head.c_str(), head.size()) == 0;
}

namespace rum{

const shared_ptr<zmq::context_t>& shared_context(){
    // proper implementation
    static std::shared_ptr<zmq::context_t> context = make_shared<zmq::context_t>(1);
    return context;

    // work around to keep context longer
    // static shared_ptr<zmq::context_t> *context = new shared_ptr<zmq::context_t>(new zmq::context_t(1));
    // return *context;
}

vector<NetInterface> GetNetInterfaces(){
    vector<NetInterface> result;
    
    struct ifaddrs *ifAddrStruct = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tmpAddrPtr = nullptr;
    string ipAddr;

    getifaddrs(&ifAddrStruct);

    int n=0;
    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {++n;}
    result.resize(n);

    for (n=0, ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        NetInterface &interface = result[n++];
        interface.name = ifa->ifa_name;

        if (!ifa->ifa_addr) {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            interface.ip = addressBuffer;
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            // tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            // char addressBuffer[INET6_ADDRSTRLEN];
            // inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            // printf("%s IPv6 Address %s\n", ifa->ifa_name, addressBuffer);
        }

    }
    if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
    return result;

}

std::string GuessIp(const std::vector<NetInterface> &interfaces) {
    string ip_addr;

    std::string backup;
    for(const auto &i : interfaces){
        if (i.ip.empty()) continue;

        if (StrStartWith(i.name, "eth0")
            || StrStartWith(i.name, "enp")
            || StrStartWith(i.name, "en") //mac os
        ) {
            ip_addr = i.ip;
            break;
        }
        // if it is a wireless card, store address, keep checking
        else if (StrStartWith(i.name, "wl")) {
            if (backup.empty())
                backup = i.ip;
        }
    }

    if (ip_addr.empty()) ip_addr = backup;

    return ip_addr;
}

string GenTcpAddr() {
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    static std::uniform_int_distribution<> dis(49152, 65534);
    int port = dis(gen);
    return "tcp://" + ip + ":" + to_string(port);
}

string GenIpcAddr() {
    static atomic_uint64_t idPool{0};
    return "ipc:///tmp/rum_ipc_" + to_string(pid) + "_" + to_string(idPool++);
}

string GenItcAddr() {
    static atomic_uint64_t idPool{0};
    return "inproc://rum_" + to_string(pid) + "_" + to_string(idPool++);
}

string BindRandTcp(zmq::socket_t &socket, int trial) {
    for (int i = 0; i < trial; ++i) {
        string addr;
        try {
            addr = GenTcpAddr();
            socket.bind(addr);
            if (i > 0) {
                log.d(__func__ , "retry success. bind to " + addr);
            }
            return addr;
        }
        catch (...) {
            log.d(__func__, addr + " binding failed, retry");
        }
    }

    log.e(__func__, "failed to bind random tcp address");
    return "";
}

std::string BindIpc(zmq::socket_t &socket) {
    string addr = GenIpcAddr();
    try{
        socket.bind(addr);
    }
    catch (...){
        log.d(__func__, "failed to bind ipc " + addr);
        return "";
    }
    return addr;
}

std::string BindTcp(zmq::socket_t &socket, const string &addr) {
    if (addr.empty()){
        auto addr_rand = BindRandTcp(socket);
        if (addr_rand.empty()) {
            return "";
        }
        return addr_rand;
    }
    else{
        try{
            socket.bind(addr);
            return addr;
        }
        catch (const exception &e){
            // log.d(__func__, "failed to bind %s. err: %s", addr.c_str(), e.what());
            return "";
        }
    }
}

ZmqMonitor::ZmqMonitor(zmq::socket_t &socket, int events) {
    auto addr = GenItcAddr();
    int rc = zmq_socket_monitor(socket.handle(), addr.c_str(), events);
    if (rc != 0)
        throw zmq::error_t();

    _socket = socket;
    _monitor_socket = zmq::socket_t(socket.ctxptr, ZMQ_PAIR);
    _monitor_socket.connect(addr.c_str());
}

ZmqMonitor::~ZmqMonitor() {
    stop();
    _monitor_socket.close();
}

bool ZmqMonitor::monitorOnce(CallbackFunc *f_ptr, int timeout) {
    zmq_msg_t eventMsg;
    zmq_msg_init(&eventMsg);

    zmq::pollitem_t items[] = {
            {_monitor_socket.handle(), 0, ZMQ_POLLIN, 0},
    };
    zmq::poll(&items[0], 1, timeout);

    if (items[0].revents & ZMQ_POLLIN) {
        int rc = zmq_msg_recv(&eventMsg, _monitor_socket.handle(), 0);
        if (rc == -1 && zmq_errno() == ETERM)
            return false;
        assert(rc != -1);

    } else {
        zmq_msg_close(&eventMsg);
        return false;
    }

    const char *data = static_cast<const char *>(zmq_msg_data(&eventMsg));
    zmq_event_t msgEvent;
    memcpy(&msgEvent.event, data, sizeof(uint16_t));
    data += sizeof(uint16_t);
    memcpy(&msgEvent.value, data, sizeof(int32_t));
    zmq_event_t *event = &msgEvent;

    zmq_msg_t addrMsg;
    zmq_msg_init(&addrMsg);
    int rc = zmq_msg_recv(&addrMsg, _monitor_socket.handle(), 0);
    if (rc == -1 && zmq_errno() == ETERM) {
        zmq_msg_close(&eventMsg);
        return false;
    }

    assert(rc != -1);
    const char *str = static_cast<const char *>(zmq_msg_data(&addrMsg));
    std::string address(str, str + zmq_msg_size(&addrMsg));
    zmq_msg_close(&addrMsg);

    (*f_ptr)(*event, address.c_str());
    zmq_msg_close(&eventMsg);
    return true;
}

void ZmqMonitor::start(CallbackFunc f_ptr) {
    AssertLog(!monitor_t_, "already started");
    to_monitor_.store(true, memory_order_release);
    monitor_t_ = make_unique<thread>([&, f_ptr] () mutable {
        tu::NameThread("ZmqMonitor");
        while(to_monitor_.load(memory_order_acquire)){
            monitorOnce(&f_ptr, 0);
        }
    });
}

void ZmqMonitor::stop() {
    zmq_socket_monitor(_socket.handle(), ZMQ_NULLPTR, 0);
    to_monitor_.store(false, memory_order_release);
    if(monitor_t_ && monitor_t_->joinable()) monitor_t_->join();
}


}