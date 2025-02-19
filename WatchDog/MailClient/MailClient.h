#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <flatbuffers/flatbuffers.h>
#include <Network/NetworkProtocol/TCPSocket.hpp>
#include <Functions/CircularQueue/CircularQueue.hpp>
#include "../Packet/mail_data_define_generated.h"
#include <condition_variable>
#include <thread>
#include <atomic>
#include <mutex>

#define MAX_FILE_NAME 50
#define MAX_MESSAGE_BUFFER_LENGTH 1024

struct FBaseMailTransmitQueueData {
public:
	Mail::PacketType m_packetType;
	Mail::RequestType m_requestType;

public:
	FBaseMailTransmitQueueData() : m_packetType(), m_requestType() {}
	FBaseMailTransmitQueueData(const Mail::PacketType packetType, const Mail::RequestType requestType) : m_packetType(packetType), m_requestType(requestType) {}

};

struct FNotificationMailTransmitQueueData : public FBaseMailTransmitQueueData, public SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<FNotificationMailTransmitQueueData> {
public:
	std::string m_sProcessName;

public:
	FNotificationMailTransmitQueueData() : FBaseMailTransmitQueueData() {}
	FNotificationMailTransmitQueueData(const Mail::RequestType requestType, const std::string& sProcessName) 
		: FBaseMailTransmitQueueData(Mail::PacketType_MailRequest, requestType), m_sProcessName(sProcessName) {}

};

struct FDumpTransmitQueueData : public FBaseMailTransmitQueueData, public SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<FDumpTransmitQueueData> {
public:
	CHAR m_sProgramName[MAX_FILE_NAME];
	CHAR m_sDumpFileName[MAX_FILE_NAME];
	std::vector<uint8_t> m_sDumpData;

public:
	FDumpTransmitQueueData() : FBaseMailTransmitQueueData() {
		ZeroMemory(m_sProgramName, MAX_FILE_NAME);
		ZeroMemory(m_sDumpFileName, MAX_FILE_NAME);
	}

	FDumpTransmitQueueData(const Mail::RequestType requestType, const char* const sProgramName, const char* const sDumpFileName, const uint8_t* pDumpData, const size_t iDataSize)
		: FBaseMailTransmitQueueData(Mail::PacketType_DumpFileTransmit, requestType) 
	{
		ZeroMemory(m_sProgramName, MAX_FILE_NAME);
		CopyMemory(m_sProgramName, sProgramName, MAX_FILE_NAME);

		ZeroMemory(m_sDumpFileName, MAX_FILE_NAME);
		CopyMemory(m_sDumpFileName, sDumpFileName, MAX_FILE_NAME);

		m_sDumpData.resize(iDataSize);
		memcpy_s(&m_sDumpData.at(0), iDataSize, pDumpData, iDataSize);
	}

};

class CMailClient {
public:
	CMailClient();

	bool Initialize(SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress);
	void Destroy();

	void RequestMail(const Mail::RequestType requestType, const std::string& sProgramName);
	void RequestMail(const Mail::RequestType requestType, const char* const sProgramName, const char* const sDumpFileName, const uint8_t* pDumpData, const size_t iDataSize);

private:
	void MailTransmitThread();

private:
	flatbuffers::FlatBufferBuilder m_flatbufferBuilder;

	SERVER::NETWORK::PROTOCOL::TCP::TCPIPSocket m_mailTransmitSocket;

	std::atomic_bool m_bMailTransmitThreadState;
	std::thread m_mailTransmitThread;

	std::mutex m_csForTransmitRequestQueue;
	std::condition_variable m_cvForTransmitRequestQueue;
	SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<FBaseMailTransmitQueueData*> m_mailTransmitRequestQueue;

};