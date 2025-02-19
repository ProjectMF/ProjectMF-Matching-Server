#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <ServerLibrary/NetworkModel/IOCP/IOCP.hpp>
#include <ServerLibrary/WatchDogClient/WatchDogClient/WatchDogClient.hpp>
#include <ServerLibrary/WatchDogClient/Util/WatchDogUtil.h>
#include <ServerLibrary/Functions/Timer/Timer.hpp>
#include <unordered_map>
#include "../MailClient/MailClient.h"
#include "../DiscordBot/DiscordBot.h"

using namespace SERVER::NETWORKMODEL::IOCP;
using namespace SERVER::NETWORKMODEL::BASEMODEL;
using namespace SERVER::NETWORK::PACKET;

struct FClientInformation {
public:
	std::string m_sProcessName;
	uint64_t m_iDiscordBotChannelID;

public:
	FClientInformation(const std::string& sProcessName, const uint64_t iDiscordBotChannelID) : m_sProcessName(sProcessName), m_iDiscordBotChannelID(iDiscordBotChannelID) {

	};

};

class CWatchDog : public IOCP {
public:
	CWatchDog();

	virtual bool Initialize(const EPROTOCOLTYPE protocolType, SocketAddress& serverAddress) override final;
	virtual void Run() override final;
	virtual void Destroy() override final;

private:
	void NewProcessDetected(PacketQueueData* const pPacketData);
	void ProcessTerminated(PacketQueueData* const pPacketData);
	void PingReceived(PacketQueueData* const pPacketData);

	void ReceivedDump(PacketQueueData* const pPacketData);

	void SendPingToClients();

private:
	PACKETPROCESSOR m_packetProcessor;

	SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection m_csForClientInformation;
	std::unordered_map<CONNECTION*, FClientInformation> m_clientInformation;

	CMailClient m_mailClient;
	CDiscordBot m_discordBot;

	SERVER::FUNCTIONS::TIMER::CTimerSystem m_timerSystem;

};