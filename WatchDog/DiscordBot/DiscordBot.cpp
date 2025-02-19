#include "DiscordBot.h"
#include <functional>

#define MAX_EMBED_FIELD_COUNT 1

CDiscordBot::CDiscordBot(const std::string& sBotToken) : SleepyDiscord::DiscordClient(sBotToken, SleepyDiscord::USER_CONTROLED_THREADS) {
	m_messageEmbed.fields.reserve(MAX_EMBED_FIELD_COUNT);
}

void CDiscordBot::Initialize() {
	setIntents(SleepyDiscord::Intent::SERVER_MESSAGES, SleepyDiscord::Intent::MESSAGE_CONTENT);

	m_discordBotWorkerThread = std::thread(std::bind(&CDiscordBot::Run, this));
}

void CDiscordBot::Run() {
	run();
}

void CDiscordBot::Destroy() {
	ioContext->stop();

	m_discordBotWorkerThread.join();
}

void CDiscordBot::Send(const uint64_t iDiscordBotChannelID, const EMessageLevel messageLevel, const std::string& sTitle, const std::string& sMessage) {
	SERVER::FUNCTIONS::CRITICALSECTION::CriticalSectionGuard lock(m_csForMessage);

	m_messageEmbed.title = sTitle;

	SleepyDiscord::EmbedField field;
	switch (messageLevel) {
	case EMessageLevel::E_Alarm:
		field.name = "ALARM";
		m_messageEmbed.color = 9807270;
		break;
	case EMessageLevel::E_Warning:
		field.name = "WARNING";
		m_messageEmbed.color = 16776960;
		break;
	case EMessageLevel::E_Error:
		field.name = "ERROR";
		m_messageEmbed.color = 15548997;
		break;
	case EMessageLevel::E_Ping:
		field.name = "PING";
		m_messageEmbed.color = 3447003;
		break;
	default:
		field.name = "UNKNOWN";
		m_messageEmbed.color = 0;
		break;
	}
	field.value = sMessage;
	m_messageEmbed.fields.emplace_back(field);

	sendMessage(iDiscordBotChannelID, "", m_messageEmbed);

	m_messageEmbed.fields.clear();
}
