#pragma once
#include "sleepy_discord/sleepy_discord.h"
#include <thread>
#include <ServerLibrary/Functions/CriticalSection/CriticalSection.hpp>
#include <concurrent_vector.h>

enum EMessageLevel : uint8_t {
	E_Alarm,
	E_Warning,
	E_Error,
	E_Ping
};

enum ECommandType : uint8_t {
	E_None,
	E_PingCommand
};

struct FCommandInformation {
public:
	ECommandType m_commandType;

	std::function<bool(const void* const)> m_callback;

public:
	FCommandInformation() : m_commandType() {};
	FCommandInformation(const ECommandType commandType, const std::function<bool(const void* const)>& callback) : m_commandType(commandType), m_callback(callback) {};

};

class CDiscordBot : private SleepyDiscord::DiscordClient {
public:
	CDiscordBot(const std::string& sBotToken);

	void Initialize();
	void Destroy();

public:
	void BindCommand(const ECommandType commandType, const std::function<bool(const void* const)>& callback);
	void Send(const uint64_t iDiscordBotChannelID, const EMessageLevel messageLevel, const std::string& sTitle, const std::string& sMessage);

protected:
	virtual void onInteraction(SleepyDiscord::Interaction interaction) override final;

private:
	void Run();

	bool ExecuteCommand(ECommandType commandType, const void* const pValue);

private:
	std::thread m_discordBotWorkerThread;

	SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection m_csForMessage;
	SleepyDiscord::Embed m_messageEmbed;

	concurrency::concurrent_vector<FCommandInformation> m_commandList;

};

