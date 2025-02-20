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
#include <ServerLibrary/Functions/UUID/UUID.hpp>

CIOCP::CIOCP(const std::string& sProgramName) : IOCP(m_packetProcessor), m_hSharedMemory(NULL) {
	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_SignInRequest, std::bind(&CIOCP::SignInRequest, this, std::placeholders::_1));
	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_FindMatchRequest, std::bind(&CIOCP::FindMatchRequest, this, std::placeholders::_1));
	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_CreateSessionRequest, std::bind(&CIOCP::CreateSessionRequest, this, std::placeholders::_1));

	m_bMainThreadRunState = true;

	m_hSharedMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SERVER::FUNCTIONS::UTIL::MBToUni(sProgramName + " IPC").c_str());
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
	if (m_hSharedMemory != NULL)
		CloseHandle(m_hSharedMemory);

	IOCP::Destroy();
}

::IOCP::CONNECTION* CIOCP::OnIOTryDisconnect(User_Server* const pClient) {
	if (auto pConnection = ::IOCP::IOCP::OnIOTryDisconnect(pClient)) {
		m_userInfoMutex.lock();

		const auto& user = m_userInformation.find(pConnection);

		if (user != m_userInformation.cend()) {
			if (user->second->m_bIsInGame) {
				m_sessionInfoMutex.lock();

				const auto& session = m_sessionInformation.find(user->second->m_iSessionUniqueID);
				if (session != m_sessionInformation.cend()) {
					if (session->second->m_iCurrentUserCount <= 1) {
						delete session->second;
						m_sessionInformation.erase(session);
					}
					else
						session->second->m_iCurrentUserCount--;
				}

				m_sessionInfoMutex.unlock();

			}
			
			if (m_hSharedMemory != NULL) {
				if (int32_t* pSharedConnectedUserCount = (int32_t*)MapViewOfFile(m_hSharedMemory, FILE_MAP_WRITE, 0, 0, 0))
					*pSharedConnectedUserCount -= 1;
			}

			delete user->second;
			m_userInformation.erase(user);
		}

		m_userInfoMutex.unlock();
		return pConnection;
	}
	return nullptr;
}


void CIOCP::SignInRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {
	if (auto pConnection = reinterpret_cast<SERVER::NETWORKMODEL::IOCP::CONNECTION*>(pPacketData->m_pOwner)) {
		auto pFlatBuffer = new FFlatBuffer;
		uint16_t iUUID = 0;
		uint16_t iSignInResult = FlatPacket::RequestMessageType::RequestMessageType_Failed;

		if (const auto pSignInRequestData = FlatPacket::GetSignInRequest(pPacketData->m_packetData->m_sPacketData)) {
			iUUID = SERVER::FUNCTIONS::UUID::UUIDGenerator::Generate();

			m_userInfoMutex.lock();

			m_userInformation.insert(std::make_pair(pConnection, new FUserInformation(iUUID)));

			m_userInfoMutex.unlock();

			iSignInResult = FlatPacket::RequestMessageType::RequestMessageType_Succeeded;

			if (m_hSharedMemory != NULL) {
				if (int32_t* pSharedConnectedUserCount = (int32_t*)MapViewOfFile(m_hSharedMemory, FILE_MAP_WRITE, 0, 0, 0))
					*pSharedConnectedUserCount += 1;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

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

			m_sessionInfoMutex.lock();
			if (m_sessionInformation.size() > 0) {
				bool bFinded = false;
				int32_t iCachedSessionUniqueID = 0;
				std::string sCachedSessionID;
				
				for (auto& iterator : m_sessionInformation) {
					if (iterator.second->m_iCurrentUserCount < 10) {
						iterator.second->m_iCurrentUserCount++;

						sCachedSessionID = iterator.second->m_sSessionID;
						iCachedSessionUniqueID = iterator.first;
						bFinded = true;
						break;
					}
				}

				if (bFinded) {
					m_userInfoMutex.lock();

					const auto& user = m_userInformation.find(pConnection);
					if (user != m_userInformation.cend())
						user->second->JoinNewSession(iCachedSessionUniqueID);

					m_userInfoMutex.unlock();

					iFindMatchResult = FlatPacket::RequestMessageType::RequestMessageType_Succeeded | (FlatPacket::FindMatchResultType::FindMatchResultType_Succeeded << 8);
					sessiondIDOffset = pFlatBuffer->m_flatbuffer.CreateString(sCachedSessionID.c_str());
				}
			}
			m_sessionInfoMutex.unlock();

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
			int32_t iCachedSessionUniqueID = SERVER::FUNCTIONS::UUID::UUIDGenerator::Generate();

			m_sessionInfoMutex.lock();

			m_sessionInformation.insert(std::make_pair(iCachedSessionUniqueID, new FSessionInformation(pCreateSessionRequestData->session_id()->c_str())));

			m_sessionInfoMutex.unlock();

			m_userInfoMutex.lock();

			const auto& user = m_userInformation.find(pConnection);
			if (user != m_userInformation.cend())
				user->second->JoinNewSession(iCachedSessionUniqueID);

			m_userInfoMutex.unlock();
		}
	}
}
