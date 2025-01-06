#pragma once
#define NOMINMAX
#include <flatbuffers/flatbuffers.h>
#include <Network/Packet/Serialization/serialization.hpp>
#include <sql.h>
#include <sqltypes.h>
#include <string>
#include <vector>
#include <regex>
#include "../data_define_generated.h"
#include "../sign_in_request_define_generated.h"
#include "../sign_in_result_define_generated.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace SERVER::NETWORK::PACKET::UTIL::SERIALIZATION;

static SYSTEMTIME GetCurrentDate() {
	SYSTEMTIME sysTime;
	auto localTime = boost::posix_time::second_clock::local_time();

	sysTime.wYear = localTime.date().year();
	sysTime.wMonth = localTime.date().month();
	sysTime.wDay = localTime.date().day();
	sysTime.wHour = localTime.time_of_day().hours();
	sysTime.wMinute = localTime.time_of_day().minutes();
	sysTime.wSecond = localTime.time_of_day().seconds();

	return sysTime;
}

static std::string RemoveEscapeCharactersFromJsonString(const std::string& jsonString) {
	std::regex escapeCharacters("[(\n\t)]");

	return std::regex_replace(jsonString, escapeCharacters, "");
}

static SYSTEMTIME operator-(const SYSTEMTIME& lhs, const uint16_t& iDays) {
	const boost::posix_time::ptime newDate(boost::gregorian::date(lhs.wYear, lhs.wMonth, lhs.wDay) - boost::gregorian::days(iDays));
	SYSTEMTIME sysTime;

	sysTime.wYear = newDate.date().year();
	sysTime.wMonth = newDate.date().month();
	sysTime.wDay = newDate.date().day();

	sysTime.wHour = lhs.wHour;
	sysTime.wMinute = lhs.wMinute;
	sysTime.wSecond = lhs.wSecond;

	return sysTime;
}

static bool operator==(const SYSTEMTIME& lhs, const SQL_DATE_STRUCT& rhs) {
	return boost::gregorian::date(lhs.wYear, lhs.wMonth, lhs.wDay) == boost::gregorian::date(rhs.year, rhs.month, rhs.day);

	return (lhs.wYear + lhs.wMonth + lhs.wDay) - (rhs.year + rhs.month + rhs.day);
}

static short GetLastDayOfMonth(const unsigned short& iYear, const unsigned short& iMonth) {
	return boost::gregorian::gregorian_calendar::end_of_month_day(iYear, iMonth);
}

static std::string GetTimeLeftUntilExpiration(const SQL_TIMESTAMP_STRUCT& receivedDateTime, const int16_t iDays) {
	const auto& currentSystemTime = ::GetCurrentDate();
	boost::posix_time::ptime expiredDate(boost::gregorian::date(receivedDateTime.year, receivedDateTime.month, receivedDateTime.day) + boost::gregorian::days(iDays), boost::posix_time::time_duration(receivedDateTime.hour - 1, 0, 0));
	boost::posix_time::ptime currentDate(boost::gregorian::date(currentSystemTime.wYear, currentSystemTime.wMonth, currentSystemTime.wDay), boost::posix_time::time_duration(currentSystemTime.wHour, 0, 0));

	const auto& remainDate = (expiredDate - currentDate);
	return std::to_string(remainDate.hours() / 24) + "Day " + std::to_string(remainDate.hours() % 24) + "Hour";
}

static uint16_t MakeDetailMessageType(const int8_t iMessageType, const int8_t iDetailType) {
	return ((iMessageType << 8) | iDetailType);
}

static uint8_t GetMessageTypeFromMessage(const uint16_t iMessage) {
	return static_cast<uint8_t>(iMessage >> 8);
}

static uint8_t GetDetailMessageFromMessage(const uint16_t iMessage) {
	return static_cast<uint8_t>(iMessage);
}

static std::string SQLDateTimeToString(const SQL_DATE_STRUCT& date) {
	return std::to_string(date.year) + "-" + std::to_string(date.month) + "-" + std::to_string(date.day);
}

static std::string SQLTimeStampToString(const SQL_TIMESTAMP_STRUCT& date) {
	return std::to_string(date.year) + "-" + std::to_string(date.month) + "-" + std::to_string(date.day) + ":" + std::to_string(date.hour) + "-" + std::to_string(date.minute) + "-" + std::to_string(date.second);
}

static std::string SQLTimeStampToSQLDateString(const SQL_TIMESTAMP_STRUCT& date) {
	return std::to_string(date.year) + "-" + std::to_string(date.month) + "-" + std::to_string(date.day);
}

static SQL_DATE_STRUCT StringToSQLDate(const std::string& str) {
	SQL_DATE_STRUCT ret;

	auto iYearOffset = str.find('-');
	ret.year = std::atoi(str.substr(0, iYearOffset).c_str());
	auto iMonthOffset = str.find('-', iYearOffset + 1);
	ret.month = std::atoi(str.substr(iYearOffset + 1, iMonthOffset).c_str());
	ret.day = std::atoi(str.substr(iMonthOffset + 1).c_str());

	return ret;
}

static SQL_TIMESTAMP_STRUCT StringToSQLTimeStamp(const std::string& str) {
	SQL_TIMESTAMP_STRUCT ret;

	auto iYearOffset = str.find('-');
	ret.year = std::atoi(str.substr(0, iYearOffset).c_str());
	auto iMonthOffset = str.find('-', iYearOffset + 1);
	ret.month = std::atoi(str.substr(iYearOffset + 1, iMonthOffset).c_str());
	ret.day = std::atoi(str.substr(iMonthOffset + 1).c_str());

	return ret;
}

static SQL_DATE_STRUCT SystemTimeToSQLDateStruct(const SYSTEMTIME& sysTime) {
	return SQL_DATE_STRUCT{ static_cast<short>(sysTime.wYear), sysTime.wMonth,sysTime.wDay };
}

static SQL_TIMESTAMP_STRUCT SystemTimeToSQLTimeStampStruct(const SYSTEMTIME& sysTime) {
	SQL_TIMESTAMP_STRUCT ret;
	ret.year = sysTime.wYear;
	ret.month = sysTime.wMonth;
	ret.day = sysTime.wDay;
	ret.hour = sysTime.wHour;
	ret.minute = sysTime.wMinute;
	ret.second = sysTime.wSecond;
	ret.fraction = 0;

	return ret;
}

static SERVER::NETWORK::PACKET::PACKET_STRUCT CreateSignInResultPacket(flatbuffers::FlatBufferBuilder& builder, const uint16_t iMessageType, const int32_t iUUID, const char* const sUserName) {
	return Serialize<FlatPacket::SignInResult>(builder, FlatPacket::PacketType_SignInResult, FlatPacket::CreateSignInResult(builder, iMessageType, iUUID, builder.CreateString(sUserName)));
}