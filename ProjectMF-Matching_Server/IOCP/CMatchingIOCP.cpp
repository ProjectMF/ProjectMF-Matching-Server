#include "CMatchingIOCP.h"

CMatchingIOCP::CMatchingIOCP() 
	: IOCP(m_packetProcessor, std::thread::hardware_concurrency())
	, m_packetProcessorInstance(std::bind(&CMatchingIOCP::PacketProcessedCallbackFromPacketProcessor, this, std::placeholders::_1), "34.47.74.118", "GameDB", "sqlserver", "1234", (std::thread::hardware_concurrency() / 2))
	, m_bTransmissionThreadRunState(true)
	, m_bIOCPRunState(true) {

	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_SignInRequest, std::bind(&CMatchingIOCP::LoginRequest, this, std::placeholders::_1));

	m_pPacketTransmissionStockQueue = std::make_unique<TRANSMISSION_QUEUE>();
	m_pPacketTransmissionProcessQueue = std::make_unique<TRANSMISSION_QUEUE>();

	m_transmissionThread = std::thread(std::bind(&CMatchingIOCP::PacketTransmission, this));
}

CMatchingIOCP::~CMatchingIOCP() {
}

bool CMatchingIOCP::Initialize(const EPROTOCOLTYPE protocolType, SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) {
	if (IOCP::Initialize(protocolType, bindAddress)) {


		return m_packetProcessorInstance.Initialize("");
	}
	return false;
}

void CMatchingIOCP::Run() {
	IOCP::Run();

}

void CMatchingIOCP::Destroy() {
	IOCP::Destroy();

	m_bTransmissionThreadRunState = false;
	if (m_transmissionThread.joinable())
		m_transmissionThread.join();
}


void CMatchingIOCP::PacketTransmission() {
	while (m_bTransmissionThreadRunState) {
		if (!m_bTransmissionThreadRunState && m_pPacketTransmissionProcessQueue->IsEmpty())
			break;

		if (m_pPacketTransmissionProcessQueue->IsEmpty() && !m_pPacketTransmissionStockQueue->IsEmpty()) {
			std::unique_lock<std::mutex> lck(m_csTransmitQueueMutex);
			m_pPacketTransmissionStockQueue.swap(m_pPacketTransmissionProcessQueue);
		}

		FTransmitQueueData* pQueueData = nullptr;
		if (m_pPacketTransmissionProcessQueue->Pop(pQueueData)) {
			pQueueData->m_pClientConnection->m_pUser->Send(pQueueData->m_packetStructure);

			delete pQueueData;
		}
	}
}

void CMatchingIOCP::PacketProcessedCallbackFromPacketProcessor(void* pPacketPrcessingResult) {
	if (auto pNewTransmitData = reinterpret_cast<FTransmitQueueData*>(pPacketPrcessingResult)) {
		std::unique_lock<std::mutex> lock(m_csTransmitQueueMutex);
		m_pPacketTransmissionStockQueue->Push(pNewTransmitData);
	}
}

void CMatchingIOCP::LoginRequest(PacketQueueData* const pPacketData) {
	if (auto pConnection = reinterpret_cast<SERVER::NETWORKMODEL::IOCP::CONNECTION*>(pPacketData->m_pOwner)) {
		m_packetProcessorInstance.AddNewPacketData(new FPacketProcessingData(pConnection, FlatPacket::PacketType::PacketType_SignInRequest, pPacketData->m_packetData));
	}
}
