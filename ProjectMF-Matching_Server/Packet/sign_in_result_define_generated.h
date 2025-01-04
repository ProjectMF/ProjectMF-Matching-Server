// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SIGNINRESULTDEFINE_FLATPACKET_H_
#define FLATBUFFERS_GENERATED_SIGNINRESULTDEFINE_FLATPACKET_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace FlatPacket {

struct SignInResult;
struct SignInResultBuilder;

struct SignInResult FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SignInResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MESSAGE_TYPE = 4,
    VT_UUID = 6
  };
  int16_t message_type() const {
    return GetField<int16_t>(VT_MESSAGE_TYPE, 0);
  }
  int32_t uuid() const {
    return GetField<int32_t>(VT_UUID, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int16_t>(verifier, VT_MESSAGE_TYPE, 2) &&
           VerifyField<int32_t>(verifier, VT_UUID, 4) &&
           verifier.EndTable();
  }
};

struct SignInResultBuilder {
  typedef SignInResult Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_message_type(int16_t message_type) {
    fbb_.AddElement<int16_t>(SignInResult::VT_MESSAGE_TYPE, message_type, 0);
  }
  void add_uuid(int32_t uuid) {
    fbb_.AddElement<int32_t>(SignInResult::VT_UUID, uuid, 0);
  }
  explicit SignInResultBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SignInResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SignInResult>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<SignInResult> CreateSignInResult(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int16_t message_type = 0,
    int32_t uuid = 0) {
  SignInResultBuilder builder_(_fbb);
  builder_.add_uuid(uuid);
  builder_.add_message_type(message_type);
  return builder_.Finish();
}

inline const FlatPacket::SignInResult *GetSignInResult(const void *buf) {
  return ::flatbuffers::GetRoot<FlatPacket::SignInResult>(buf);
}

inline const FlatPacket::SignInResult *GetSizePrefixedSignInResult(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<FlatPacket::SignInResult>(buf);
}

inline bool VerifySignInResultBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<FlatPacket::SignInResult>(nullptr);
}

inline bool VerifySizePrefixedSignInResultBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<FlatPacket::SignInResult>(nullptr);
}

inline void FinishSignInResultBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<FlatPacket::SignInResult> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSignInResultBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<FlatPacket::SignInResult> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace FlatPacket

#endif  // FLATBUFFERS_GENERATED_SIGNINRESULTDEFINE_FLATPACKET_H_