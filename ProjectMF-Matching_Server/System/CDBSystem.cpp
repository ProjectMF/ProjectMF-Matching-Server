#include "CDBSystem.h"
#include "../PacketProcessor/CMatchingPacketProcessor.h"

#define SQL_VCHAR_DEFAULT_LENGTH 50
#define MAIL_REWARDS_JSON_LENGTH 512

using namespace SERVER::NETWORKMODEL::IOCP;

CDBSystem::CDBSystem(const std::string& sHostName, const std::string& sDBName, const std::string& sUserName, const std::string& sPassword, const uint16_t iMaxPoolConnection, const std::function<void(FPacketProcessingData*)> dbRequestProcessedCallback)
	: m_sqlPool(sHostName, sDBName, sUserName, sPassword, iMaxPoolConnection * 2), m_bDBThreadRunState(true), m_dbThreadLoopDuration(std::chrono::milliseconds(125)), m_dbRequestProcessedCallback(dbRequestProcessedCallback) {

	m_requestProcessor.emplace(EDBRequestType::EDBType_SignInRequest, std::bind(&CDBSystem::SignIn, this, std::placeholders::_1));

	m_pDBRequestProcessingQueue = std::make_unique<DB_QUEUE_TYPE>();
	m_pDBRequestStockQueue = std::make_unique<DB_QUEUE_TYPE>();

	m_dbRequestProcessingThread = std::thread(std::bind(&CDBSystem::DBWorkerThread, this));
}

CDBSystem::~CDBSystem() {
	m_bDBThreadRunState = false;
	if (m_dbRequestProcessingThread.joinable())
		m_dbRequestProcessingThread.join();
}


void CDBSystem::DBWorkerThread() {
	while (m_bDBThreadRunState) {
		// add cv

		if (m_pDBRequestProcessingQueue->IsEmpty() && !m_pDBRequestStockQueue->IsEmpty()) {
			m_csForDBWorkerThread.lock();
			m_pDBRequestStockQueue.swap(m_pDBRequestProcessingQueue);
			m_csForDBWorkerThread.unlock();
		}

		FDBBaseQueueData* pQueueData = nullptr;
		if (m_pDBRequestProcessingQueue->Pop(pQueueData)) {
			auto findResult = m_requestProcessor.find(pQueueData->m_requestType);
			if (findResult != m_requestProcessor.cend()) {
				if (auto pRet = findResult->second(pQueueData))
					m_dbRequestProcessedCallback(new FPacketProcessingData(static_cast<CONNECTION*>(pQueueData->m_pRequestedClientConnection), pQueueData->m_requestType, pRet));
			}
		}
	}
}

void CDBSystem::AddNewDBRequestData(FDBBaseQueueData* pNewQueueData) {
	std::unique_lock<std::mutex> lck(m_csForDBWorkerThread);
	m_pDBRequestStockQueue->Push(pNewQueueData);
}

std::shared_ptr<void> CDBSystem::SignIn(void* const pRequestData) {
	using namespace SERVER::FUNCTIONS::UTIL;

	if (auto pSignInRequest = static_cast<FDBSignInRequest* const>(pRequestData)) {
		auto pSTMT = m_sqlPool.GetConnection()->AllocSTMT();

		SQLPrepare(*pSTMT, (SQLWCHAR*)L"{call sign_in_by_epic_id (?, ?)}", SQL_NTS);
		
		SQLBindParameter(*pSTMT, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, EPIC_USER_ID_LENGTH, 0, pSignInRequest->m_sEpicUserID, 0, NULL);
		SQLBindParameter(*pSTMT, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_BIGINT, 0, 0, &pSignInRequest->m_iUUID, sizeof(SQLINTEGER), NULL);

		if (SQLExecute(*pSTMT) != SQL_SUCCESS)
			GetMSSQLErrorMessage(SQL_HANDLE_STMT, *pSTMT);
		else {
			while (SQLFetch(*pSTMT) == SQL_SUCCESS)
				SQLGetData(*pSTMT, 1, SQL_C_LONG, &pSignInRequest->m_iUUID, sizeof(SQLINTEGER), NULL);
			SQLFreeStmt(*pSTMT, SQL_CLOSE);

			pSignInRequest->m_requestResult = FlatPacket::RequestMessageType_Succeeded;
		}
		return std::shared_ptr<void>(pSignInRequest);
	}
	return nullptr;
}