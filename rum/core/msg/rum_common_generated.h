// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RUMCOMMON_RUM_MSG_H_
#define FLATBUFFERS_GENERATED_RUMCOMMON_RUM_MSG_H_

#include "flatbuffers/flatbuffers.h"

namespace rum {
namespace msg {

struct Ip;

struct TcpAddress;

struct CompactNodeId;

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(1) Ip FLATBUFFERS_FINAL_CLASS {
 private:
  uint8_t ip0_;
  uint8_t ip1_;
  uint8_t ip2_;
  uint8_t ip3_;

 public:
  Ip() {
    memset(static_cast<void *>(this), 0, sizeof(Ip));
  }
  Ip(uint8_t _ip0, uint8_t _ip1, uint8_t _ip2, uint8_t _ip3)
      : ip0_(flatbuffers::EndianScalar(_ip0)),
        ip1_(flatbuffers::EndianScalar(_ip1)),
        ip2_(flatbuffers::EndianScalar(_ip2)),
        ip3_(flatbuffers::EndianScalar(_ip3)) {
  }
  uint8_t ip0() const {
    return flatbuffers::EndianScalar(ip0_);
  }
  uint8_t ip1() const {
    return flatbuffers::EndianScalar(ip1_);
  }
  uint8_t ip2() const {
    return flatbuffers::EndianScalar(ip2_);
  }
  uint8_t ip3() const {
    return flatbuffers::EndianScalar(ip3_);
  }
};
FLATBUFFERS_STRUCT_END(Ip, 4);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) TcpAddress FLATBUFFERS_FINAL_CLASS {
 private:
  rum::msg::Ip ip_;
  int32_t port_;

 public:
  TcpAddress() {
    memset(static_cast<void *>(this), 0, sizeof(TcpAddress));
  }
  TcpAddress(const rum::msg::Ip &_ip, int32_t _port)
      : ip_(_ip),
        port_(flatbuffers::EndianScalar(_port)) {
  }
  const rum::msg::Ip &ip() const {
    return ip_;
  }
  int32_t port() const {
    return flatbuffers::EndianScalar(port_);
  }
};
FLATBUFFERS_STRUCT_END(TcpAddress, 8);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) CompactNodeId FLATBUFFERS_FINAL_CLASS {
 private:
  rum::msg::TcpAddress tcp_address_;
  int32_t pid_;

 public:
  CompactNodeId() {
    memset(static_cast<void *>(this), 0, sizeof(CompactNodeId));
  }
  CompactNodeId(const rum::msg::TcpAddress &_tcp_address, int32_t _pid)
      : tcp_address_(_tcp_address),
        pid_(flatbuffers::EndianScalar(_pid)) {
  }
  const rum::msg::TcpAddress &tcp_address() const {
    return tcp_address_;
  }
  int32_t pid() const {
    return flatbuffers::EndianScalar(pid_);
  }
};
FLATBUFFERS_STRUCT_END(CompactNodeId, 12);

}  // namespace msg
}  // namespace rum

#endif  // FLATBUFFERS_GENERATED_RUMCOMMON_RUM_MSG_H_