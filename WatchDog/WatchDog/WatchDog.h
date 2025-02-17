#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <ServerLibrary/NetworkModel/IOCP/IOCP.hpp>
#include <ServerLibrary/WatchDog/WatchDogClient/WatchDogClient.hpp>
#include <ServerLibrary/WatchDog/Util/WatchDogUtil.h>
#include <unordered_map>
#include "../MailClient/MailClient.h"

using namespace SERVER::NETWORKMODEL::IOCP;

class CWatchDog : public IOCP {
private:
	PACKETPROCESSOR m_packetProcessor;

	std::unordered_map<CONNECTION*, SERVER::WATCHDOG::CLIENT::FWatchDogClientInformation> m_clientInformation;

	CMailClient m_mailClient;

private:
	void NewClientRequest(PacketQueueData* const pPacketData);
	void ClientDisconnectRequest(PacketQueueData* const pPacketData);

private:
	bool ClientDisconnect(CONNECTION* pConnection);

public:
	CWatchDog();

	virtual bool Initialize(const EPROTOCOLTYPE protocolType, SocketAddress& serverAddress) override final;
	virtual void Run() override final;
	virtual void Destroy() override final;

	virtual CONNECTION* OnIODisconnect(User_Server* const pClient) override final;

};