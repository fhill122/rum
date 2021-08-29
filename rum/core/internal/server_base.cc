//
// Created by Ivan B on 2021/3/15.
//

#include "server_base.h"

#include <utility>

using namespace std;

namespace rum {

// auto tryBind= [this, owner](zmq::socket_t *socket){
//     const int MAX_TRIAL = 10;
//     bool ok = false;
//     string tcp_addr;
//     for (int i = 0; i < MAX_TRIAL; ++I) {
//         try{
//             // tcp_addr = genTcpAddr();
//             tcp_addr = owner->genIpcAddr();
//             socket ->bindTcpRaw(tcp_addr);
//             ok = true;
//             if (I>0){
//                 ol.i(TAG, "retry success. bindTcpRaw to " + tcp_addr);
//             }
//             break;
//         }
//         catch (...){
//             ol.i(TAG, tcp_addr +" binding failed, retry");
//         }
//     }
//     assertLog(ok,"failed to find an available tcp address");
//     return tcp_addr;
// };

ServerBase::ServerBase(string name, const shared_ptr<zmq::context_t> &context)
        : name_(std::move(name)), context_(context) {

}

}
