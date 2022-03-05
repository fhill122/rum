/*
 * Created by Ivan B on 2022/1/21.
 */

#ifndef RUM_COMMON_SRV_DEF_H_
#define RUM_COMMON_SRV_DEF_H_

namespace rum{

// we ditch kEnumName naming style
enum class SrvStatus{
    OK = 0,
    ServerErr,
    ClientErr,
    Timeout,
    NoConnections,
    Cancelled,
    Unknown
};

}

#endif //RUM_COMMON_SRV_DEF_H_
