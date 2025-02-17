#include "MailClient.h"
#include "../Util/MailUtil.h"

#define MAX_FLATBUFFER_LENGTH 2048

CMailClient::CMailClient() : EventSelect(1, PACKETPROCESSOR()), m_flatbufferBuilder(MAX_FLATBUFFER_LENGTH) {
}

bool CMailClient::Initialize(const EPROTOCOLTYPE protocolType, SocketAddress& serverAddress) {
	if (EventSelect::Initialize(protocolType, serverAddress))
		return m_dumpTransmitSystem.Initialize(serverAddress);
	return false;
}

void CMailClient::Destroy() {
	EventSelect::Destroy();

	m_dumpTransmitSystem.Destroy();
}

void CMailClient::RequestMail(const Mail::RequestType requestType, const std::string& sProgramName) {
	Send(CreateMailRequestPacket(m_flatbufferBuilder, requestType, sProgramName));
}

void CMailClient::AddNewDumpTransmitQueueData(const std::string& sProgramName, const std::string& sDumpFilePath) {
	m_dumpTransmitSystem.AddNewDumpTransmitQueueData(sProgramName, sDumpFilePath);
}