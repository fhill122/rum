//
// Created by Ivan B on 2021/4/21.
//

#include "node_info.h"

using namespace std;
namespace rum {

NodeInfo::NodeInfo(int nid, int pid, const std::string &tcp_addr, const std::string &ipc_addr)
        : nid(nid), pid(pid), tcp_addr(tcp_addr), ipc_addr(ipc_addr) {
    uid = to_string(pid) + "_" + to_string(nid) + "_" + tcp_addr;
}



}