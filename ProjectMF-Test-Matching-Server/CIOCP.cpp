#pragma comment(lib, "Rpcrt4.lib")
#include "CIOCP.h"
#include "data_define_generated.h"
#include "sign_in_request_define_generated.h"
#include "sign_in_result_define_generated.h"
#include "find_match_request_generated.h"
#include "find_match_result_generated.h"
#include "create_session_request_generated.h"
#include <rpc.h>
#include <ServerLibrary/Network/Packet/Serialization/serialization.hpp>


uint16_t GenerateUUID() {
	UUID userUUID;

	if (UuidCreate(&userUUID) != RPC_S_OK)
		return 0;

	RPC_STATUS result;
	auto iUUID = UuidHash(&userUUID, &result);

	if (result != RPC_S_OK)
		return 0;
	return iUUID;
}


CIOCP::CIOCP() : IOCP(m_packetProcessor) {
	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_SignInRequest, std::bind(&CIOCP::SignInRequest, this, std::placeholders::_1));
	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_FindMatchRequest, std::bind(&CIOCP::FindMatchRequest, this, std::placeholders::_1));
	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_CreateSessionRequest, std::bind(&CIOCP::CreateSessionRequest, this, std::placeholders::_1));
}

CIOCP::~CIOCP() {

}

bool CIOCP::Initialize(const EPROTOCOLTYPE protocolType, SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) {
	return IOCP::Initialize(protocolType, bindAddress);
}

void CIOCP::Run() {
	IOCP::Run();

}

void CIOCP::Destroy() {
	IOCP::Destroy();

}

void CIOCP::SignInRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {
	if (auto pConnection = reinterpret_cast<SERVER::NETWORKMODEL::IOCP::CONNECTION*>(pPacketData->m_pOwner)) {
		auto pFlatBuffer = new FFlatBuffer;
		uint16_t iUUID = 0;
		uint16_t iSignInResult = FlatPacket::RequestMessageType::RequestMessageType_Failed;
		bool bIsHost = false;

		if (const auto pSignInRequestData = FlatPacket::GetSignInRequest(pPacketData->m_packetData->m_sPacketData)) {
			iUUID = GenerateUUID();

			UUIDListMutex.lock();
			bIsHost = m_userUUIDList.size() <= 0;
			m_userUUIDList.push_back(iUUID);
			UUIDListMutex.unlock();

			iSignInResult = FlatPacket::RequestMessageType::RequestMessageType_Succeeded;
		}
		Sleep(2000);
		pConnection->m_pUser->Send(
			SERVER::NETWORK::PACKET::UTIL::SERIALIZATION::Serialize(
				pFlatBuffer->m_flatbuffer, 
				FlatPacket::PacketType::PacketType_SignInResult, 
				FlatPacket::CreateSignInResult(pFlatBuffer->m_flatbuffer, iSignInResult, iUUID)));
	}
}

void CIOCP::FindMatchRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {
	if (auto pConnection = reinterpret_cast<SERVER::NETWORKMODEL::IOCP::CONNECTION*>(pPacketData->m_pOwner)) {
		if (const auto pFindMatchRequestData = FlatPacket::GetFindMatchRequest(pPacketData->m_packetData->m_sPacketData)) {
			auto pFlatBuffer = new FFlatBuffer;
			uint16_t iFindMatchResult = FlatPacket::RequestMessageType::RequestMessageType_Succeeded | (FlatPacket::FindMatchResultType::FindMatchResultType_Failed << 8);
			flatbuffers::Offset<flatbuffers::String> sessiondIDOffset = 0;

			if (m_sessionIDList.size() > 0) {
				SessionIDListMutex.lock();
				bool bFinded = false;
				std::string sCachedSessionID;
				for (auto& iterator : m_sessionIDList) {
					if (iterator.second < 10) {
						sCachedSessionID = iterator.first;
						bFinded = true;
						break;
					}
				}
				SessionIDListMutex.unlock();

				if (bFinded) {
					iFindMatchResult = FlatPacket::RequestMessageType::RequestMessageType_Succeeded | (FlatPacket::FindMatchResultType::FindMatchResultType_Succeeded << 8);
					sessiondIDOffset = pFlatBuffer->m_flatbuffer.CreateString(sCachedSessionID.c_str());
				}
			}

			pConnection->m_pUser->Send(
				SERVER::NETWORK::PACKET::UTIL::SERIALIZATION::Serialize(
					pFlatBuffer->m_flatbuffer,
					FlatPacket::PacketType::PacketType_FindMatchResult,
					FlatPacket::CreateFindMatchResult(pFlatBuffer->m_flatbuffer, iFindMatchResult, pFindMatchRequestData->uuid(), sessiondIDOffset)));
		}
	}
}

void CIOCP::CreateSessionRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {
	if (auto pConnection = reinterpret_cast<SERVER::NETWORKMODEL::IOCP::CONNECTION*>(pPacketData->m_pOwner)) {
		if (const auto pCreateSessionRequestData = FlatPacket::GetCreateSessionRequest(pPacketData->m_packetData->m_sPacketData)) {
			SessionIDListMutex.lock();

			m_sessionIDList.push_back(std::make_pair(pCreateSessionRequestData->session_id()->c_str(), 1));

			SessionIDListMutex.unlock();
		}
	}
}
