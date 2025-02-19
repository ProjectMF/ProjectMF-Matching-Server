#include "WatchDog.h"
#include "../Util/MailUtil.h"
#include <sstream>
#include <ServerLibrary/Functions/FPSManagement/FPSManager.hpp>

#define SECOND 60
#define MINUTE(x) ((x) * 60)
#define HOUR(x) ((x) * (MINUTE(x) * SECOND))

CWatchDog::CWatchDog() : IOCP(m_packetProcessor, 10), m_discordBot("") {
	m_packetProcessor.emplace(WatchDogPacket::PacketType_NewProcessDetected, std::bind(&CWatchDog::NewProcessDetected, this, std::placeholders::_1));
	m_packetProcessor.emplace(WatchDogPacket::PacketType_ProcessTerminated, std::bind(&CWatchDog::ProcessTerminated, this, std::placeholders::_1));
	m_packetProcessor.emplace(WatchDogPacket::PacketType_DumpFile, std::bind(&CWatchDog::ReceivedDump, this, std::placeholders::_1));
	m_packetProcessor.emplace(WatchDogPacket::PacketType_Ping, std::bind(&CWatchDog::PingReceived, this, std::placeholders::_1));
}

bool CWatchDog::Initialize(const EPROTOCOLTYPE protocolType, SocketAddress& serverAddress) {
	if (IOCP::Initialize(protocolType, serverAddress)) {
		m_discordBot.Initialize();
		
		SocketAddress mailServerAddress("127.0.0.1", 3540);
		return m_mailClient.Initialize(mailServerAddress);
	}
	return false;
}

void CWatchDog::Run() {
	using namespace SERVER::FUNCTIONS;
	if (!FPS::FPSMANAGER::Skip()) {
		IOCP::Run();
		

	}
}

void CWatchDog::Destroy() {
	IOCP::Destroy();

	m_mailClient.Destroy();
	m_discordBot.Destroy();
}

void CWatchDog::NewProcessDetected(PacketQueueData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;
	using namespace SERVER::FUNCTIONS::CRITICALSECTION;

	auto pConnection = reinterpret_cast<CONNECTION*>(pPacketData->m_pOwner);
	auto pPacket = WatchDogPacket::GetWatchDogClientInformation(pPacketData->m_packetData->m_sPacketData);

	if (pConnection && pPacket) {
		SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Watch Dog : New Process Detected! [Name] : [%ls]", MBToUni(pPacket->program_name()->str()).c_str());

		FClientInformation newProcessInformation(pPacket->program_name()->c_str(), pPacket->discord_bot_channe_id());
		
		m_mailClient.RequestMail(Mail::RequestType_Start, newProcessInformation.m_sProcessName);

		CriticalSectionGuard lock(m_csForClientInformation);
		auto emplaceResult = m_clientInformation.emplace(pConnection, newProcessInformation);

		if (emplaceResult.second && newProcessInformation.m_iDiscordBotChannelID != 0) {
			m_discordBot.Send(newProcessInformation.m_iDiscordBotChannelID, EMessageLevel::E_Alarm, "NOTIFY", "```ansi\n[1;32m[" + newProcessInformation.m_sProcessName+ "] Is Running.\n[1m```");

			m_timerSystem.BindTimer(std::bind(&CWatchDog::SendPingToClients, this), 5, false);
		}
	}
}

void CWatchDog::ProcessTerminated(PacketQueueData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;
	using namespace SERVER::FUNCTIONS::CRITICALSECTION;

	auto pConnection = reinterpret_cast<CONNECTION*>(pPacketData->m_pOwner);
	auto pTerminatedPacket = WatchDogPacket::GetWatchDogClientTerminated(pPacketData->m_packetData->m_sPacketData);

	if (pConnection && pTerminatedPacket) {
		CriticalSectionGuard lock(m_csForClientInformation);

		auto findResult = m_clientInformation.find(pConnection);
		if (findResult != m_clientInformation.cend()) {
			if (pTerminatedPacket->has_dump()) {
				SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Watch Dog : [%ls] Process Aborted! Watch Dog Will Email You The Dump File!", MBToUni(findResult->second.m_sProcessName).c_str());
				m_discordBot.Send(findResult->second.m_iDiscordBotChannelID, EMessageLevel::E_Error, "NOTIFY", "```ansi\n[1;31m[" + findResult->second.m_sProcessName+ "] Is Aborted![1m```");
			}
			else {
				m_mailClient.RequestMail(Mail::RequestType_Stop, findResult->second.m_sProcessName);

				SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Watch Dog : [%ls] Process Terminated!", MBToUni(findResult->second.m_sProcessName).c_str());
				m_discordBot.Send(findResult->second.m_iDiscordBotChannelID, EMessageLevel::E_Alarm, "NOTIFY", "```ansi\n[1;34m[" + findResult->second.m_sProcessName + "] Is Stopped.[1m```");
			}
			m_clientInformation.erase(findResult);
		}
	}
}

void CWatchDog::PingReceived(PacketQueueData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;
	using namespace SERVER::FUNCTIONS::CRITICALSECTION;

	auto pConnection = reinterpret_cast<CONNECTION*>(pPacketData->m_pOwner);
	auto pPingPacket = WatchDogPacket::GetPingPacket(pPacketData->m_packetData->m_sPacketData);

	if (pConnection && pPingPacket) {
		CriticalSectionGuard lock(m_csForClientInformation);

		auto findResult = m_clientInformation.find(pConnection);
		if (findResult != m_clientInformation.cend()) {
			std::stringstream stringStream;

			stringStream << "CPU USAGE : " << pPingPacket->cpu_usage() << "% / 100.00% \n";
			stringStream << "MEMORY USAGE : " << pPingPacket->memory_usage() << "MB / " << pPingPacket->total_memory()<< "MB \n";
			stringStream << "CONNECTED USERS : " << pPingPacket->connected_user_count() << "\n";

			m_discordBot.Send(
				findResult->second.m_iDiscordBotChannelID, 
				EMessageLevel::E_Ping, 
				"NOTIFY", 
				"```ansi\n[1;35m[" + findResult->second.m_sProcessName + "] STATE\n" + stringStream.str() + "[1m```");
		}
	}
}

void CWatchDog::ReceivedDump(PacketQueueData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;
	using namespace SERVER::FUNCTIONS::CRITICALSECTION;

	auto pDumpPacket = WatchDogPacket::GetDumpTransmitPacket(pPacketData->m_packetData->m_sPacketData);
	if (pDumpPacket) 
		m_mailClient.RequestMail((const Mail::RequestType)pDumpPacket->dump_transfer_status(), pDumpPacket->program_name()->c_str(), pDumpPacket->dump_file_name()->c_str(), pDumpPacket->dump_data()->Data(), pDumpPacket->dump_data()->Length());
}


void CWatchDog::SendPingToClients() {
	using namespace SERVER::FUNCTIONS::CRITICALSECTION;

	CriticalSectionGuard lock(m_csForClientInformation);
	for (auto& iterator : m_clientInformation) {
		flatbuffers::FlatBufferBuilder builder;

		iterator.first->m_pUser->Send(SERVER::WATCHDOG::UTIL::CreatePingPacket(builder));
	}
}
