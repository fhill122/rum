/*
 * Created by Ivan B on 2022/1/9.
 */

#ifndef RUM_COMMON_NODE_PARAM_H_
#define RUM_COMMON_NODE_PARAM_H_


namespace rum{

struct NodeParam{
    // todo ivan.
    //  - option to start master?
    //  - option to set name

    bool enable_ipc_txrx = true;
    // Note: despite this setting, sync is always over tcp.
    // And tcp address is always there unlike ipc to id the node,
    // because of this, this only affect pubs' connection in this node
    bool enable_tcp_tx = true;

    inline NodeParam() {};

    inline bool check(){
        // no ipc communication
        if (!enable_tcp_tx && !enable_ipc_txrx) return false;
        return true;
    };
};


}

#endif //RUM_COMMON_NODE_PARAM_H_
