#pragma once
#include "sleepy_discord/sleepy_discord.h"
#include <thread>
#include <ServerLibrary/Functions/CriticalSection/CriticalSection.hpp>

enum EMessageLevel : uint8_t {
	E_Alarm,
	E_Warning,
	E_Error,
	E_Ping
};

class CDiscordBot : private SleepyDiscord::DiscordClient {
public:
	CDiscordBot(const std::string& sBotToken);

	void Initialize();
	void Destroy();

public:
	void Send(const uint64_t iDiscordBotChannelID, const EMessageLevel messageLevel, const std::string& sTitle, const std::string& sMessage);

private:
	void Run();

private:
	std::thread m_discordBotWorkerThread;

	SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection m_csForMessage;
	SleepyDiscord::Embed m_messageEmbed;

};

