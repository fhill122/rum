/*
 * Created by Ivan B on 2022/1/8.
 */

#ifndef RUM_COMMON_MESSAGE_H_
#define RUM_COMMON_MESSAGE_H_

#include <rum/extern/zmq/zmq.hpp>

namespace rum{
    using Message = zmq::message_t;
}

#endif //RUM_COMMON_MESSAGE_H_
