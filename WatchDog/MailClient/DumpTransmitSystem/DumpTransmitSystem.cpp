#include "DumpTransmitSystem.h"
#include "../../Util/MailUtil.h"
#include <functional>
#include <chrono>

#define MAX_MESSAGE_BUFFER_LENGTH 1024

CDumpTransmitSystem::CDumpTransmitSystem() : m_bDumpFileTransmitThreadState(true), m_flatbuffer(MAX_MESSAGE_BUFFER_LENGTH * 2) {
}

bool CDumpTransmitSystem::Initialize(SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	if (m_dumpTransmitSocket.Connect(serverAddress)) {
		m_dumpFileTransmitThread = std::thread(std::bind(&CDumpTransmitSystem::DumpFileTransmitThread, this));
		return true;
	}
	return false;
}

void CDumpTransmitSystem::Destroy() {
	m_bDumpFileTransmitThreadState = false;
	if (m_dumpFileTransmitThread.joinable())
		m_dumpFileTransmitThread.join();
}

void CDumpTransmitSystem::AddNewDumpTransmitQueueData(const std::string& sProgramName, const std::string& sDumpFilePath) {
	WIN32_FIND_DATAA fileFindData;
	HANDLE hFileFindHandle = FindFirstFileA((sDumpFilePath + "\\*.dmp").c_str(), &fileFindData);
	if (hFileFindHandle != INVALID_HANDLE_VALUE) {
		std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
		m_dumpTransmitRequestQueue.Push(new FDumpTransmitRequestQueueData(sProgramName.c_str(), (sDumpFilePath + "\\" + fileFindData.cFileName).c_str(), fileFindData.cFileName));
		m_cvForTransmitRequestQueue.notify_all();
	}
}

void CDumpTransmitSystem::DumpFileTransmitThread() {
	std::vector<uint8_t> messageBuffer(MAX_MESSAGE_BUFFER_LENGTH, '\0');

	while (m_bDumpFileTransmitThreadState) {
		std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
		m_cvForTransmitRequestQueue.wait(lck, [&]() { return !m_dumpTransmitRequestQueue.IsEmpty(); });
		lck.unlock();

		FDumpTransmitRequestQueueData* pQueueData = nullptr;
		if (m_dumpTransmitRequestQueue.Pop(pQueueData)) {
			std::ifstream dumpFileStream(pQueueData->m_sDumpFilePath, std::ios::in | std::ios::binary);
			if (dumpFileStream.is_open()) {
				Mail::RequestType requestType = Mail::RequestType::RequestType_Start;

				while (!dumpFileStream.eof()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(125));

					ZeroMemory(&messageBuffer.at(0), MAX_MESSAGE_BUFFER_LENGTH);
					std::streamsize iReadBytes = dumpFileStream.read(reinterpret_cast<char*>(&messageBuffer.at(0)), MAX_MESSAGE_BUFFER_LENGTH).gcount();
					if (iReadBytes < MAX_MESSAGE_BUFFER_LENGTH)
						requestType = Mail::RequestType::RequestType_Stop;
					else
						requestType = Mail::RequestType::RequestType_Loop;

					m_dumpTransmitSocket.Write(::CreateDumpTransmitPacket(m_flatbuffer, requestType, pQueueData->m_sProgramName, pQueueData->m_sDumpFileName, messageBuffer));
				}
				dumpFileStream.close();
				DeleteFileA(pQueueData->m_sDumpFilePath);
			}
		}
	}
}