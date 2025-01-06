#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include "../PacketProcessor/CMatchingPacketProcessor.h"
#include <NetworkModel/IOCP/IOCP.hpp>
#include <thread>
#include <atomic>
#include <mutex>

using namespace SERVER::NETWORKMODEL;
using namespace SERVER::NETWORKMODEL::BASEMODEL;
using namespace SERVER::NETWORKMODEL::IOCP;
using namespace SERVER::NETWORK::PACKET;

struct FTransmitQueueData : public SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<FTransmitQueueData, 400> {
public:
	SERVER::NETWORKMODEL::IOCP::CONNECTION* m_pClientConnection;
	PACKET_STRUCT m_packetStructure;

public:
	FTransmitQueueData() : m_pClientConnection(nullptr), m_packetStructure() {};
	FTransmitQueueData(SERVER::NETWORKMODEL::IOCP::CONNECTION* pConnectedUser) : m_pClientConnection(pConnectedUser), m_packetStructure() {};
	FTransmitQueueData(const PACKET_STRUCT& packetStructure) : m_packetStructure(packetStructure), m_pClientConnection() {};
	FTransmitQueueData(SERVER::NETWORKMODEL::IOCP::CONNECTION* pConnectedUser, const PACKET_STRUCT& packetStructure) : m_pClientConnection(pConnectedUser), m_packetStructure(packetStructure) {};

};

class CMatchingIOCP : SERVER::NETWORKMODEL::IOCP::IOCP {
	typedef SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<FTransmitQueueData*> TRANSMISSION_QUEUE;
private:
	PACKETPROCESSOR m_packetProcessor;

	CMatchingPacketProcessor m_packetProcessorInstance;

	std::mutex m_csTransmitQueueMutex;
	std::unique_ptr<TRANSMISSION_QUEUE> m_pPacketTransmissionProcessQueue;
	std::unique_ptr<TRANSMISSION_QUEUE> m_pPacketTransmissionStockQueue;

	std::atomic_bool m_bTransmissionThreadRunState;
	std::thread m_transmissionThread;

	std::atomic_bool m_bIOCPRunState;

private:
	void PacketTransmission();
	void PacketProcessedCallbackFromPacketProcessor(void* pPacketPrcessingResult);

	void SignInRequest(PacketQueueData* const pPacketData);

public:
	CMatchingIOCP();
	virtual ~CMatchingIOCP() override final;

public:
	virtual bool Initialize(const EPROTOCOLTYPE protocolType, SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) override final;
	virtual void Run() override final;
	virtual void Destroy() override final;

public:
	__forceinline bool IOCPRunState() const { return m_bIOCPRunState; }

};