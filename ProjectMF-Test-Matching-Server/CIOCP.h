#pragma once
#define NOMINMAX
#include <ServerLibrary/NetworkModel/IOCP/IOCP.hpp>
#include <flatbuffers/flatbuffers.h>
#include <unordered_map>

using namespace SERVER::NETWORKMODEL;

struct FFlatBuffer : SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<FFlatBuffer> {
public:
	flatbuffers::FlatBufferBuilder m_flatbuffer;

public:
	FFlatBuffer() : m_flatbuffer(SERVER::NETWORK::PACKET::PACKET_STRUCT::BUFFER_LENGTH) {};

};

struct FUserInformation : SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<FUserInformation> {
public:
	int32_t m_iUUID;

	bool m_bIsInGame;
	int32_t m_iSessionUniqueID;

public:
	FUserInformation(const int32_t iUUID) : m_iUUID(iUUID), m_bIsInGame(), m_iSessionUniqueID() {}

	void JoinNewSession(const int32_t iSessionUniqueID) {
		m_iSessionUniqueID = iSessionUniqueID;
		m_bIsInGame = true;
	}

};

struct FSessionInformation : SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<FSessionInformation> {
public:
	std::string m_sSessionID;
	int32_t m_iCurrentUserCount;

public:
	FSessionInformation() : m_sSessionID(), m_iCurrentUserCount(0) {};
	FSessionInformation(const std::string& sSessionID) : m_sSessionID(sSessionID), m_iCurrentUserCount(1) {};

};

class CIOCP : public IOCP::IOCP {
public:
	CIOCP();
	virtual ~CIOCP() override final;

public:
	virtual bool Initialize(const EPROTOCOLTYPE protocolType, SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) override final;
	virtual void Run() override final;
	virtual void Destroy() override final;

protected:
	virtual ::IOCP::CONNECTION* OnIOTryDisconnect(User_Server* const pClient) override final;

private:
	void SignInRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);
	void FindMatchRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);
	void CreateSessionRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);

private:
	BASEMODEL::PACKETPROCESSOR m_packetProcessor;

	std::mutex m_userInfoMutex;
	std::unordered_map<::IOCP::CONNECTION*, FUserInformation*> m_userInformation;

	std::mutex m_sessionInfoMutex;
	std::unordered_map<int32_t, FSessionInformation*> m_sessionInformation;

};

