#include "MailClient.h"
#include "../Util/MailUtil.h"

#define MAX_FLATBUFFER_LENGTH 2048

CMailClient::CMailClient() : m_flatbufferBuilder(MAX_FLATBUFFER_LENGTH), m_bMailTransmitThreadState(true) {
}

bool CMailClient::Initialize(SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	if(m_mailTransmitSocket.Connect(serverAddress)) {
		m_mailTransmitThread = std::thread(std::bind(&CMailClient::MailTransmitThread, this));

		return true;
	}
	return false;
}

void CMailClient::Destroy() {
	m_bMailTransmitThreadState = false;
	if (m_mailTransmitThread.joinable()) {
		m_cvForTransmitRequestQueue.notify_all();
		m_mailTransmitThread.join();
	}
}

void CMailClient::RequestMail(const Mail::RequestType requestType, const std::string& sProgramName) {
	std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
	m_mailTransmitRequestQueue.Push(new FNotificationMailTransmitQueueData(requestType, sProgramName.c_str()));
	m_cvForTransmitRequestQueue.notify_all();
}

void CMailClient::RequestMail(const Mail::RequestType requestType, const char* const sProgramName, const char* const sDumpFileName, const uint8_t* pDumpData, const size_t iDataSize) {
	std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
	m_mailTransmitRequestQueue.Push(new FDumpTransmitQueueData(requestType, sProgramName, sDumpFileName, pDumpData, iDataSize));
	m_cvForTransmitRequestQueue.notify_all();
}

void CMailClient::MailTransmitThread() {
	while (m_bMailTransmitThreadState) {
		std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
		m_cvForTransmitRequestQueue.wait(lck, [&]() { return !m_mailTransmitRequestQueue.IsEmpty() || !m_bMailTransmitThreadState; });
		lck.unlock();

		FBaseMailTransmitQueueData* pQueueData = nullptr;
		if (m_mailTransmitRequestQueue.Pop(pQueueData)) {
			if (pQueueData->m_packetType == Mail::PacketType_MailRequest) {
				auto pNotificationMail = static_cast<FNotificationMailTransmitQueueData*>(pQueueData);

				m_mailTransmitSocket.Write(::CreateMailRequestPacket(m_flatbufferBuilder, pNotificationMail->m_requestType, pNotificationMail->m_sProcessName));

				delete pNotificationMail;
			}
			else if(pQueueData->m_packetType == Mail::PacketType_DumpFileTransmit) {
				auto pDumpMail = static_cast<FDumpTransmitQueueData*>(pQueueData);

				m_mailTransmitSocket.Write(::CreateDumpTransmitPacket(m_flatbufferBuilder, pDumpMail->m_requestType, pDumpMail->m_sProgramName, pDumpMail->m_sDumpFileName, pDumpMail->m_sDumpData));

				delete pDumpMail;
			}
		}
	}
}