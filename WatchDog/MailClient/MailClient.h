#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include "../Packet/mail_data_define_generated.h"
#include <ServerLibrary/NetworkModel/EventSelect/EventSelect.hpp>
#include "DumpTransmitSystem/DumpTransmitSystem.h"

using namespace SERVER::NETWORKMODEL::EVENTSELECT;

class CMailClient : public EventSelect {
private:
	flatbuffers::FlatBufferBuilder m_flatbufferBuilder;

	CDumpTransmitSystem m_dumpTransmitSystem;

public:
	CMailClient();

	virtual bool Initialize(const EPROTOCOLTYPE protocolType, SocketAddress& serverAddress) override final;
	virtual void Destroy() override final;

public:
	void RequestMail(const Mail::RequestType requestType, const std::string& sProgramName);

	void AddNewDumpTransmitQueueData(const std::string& sProgramName, const std::string& sDumpFilePath);

};