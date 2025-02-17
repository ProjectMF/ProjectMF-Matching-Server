#include "WatchDog.h"
#include "../Util/MailUtil.h"
#include <sstream>

CWatchDog::CWatchDog() : IOCP(m_packetProcessor, 10) {
	m_packetProcessor.emplace(WatchDogPacket::PacketType_WatchDogStart, std::bind(&CWatchDog::NewClientRequest, this, std::placeholders::_1));
	m_packetProcessor.emplace(WatchDogPacket::PacketType_WatchDogEndRequest, std::bind(&CWatchDog::ClientDisconnectRequest, this, std::placeholders::_1));
}

bool CWatchDog::Initialize(const EPROTOCOLTYPE protocolType, SocketAddress& serverAddress) {
	if (IOCP::Initialize(protocolType, serverAddress)) {
		SocketAddress mailServerAddress("127.0.0.1", 3540);
		return m_mailClient.Initialize(EPROTOCOLTYPE::EPT_TCP, mailServerAddress);
	}
	return false;
}

void CWatchDog::Run() {
	IOCP::Run();

}

void CWatchDog::Destroy() {
	IOCP::Destroy();

	m_mailClient.Destroy();
}

CONNECTION* CWatchDog::OnIODisconnect(User_Server* const pClient) {
	if (auto pConnection = IOCP::OnIODisconnect(pClient)) {
		ClientDisconnect(pConnection);
		return pConnection;
	}
	return nullptr;
}

void CWatchDog::NewClientRequest(PacketQueueData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;

	auto pConnection = reinterpret_cast<CONNECTION*>(pPacketData->m_pOwner);
	auto pPacket = WatchDogPacket::GetWatchDogClientInformation(pPacketData->m_packetData->m_sPacketData);

	if (pConnection && pPacket) {
		SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Watch Dog : New Process Detected! [Name] : [%ls]", MBToUni(pPacket->program_name()->str()).c_str());

		SERVER::WATCHDOG::CLIENT::FWatchDogClientInformation newInformation(pPacket->program_name()->c_str(), pPacket->program_path()->c_str(), pPacket->dump_file_path()->c_str(), pPacket->command_line_argv()->c_str(), pPacket->enable_restart());
		m_mailClient.RequestMail(Mail::RequestType_Start, newInformation.m_sProgramName);
		m_clientInformation.emplace(pConnection, newInformation);
	}
}

void CWatchDog::ClientDisconnectRequest(PacketQueueData* const pPacketData) {
	using namespace SERVER::FUNCTIONS::UTIL;

	auto pConnection = reinterpret_cast<CONNECTION*>(pPacketData->m_pOwner);
	auto pPacket = WatchDogPacket::GetWatchDogClientEndRequest(pPacketData->m_packetData->m_sPacketData);

	if (pConnection && pPacket) {
		auto findResult = m_clientInformation.find(pConnection);
		if (findResult != m_clientInformation.cend()) {
			flatbuffers::FlatBufferBuilder flatBuffer;

			pConnection->m_pUser->Send(SERVER::WATCHDOG::UTIL::CreateWatchDogClientEndResultPacket(flatBuffer, WatchDogPacket::RequestMessageType::RequestMessageType_Succeeded));

			SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Watch Dog : [%ls] Process Terminated!", MBToUni(findResult->second.m_sProgramName).c_str());
			m_mailClient.RequestMail(Mail::RequestType_Stop, findResult->second.m_sProgramName);
			findResult->second.m_bProgramRunningState = false;
			findResult->second.m_bEnableRestart = pPacket->enable_restart();
		}
	}
}

bool CWatchDog::ClientDisconnect(CONNECTION* pConnection) {
	using namespace SERVER::FUNCTIONS::UTIL;

	auto findResult = m_clientInformation.find(pConnection);
	if (findResult != m_clientInformation.cend()) {
		if (findResult->second.m_bProgramRunningState) {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Watch Dog : [%ls] Process Aborted! Watch Dog Will Email You The Dump File!", MBToUni(findResult->second.m_sProgramName).c_str());

			m_mailClient.AddNewDumpTransmitQueueData(findResult->second.m_sProgramName, findResult->second.m_sDumpFilePath);
		}

		if (findResult->second.m_bEnableRestart) {
			const std::string sCombine = findResult->second.m_sProgramPath + "\\" + findResult->second.m_sProgramName;
			STARTUPINFOA startInfo = { sizeof(STARTUPINFOA) };
			PROCESS_INFORMATION processInformation;
			
			if (CreateProcessA(sCombine.c_str(), const_cast<char*>(findResult->second.m_sCommandLineArgv.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startInfo, &processInformation))
				SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Client Process Restart Successful : %ls", SERVER::FUNCTIONS::UTIL::MBToUni(sCombine).c_str());
			else
				SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Failed To Restart Client Process : %ls", SERVER::FUNCTIONS::UTIL::MBToUni(findResult->second.m_sProgramName).c_str());
		}
		m_clientInformation.erase(findResult);
		return true;
	}
	return false;
}