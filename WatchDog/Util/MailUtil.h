#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <flatbuffers/flatbuffers.h>
#include <ServerLibrary/Network/Packet/Serialization/serialization.hpp>
#include "../Packet/mail_data_define_generated.h"
#include "../Packet/dump_transmit_generated.h"

using namespace SERVER::NETWORK::PACKET;
using namespace SERVER::NETWORK::PACKET::UTIL::SERIALIZATION;

static PACKET_STRUCT CreateMailRequestPacket(flatbuffers::FlatBufferBuilder& builder, const Mail::RequestType requestType = Mail::RequestType::RequestType_None, const std::string& sProgramName = "") {
	return Serialize<Mail::MailRequest>(builder, Mail::PacketType::PacketType_MailRequest, Mail::CreateMailRequest(builder, requestType, builder.CreateString(sProgramName)));
}

static PACKET_STRUCT CreateDumpTransmitPacket(flatbuffers::FlatBufferBuilder& builder, const Mail::RequestType requestType = Mail::RequestType::RequestType_None, const std::string& sProgramName = "", const std::string& sDumpFileName = "", const std::vector<uint8_t>& sDumpFileData = {}) {
	return Serialize<Mail::DumpFileTransmit>(builder, Mail::PacketType::PacketType_DumpFileTransmit, Mail::CreateDumpFileTransmit(builder, requestType, builder.CreateString(sProgramName), builder.CreateString(sDumpFileName), builder.CreateVector(sDumpFileData)));
}