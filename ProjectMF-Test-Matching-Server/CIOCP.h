#pragma once
#define NOMINMAX
#include <ServerLibrary/NetworkModel/IOCP/IOCP.hpp>
#include <flatbuffers/flatbuffers.h>

using namespace SERVER::NETWORKMODEL;

struct FFlatBuffer : SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<FFlatBuffer> {
public:
	flatbuffers::FlatBufferBuilder m_flatbuffer;

public:
	FFlatBuffer() : m_flatbuffer(SERVER::NETWORK::PACKET::PACKET_STRUCT::BUFFER_LENGTH) {};

};

class CIOCP : public IOCP::IOCP {
public:
	CIOCP();
	virtual ~CIOCP() override final;

public:
	virtual bool Initialize(const EPROTOCOLTYPE protocolType, SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) override final;
	virtual void Run() override final;
	virtual void Destroy() override final;

private:
	void SignInRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);
	void FindMatchRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);
	void CreateSessionRequest(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);

private:
	BASEMODEL::PACKETPROCESSOR m_packetProcessor;

	std::mutex UUIDListMutex;
	std::vector<int32_t> m_userUUIDList;

	std::mutex SessionIDListMutex;
	std::vector<std::pair<std::string, short>> m_sessionIDList;

};

