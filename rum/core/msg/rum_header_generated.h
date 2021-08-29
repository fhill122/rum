// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RUMHEADER_RUM_MSG_H_
#define FLATBUFFERS_GENERATED_RUMHEADER_RUM_MSG_H_

#include "flatbuffers/flatbuffers.h"

namespace rum {
namespace msg {

struct MsgHeader;
struct MsgHeaderBuilder;

enum MsgType {
  MsgType_Interrupt = 0,
  MsgType_Message = 1,
  MsgType_ServiceCall = 2,
  MsgType_MIN = MsgType_Interrupt,
  MsgType_MAX = MsgType_ServiceCall
};

inline const MsgType (&EnumValuesMsgType())[3] {
  static const MsgType values[] = {
    MsgType_Interrupt,
    MsgType_Message,
    MsgType_ServiceCall
  };
  return values;
}

inline const char * const *EnumNamesMsgType() {
  static const char * const names[4] = {
    "Interrupt",
    "Message",
    "ServiceCall",
    nullptr
  };
  return names;
}

inline const char *EnumNameMsgType(MsgType e) {
  if (flatbuffers::IsOutRange(e, MsgType_Interrupt, MsgType_ServiceCall)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMsgType()[index];
}

struct MsgHeader FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MsgHeaderBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TYPE = 4,
    VT_NAME = 6,
    VT_PROTOCAL = 8
  };
  rum::msg::MsgType type() const {
    return static_cast<rum::msg::MsgType>(GetField<int8_t>(VT_TYPE, 0));
  }
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::String *protocal() const {
    return GetPointer<const flatbuffers::String *>(VT_PROTOCAL);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_TYPE) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_PROTOCAL) &&
           verifier.VerifyString(protocal()) &&
           verifier.EndTable();
  }
};

struct MsgHeaderBuilder {
  typedef MsgHeader Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(rum::msg::MsgType type) {
    fbb_.AddElement<int8_t>(MsgHeader::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(MsgHeader::VT_NAME, name);
  }
  void add_protocal(flatbuffers::Offset<flatbuffers::String> protocal) {
    fbb_.AddOffset(MsgHeader::VT_PROTOCAL, protocal);
  }
  explicit MsgHeaderBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  MsgHeaderBuilder &operator=(const MsgHeaderBuilder &);
  flatbuffers::Offset<MsgHeader> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<MsgHeader>(end);
    return o;
  }
};

inline flatbuffers::Offset<MsgHeader> CreateMsgHeader(
    flatbuffers::FlatBufferBuilder &_fbb,
    rum::msg::MsgType type = rum::msg::MsgType_Interrupt,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::String> protocal = 0) {
  MsgHeaderBuilder builder_(_fbb);
  builder_.add_protocal(protocal);
  builder_.add_name(name);
  builder_.add_type(type);
  return builder_.Finish();
}

inline flatbuffers::Offset<MsgHeader> CreateMsgHeaderDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    rum::msg::MsgType type = rum::msg::MsgType_Interrupt,
    const char *name = nullptr,
    const char *protocal = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto protocal__ = protocal ? _fbb.CreateString(protocal) : 0;
  return rum::msg::CreateMsgHeader(
      _fbb,
      type,
      name__,
      protocal__);
}

inline const rum::msg::MsgHeader *GetMsgHeader(const void *buf) {
  return flatbuffers::GetRoot<rum::msg::MsgHeader>(buf);
}

inline const rum::msg::MsgHeader *GetSizePrefixedMsgHeader(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<rum::msg::MsgHeader>(buf);
}

inline bool VerifyMsgHeaderBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<rum::msg::MsgHeader>(nullptr);
}

inline bool VerifySizePrefixedMsgHeaderBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<rum::msg::MsgHeader>(nullptr);
}

inline void FinishMsgHeaderBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<rum::msg::MsgHeader> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMsgHeaderBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<rum::msg::MsgHeader> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace msg
}  // namespace rum

#endif  // FLATBUFFERS_GENERATED_RUMHEADER_RUM_MSG_H_
