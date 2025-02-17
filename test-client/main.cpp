#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "../ProjectMF-Test-Matching-Server/data_define_generated.h"
#include "../ProjectMF-Test-Matching-Server/sign_in_request_define_generated.h"
#include "../ProjectMF-Test-Matching-Server/sign_in_result_define_generated.h"
#include "ServerLibrary/Network/Packet/Serialization/serialization.hpp"

int main() {
	WSADATA wsaData;
	WSAStartup(WINSOCK_VERSION, &wsaData);

	int clid;
	char buffer[BUFSIZ];
	struct sockaddr_in socketaddr;
	struct sockaddr socbuf;
	size_t sockrwlen;

	memset((char*)&socketaddr, 0, sizeof(struct sockaddr_in));
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(19780);
	socketaddr.sin_addr.s_addr = inet_addr("54.180.22.22");
	clid = socket(AF_INET, SOCK_STREAM, 0);
	if (clid < 0) {
		perror("socket");
		return -1;
	}
	printf("Connecting...\n");
	if (connect(clid, (struct sockaddr*)&socketaddr, sizeof(struct sockaddr_in))) {
		perror("connect");
		return -1;
	}
	printf("Connected.\n");

	flatbuffers::FlatBufferBuilder builder;

	auto packet = SERVER::NETWORK::PACKET::UTIL::SERIALIZATION::Serialize(builder, FlatPacket::PacketType::PacketType_SignInRequest, FlatPacket::CreateSignInRequest(builder, builder.CreateString("PPODO")));

	char messageBuffer[2048];

	memcpy(messageBuffer, (void*)&packet.m_packetInfo, sizeof(packet.m_packetInfo));
	memcpy(messageBuffer + sizeof(packet.m_packetInfo), packet.m_sPacketData, strlen(packet.m_sPacketData));

	send(clid, messageBuffer, 2048, 0);

	char recvMessageBuffer[2048];

	recv(clid, recvMessageBuffer, 2048, 0);

	auto p = FlatPacket::GetSignInResult(recvMessageBuffer + sizeof(packet.m_packetInfo));

	std::cout << p->uuid() << std::endl;

	closesocket(clid);
	printf("Disconnected.\n");
	return 0;
	
	WSACleanup();
	return 0;
}