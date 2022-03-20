/*
 * Created by Ivan B on 2021/12/31.
 */

#ifndef RUM_CORE_MSG_UTIL_H_
#define RUM_CORE_MSG_UTIL_H_

#include "rum_sync_generated.h"

using namespace std;

namespace rum{

inline std::string ToString(const msg::NodeId *node){
    std::string out = "node(";
    out += std::to_string(node->pid());
    out += "@";
    out += node->tcp_addr()->str();
    out += ")";
    return out;
}

inline std::string ToString(const msg::SyncBroadcast *sync){
    using namespace std;
    string str = ToString(sync->node());
    str += ", v";
    str += to_string(sync->version());
    str += ", ";

    str += to_string(sync->subscribers()->size());
    str += " sub(s)";
    if (sync->subscribers()->size()>0){
        str += ": ";
        for (const auto*sub : *sync->subscribers()){
            str += sub->topic()->str() + " ";
        }
    }

    str += ", ";
    str += to_string(sync->clients()->size());
    str += " cli(s)";
    if (sync->clients()->size()>0){
        str += ": ";
        for (const auto*cli : *sync->clients()){
            str += cli->topic()->str() + " ";
        }
    }

    return str;
}

}

#endif //RUM_CORE_MSG_UTIL_H_
