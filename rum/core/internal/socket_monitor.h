//
// Created by Ivan B on 2021/6/27.
//

#ifndef RUM_CORE_INTERNAL_SOCKET_MONITOR_H_
#define RUM_CORE_INTERNAL_SOCKET_MONITOR_H_

#include <rum/extern/zmq/zmq.hpp>

namespace rum{

// class SocketMonitor : zmq::monitor_t {
//   private:
//     std::unique_ptr<std::thread> monitor_t = nullptr;
//
//
//   private:
//     bool check_event_all(uint16_t *eventId, int timeout = 0);
//
//   public:
//     SocketMonitor(zmq::socket_t &socket, std::string const addr, int events = ZMQ_EVENT_ALL);
//
//     void startMonitor(std::function<void(uint16_t, std::string)>);
//
//     virtual ~SocketMonitor();
//
// };


}

#endif //RUM_CORE_INTERNAL_SOCKET_MONITOR_H_
