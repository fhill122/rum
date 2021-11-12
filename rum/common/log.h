//
// Created by Ivan B on 2021/4/4.
//

#ifndef RUM_COMMON_LOG_H_
#define RUM_COMMON_LOG_H_

#include <rum/extern/ivtb/log.h>

namespace rum{

using Log = ivtb::Log;

// static function could be using theses
inline static Log log{};
inline static Log printer{Log::Level::v, Log::Level::s};

}


#endif //RUM_COMMON_LOG_H_
