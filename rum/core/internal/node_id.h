/*
 * Created by Ivan B on 2022/1/26.
 */

#ifndef RUM_CORE_INTERNAL_NODEID_H_
#define RUM_CORE_INTERNAL_NODEID_H_

#include <string>
#include "rum/common/zmq_helper.h"
#include "rum/core/msg/rum_common_generated.h"

namespace rum{

    struct CompactNodeId{
        msg::CompactNodeId fbs;

        CompactNodeId(const msg::CompactNodeId _fbs) : fbs(_fbs){}

        static inline std::string ToTcpStr(const msg::TcpAddress &fbs){
            std::string out;
            out.reserve(27);
            out.append("tcp://");
            out.append(std::to_string(fbs.ip().ip0()));
            out.append(".");
            out.append(std::to_string(fbs.ip().ip1()));
            out.append(".");
            out.append(std::to_string(fbs.ip().ip2()));
            out.append(".");
            out.append(std::to_string(fbs.ip().ip3()));
            out.append(":");
            out.append(std::to_string(fbs.port()));
            return out;
        }
    };

    static inline std::string GetNodeStrId (int pid, std::string_view tcp_addr){
        std::string out;

        // short
        // tcp_addr.remove_prefix(6);  // remove "tcp://"
        // int endian_pid = flatbuffers::EndianScalar(pid);
        // out.reserve(sizeof(endian_pid) + tcp_addr.size());
        // out.append((const char*)(&endian_pid), sizeof(endian_pid));
        // out.append(tcp_addr);
        // return out;

        // human readable
        tcp_addr.remove_prefix(6);  // remove "tcp://"
        out.reserve(7+tcp_addr.size());
        out.append(std:: to_string(pid));
        out.append("@");
        out.append(tcp_addr);
        return out;
    }
}

#endif //RUM_CORE_INTERNAL_NODEID_H_
