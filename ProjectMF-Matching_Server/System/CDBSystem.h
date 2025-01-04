#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/SQL/MSSQL/MSSQL.hpp>
#include "../Packet/Util/Util.h"
#include <thread>
#include <mutex>
#include <atomic>
#include "DBUtil/DBUtil.h"

class CDBSystem {
	typedef SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<FDBBaseQueueData*, 200> DB_QUEUE_TYPE;
	typedef std::unordered_map<EDBRequestType, std::function<std::shared_ptr<void>(void*)>> DB_REQUEST_PROCESSOR;

	const std::chrono::milliseconds m_dbThreadLoopDuration;
	const std::function<void(struct FPacketProcessingData*)> m_dbRequestProcessedCallback;
private:
	SERVER::FUNCTIONS::SQL::MSSQL::CMSSQLPool m_sqlPool;

	DB_REQUEST_PROCESSOR m_requestProcessor;

private:
	std::unique_ptr<DB_QUEUE_TYPE> m_pDBRequestProcessingQueue;
	std::unique_ptr<DB_QUEUE_TYPE> m_pDBRequestStockQueue;

	std::atomic_bool m_bDBThreadRunState;
	std::thread m_dbRequestProcessingThread;

	std::condition_variable m_cvForDBWorkerThread;
	std::mutex m_csForDBWorkerThread;

private:
	std::shared_ptr<void> SignIn(void* const pRequestData);

private:
	void DBWorkerThread();

public:
	CDBSystem(const std::string& sHostName, const std::string& sDBName, const std::string& sUserName, const std::string& sPassword, const uint16_t iMaxPoolConnection, const std::function<void(struct FPacketProcessingData*)> dbRequestProcessedCallback);
	~CDBSystem();

public:
	void AddNewDBRequestData(FDBBaseQueueData* pNewQueueData);

};