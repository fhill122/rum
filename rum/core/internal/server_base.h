//
// Created by Ivan B on 2021/3/15.
//

#ifndef RUM_CORE_SERVER_BASE_H_
#define RUM_CORE_SERVER_BASE_H_

#include <rum/extern/zmq/zmq.hpp>
#include "rum/core/msg_dispatcher.h"

namespace rum {

class ServerBase {

  private:
    std::shared_ptr<MsgDispatcher> msg_dispatcher_;
    const std::string name_;
    std::string addr_;
    const std::shared_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> socket_;

  public:

  private:

  public:
    ServerBase(std::string name, const std::shared_ptr<zmq::context_t> &context);

};

}

#endif //RUM_CORE_SERVER_BASE_H_
