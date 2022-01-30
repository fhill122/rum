/*
 * Created by Ivan B on 2022/1/26.
 */

#ifndef RUM_CORE_INTERNAL_NODEID_H_
#define RUM_CORE_INTERNAL_NODEID_H_

#include <string>

namespace rum{

    static std::string GetNodeStrId (int pid, const std::string &tcp_addr){
        return std::to_string(pid) + "@" + tcp_addr;
    }
}

#endif //RUM_CORE_INTERNAL_NODEID_H_
