#include "CMatchingPacketProcessor.h"
#include "CMatchingIOCP.h"

using namespace SERVER::FUNCTIONS::LOG;

CMatchingPacketProcessor::CMatchingPacketProcessor(std::function<void(void*)>&& packetProcessedCallback, const std::string& sHostName, const std::string& sDBName, const std::string& sUserName, const std::string& sPassword, const uint16_t iMaxPoolConnection)
	: m_packetProcessedCallback(packetProcessedCallback)
	, m_bPacketProcessingThreadRunState(true) {

	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_SignInRequest, std::bind(&CMatchingPacketProcessor::LoginProcessing, this, std::placeholders::_1));

	m_pPacketProcessQueue = std::make_unique<PACKET_QUEUE>();
	m_pPacketStockQueue = std::make_unique<PACKET_QUEUE>();

	m_packetProcessingThread = std::thread(std::bind(&CMatchingPacketProcessor::PacketProcessingWorkerThread, this));
}

CMatchingPacketProcessor::~CMatchingPacketProcessor() {
	m_bPacketProcessingThreadRunState = false;
	if (m_packetProcessingThread.joinable())
		m_packetProcessingThread.join();
}

bool CMatchingPacketProcessor::Initialize(const std::string& sConfigFilePath) {


	return true;
}

void CMatchingPacketProcessor::AddNewPacketData(FPacketProcessingData* const pNewPacketData) {
	std::unique_lock<std::mutex> lck(m_csProcessingQueueMutex);
	m_pPacketStockQueue->Push(pNewPacketData);
}


void CMatchingPacketProcessor::PacketProcessingWorkerThread() {
	while (m_bPacketProcessingThreadRunState) {
		if (m_pPacketProcessQueue->IsEmpty() && !m_pPacketStockQueue->IsEmpty()) {
			m_csProcessingQueueMutex.lock();
			m_pPacketStockQueue.swap(m_pPacketProcessQueue);
			m_csProcessingQueueMutex.unlock();
		}

		FPacketProcessingData* pQueueData = nullptr;
		if (m_pPacketProcessQueue->Pop(pQueueData)) {
			if (auto pRet = PacketProcessing(pQueueData))
				m_packetProcessedCallback(pRet);

			delete pQueueData;
		}
	}
}

void* CMatchingPacketProcessor::PacketProcessing(FPacketProcessingData* pPacketData) {
	FTransmitQueueData* pTransmitQueueData = nullptr;
	if (pPacketData) {
		const auto& findResult = m_packetProcessor.find(static_cast<FlatPacket::PacketType>(pPacketData->m_packetType));
		if (findResult != m_packetProcessor.cend())
			pTransmitQueueData = findResult->second(pPacketData);
	}
	return pTransmitQueueData;
}


FTransmitQueueData* CMatchingPacketProcessor::LoginProcessing(const FPacketProcessingData* const pPacketData) {
	if (auto pPacketStruct = std::static_pointer_cast<PACKET_STRUCT, void>(pPacketData->m_pMessage)) {
	//	if (auto pLoginPacket = FlatPacket::LoginPacket::GetLoginPacket(pPacketStruct->m_sPacketData))
	//		m_databaseSystem.AddNewDBRequestData(new FDBLoginRequest(pPacketData->m_pRequestedUser, m_system.GenerateUUID(), pLoginPacket->address()->c_str()));
	}
	return nullptr;
}