#include "CMatchingPacketProcessor.h"
#include "../Packet/Util/Util.h"
#include "../IOCP/CMatchingIOCP.h"

using namespace SERVER::FUNCTIONS::LOG;

CMatchingPacketProcessor::CMatchingPacketProcessor(std::function<void(void*)>&& packetProcessedCallback, const std::string& sHostName, const std::string& sDBName, const std::string& sUserName, const std::string& sPassword, const uint16_t iMaxPoolConnection)
	: m_databaseSystem("", "", "", "", iMaxPoolConnection, std::bind(&CMatchingPacketProcessor::AddNewPacketData, this, std::placeholders::_1))
	, m_packetProcessedCallback(packetProcessedCallback)
	, m_bPacketProcessingThreadRunState(true) {

	m_packetProcessor.emplace(FlatPacket::PacketType::PacketType_SignInRequest, std::bind(&CMatchingPacketProcessor::SignInProcessing, this, std::placeholders::_1));


	m_packetProcessor.emplace(EDBRequestType::EDBType_SignInRequest, std::bind(&CMatchingPacketProcessor::DBSignInProcessed, this, std::placeholders::_1));

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

FTransmitQueueData* CMatchingPacketProcessor::SignInProcessing(const FPacketProcessingData* const pPacketData) {
	if (auto pPacketStruct = std::static_pointer_cast<PACKET_STRUCT, void>(pPacketData->m_pMessage)) {
		if (auto pLoginPacket = FlatPacket::GetSignInRequest(pPacketStruct->m_sPacketData)) {
			auto flatbuffer = std::make_unique<FFlatBuffer>();

			m_databaseSystem.AddNewDBRequestData(new FDBSignInRequest(pPacketData->m_pRequestedUser, m_system.GenerateUUID()));
		}
	}
	return nullptr;
}

// DB Result

FTransmitQueueData* CMatchingPacketProcessor::DBSignInProcessed(const FPacketProcessingData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;
	using namespace FlatPacket;

	FTransmitQueueData* pNewTransmitQueueData = nullptr;

	if (auto pSignInDBResult = std::static_pointer_cast<FDBSignInRequest, void>(pPacketData->m_pMessage)) {
		auto flatBuffer = std::make_unique<FFlatBuffer>();
		RequestMessageType iRequestResult = RequestMessageType::RequestMessageType_Failed;

		if (pSignInDBResult->m_requestResult == RequestMessageType::RequestMessageType_Succeeded) {
			iRequestResult = RequestMessageType::RequestMessageType_Succeeded;

			Log::WriteLog(L"Client [%d] Login Request Successful!", pSignInDBResult->m_iUUID);
		}
		else
			Log::WriteLog(L"Client [%d] Login Request Failed!", pSignInDBResult->m_iUUID);
		
		pNewTransmitQueueData = new FTransmitQueueData(pPacketData->m_pRequestedUser, CreateSignInResultPacket(flatBuffer->m_flatbuffer, iRequestResult, pSignInDBResult->m_iUUID));
	}
	return pNewTransmitQueueData;
}