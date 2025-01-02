#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include "Packet/data_define_generated.h"
#include <NetworkModel/IOCP/IOCP.hpp>
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <flatbuffers/flatbuffers.h>
#include <functional>

using namespace SERVER::NETWORK::PACKET;

struct FPacketProcessingData : public SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<FPacketProcessingData> {
public:
	SERVER::NETWORKMODEL::IOCP::CONNECTION* const m_pRequestedUser;
	uint8_t m_packetType;

	std::shared_ptr<void> m_pMessage;

public:
	FPacketProcessingData() : m_pRequestedUser(), m_pMessage(), m_packetType() {};
	FPacketProcessingData(SERVER::NETWORKMODEL::IOCP::CONNECTION* const pRequestedUser, const uint8_t packetType, std::shared_ptr<void> pMessage) : m_pRequestedUser(pRequestedUser), m_packetType(packetType), m_pMessage(pMessage) {};

};

struct FFlatBuffer : SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<FFlatBuffer> {
public:
	flatbuffers::FlatBufferBuilder m_flatbuffer;

public:
	FFlatBuffer() : m_flatbuffer(SERVER::NETWORK::PACKET::PACKET_STRUCT::BUFFER_LENGTH) {};

};

struct FTransmitQueueData;

class CMatchingPacketProcessor {
	typedef SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<FPacketProcessingData*> PACKET_QUEUE;
	typedef std::unordered_map<FlatPacket::PacketType, std::function<FTransmitQueueData* (const FPacketProcessingData* const)>> PROCESSOR;

	const std::function<void(void*)> m_packetProcessedCallback;
private:


private:
	std::mutex m_csProcessingQueueMutex;
	std::condition_variable m_cvProcessingThread;
	std::unique_ptr<PACKET_QUEUE> m_pPacketProcessQueue;
	std::unique_ptr<PACKET_QUEUE> m_pPacketStockQueue;

	std::atomic_bool m_bPacketProcessingThreadRunState;
	std::thread m_packetProcessingThread;

	PROCESSOR m_packetProcessor;

private:
	void PacketProcessingWorkerThread();
	void* PacketProcessing(FPacketProcessingData* pPacketData);

private:
	FTransmitQueueData* LoginProcessing(const FPacketProcessingData* const pPacketData);

public:
	CMatchingPacketProcessor(std::function<void(void*)>&& packetProcessedCallback, const std::string& sHostName, const std::string& sDBName, const std::string& sUserName, const std::string& sPassword, const uint16_t iMaxPoolConnection);
	~CMatchingPacketProcessor();

	bool Initialize(const std::string& sConfigFilePath);

public:
	void AddNewPacketData(FPacketProcessingData* const pNewPacketData);

};
