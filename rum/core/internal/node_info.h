//
// Created by Ivan B on 2021/4/21.
//

#ifndef RUM_CORE_INTERNAL_NODE_INFO_H_
#define RUM_CORE_INTERNAL_NODE_INFO_H_

#include <rum/core/msg/rum_sync_generated.h>
#include <string>

namespace rum {

struct NodeInfo {
    int nid;
    int pid;

    std::string tcp_addr;
    std::string ipc_addr;
    std::string name;

    // todo ivan. the generation of uid can be much more efficient, especially for later lookup
    std::string uid;

    // essential for sync
    std::vector<std::string> sub_list;
    std::vector<std::string> server_list;
    // just for bookkeeping
    std::vector<std::string> pub_list;
    std::vector<std::string> client_list;

    NodeInfo(int nid, int pid, const std::string &tcp_addr, const std::string &ipc_addr);
};


}
#endif //RUM_CORE_INTERNAL_NODE_INFO_H_
