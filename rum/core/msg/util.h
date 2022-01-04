/*
 * Created by Ivan B on 2021/12/31.
 */

#ifndef RUM_CORE_MSG_UTIL_H_
#define RUM_CORE_MSG_UTIL_H_

#include "rum_sync_generated.h"

using namespace std;

namespace rum{

inline std::string ToString(const msg::NodeId *node){
    return "node(" + node->tcp_addr()->str() + ":" + std::to_string(node->pid())+")";
}

inline std::string ToString(const msg::SyncBroadcast *sync){
    string str = ToString(sync->node()) + ", " +
            "version " + to_string(sync->version()) + ", " +
            to_string(sync->subscribers()->size()) +
            " sub(s)";

    if (sync->subscribers()->size()>0){
        str += ": ";
        for (const auto*sub : *sync->subscribers()){
            str += sub->topic()->str() + " ";
        }
    }

    return str;
}

}

#endif //RUM_CORE_MSG_UTIL_H_
