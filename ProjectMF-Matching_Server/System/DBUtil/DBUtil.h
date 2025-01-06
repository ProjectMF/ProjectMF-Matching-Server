#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <flatbuffers/flatbuffers.h>
#include "../../Packet/data_define_generated.h"
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <sqltypes.h>

using namespace SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA;

#define MAX_NVARCHAR_LENGTH 50
#define EPIC_USER_ID_LENGTH 100

enum EDBRequestType : uint8_t {
	EDBType_None = 100,
	EDBType_SignInRequest,
	EDBType_UserAccountInfoRequest
};

struct FDBBaseQueueData {
public:
	EDBRequestType m_requestType;
	uint16_t m_requestResult;

	void* m_pRequestedClientConnection;

public:
	FDBBaseQueueData(const EDBRequestType requestType, void* pRequestedClientConnection) : m_requestResult(FlatPacket::RequestMessageType_Failed), m_requestType(requestType), m_pRequestedClientConnection(pRequestedClientConnection) {};

};

template<typename T>
struct FDBRequestBaseQueueData : public SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<T>, public FDBBaseQueueData {
public:
	FDBRequestBaseQueueData(const EDBRequestType requestType, void* pRequestedClientConnection) : FDBBaseQueueData(requestType, pRequestedClientConnection) {};
};


struct FDBSignInRequest : public FDBRequestBaseQueueData<FDBSignInRequest> {
public:
	char m_sEpicUserID[EPIC_USER_ID_LENGTH];
	int32_t m_iUUID;
	char m_sUserName[MAX_NVARCHAR_LENGTH];

public:
	FDBSignInRequest() : FDBRequestBaseQueueData<FDBSignInRequest>(EDBRequestType::EDBType_SignInRequest, nullptr), m_iUUID() {
		ZeroMemory(m_sEpicUserID, EPIC_USER_ID_LENGTH);
		ZeroMemory(m_sUserName, MAX_NVARCHAR_LENGTH);
	};

	FDBSignInRequest(void* pRequestedClientConnection, const char* const sEpicUserID, const int32_t iUUID) : m_iUUID(iUUID), FDBRequestBaseQueueData<FDBSignInRequest>(EDBRequestType::EDBType_SignInRequest, pRequestedClientConnection) {
		CopyMemory(m_sEpicUserID, sEpicUserID, EPIC_USER_ID_LENGTH);
		ZeroMemory(m_sUserName, MAX_NVARCHAR_LENGTH);
	};

};