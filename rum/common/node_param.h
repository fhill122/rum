/*
 * Created by Ivan B on 2022/1/9.
 */

#ifndef RUM_COMMON_NODE_PARAM_H_
#define RUM_COMMON_NODE_PARAM_H_

namespace rum{

struct NodeParam{
    bool enable_ipc_socket = true;
    bool enable_tcp_socket = true;  // note: despite this setting, sync is always over tcp
    inline NodeParam() {};

    inline bool check(){
        // no ipc communication
        if (!enable_tcp_socket && !enable_ipc_socket) return false;
        return true;
    };
};


}

#endif //RUM_COMMON_NODE_PARAM_H_
