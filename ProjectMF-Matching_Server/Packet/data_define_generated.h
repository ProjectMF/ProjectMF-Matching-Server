// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DATADEFINE_FLATPACKET_H_
#define FLATBUFFERS_GENERATED_DATADEFINE_FLATPACKET_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace FlatPacket {

enum PacketType : int8_t {
  PacketType_None = 0,
  PacketType_SignInRequest = 1,
  PacketType_SignInResult = 2,
  PacketType_UserAccountInfoRequest = 3,
  PacketType_UserAccountInfoResult = 4,
  PacketType_MIN = PacketType_None,
  PacketType_MAX = PacketType_UserAccountInfoResult
};

inline const PacketType (&EnumValuesPacketType())[5] {
  static const PacketType values[] = {
    PacketType_None,
    PacketType_SignInRequest,
    PacketType_SignInResult,
    PacketType_UserAccountInfoRequest,
    PacketType_UserAccountInfoResult
  };
  return values;
}

inline const char * const *EnumNamesPacketType() {
  static const char * const names[6] = {
    "None",
    "SignInRequest",
    "SignInResult",
    "UserAccountInfoRequest",
    "UserAccountInfoResult",
    nullptr
  };
  return names;
}

inline const char *EnumNamePacketType(PacketType e) {
  if (::flatbuffers::IsOutRange(e, PacketType_None, PacketType_UserAccountInfoResult)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPacketType()[index];
}

enum RequestMessageType : int8_t {
  RequestMessageType_None = 0,
  RequestMessageType_Succeeded = 1,
  RequestMessageType_Failed = 2,
  RequestMessageType_MIN = RequestMessageType_None,
  RequestMessageType_MAX = RequestMessageType_Failed
};

inline const RequestMessageType (&EnumValuesRequestMessageType())[3] {
  static const RequestMessageType values[] = {
    RequestMessageType_None,
    RequestMessageType_Succeeded,
    RequestMessageType_Failed
  };
  return values;
}

inline const char * const *EnumNamesRequestMessageType() {
  static const char * const names[4] = {
    "None",
    "Succeeded",
    "Failed",
    nullptr
  };
  return names;
}

inline const char *EnumNameRequestMessageType(RequestMessageType e) {
  if (::flatbuffers::IsOutRange(e, RequestMessageType_None, RequestMessageType_Failed)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesRequestMessageType()[index];
}

}  // namespace FlatPacket

#endif  // FLATBUFFERS_GENERATED_DATADEFINE_FLATPACKET_H_
