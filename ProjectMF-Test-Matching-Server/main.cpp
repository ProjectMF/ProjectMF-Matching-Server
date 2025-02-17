#include <iostream>
#include "CIOCP.h"

int main() {
	CIOCP iocpInstance;

	SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress bindAddress("172.31.35.38", 19780);
	if (!iocpInstance.Initialize(EPROTOCOLTYPE::EPT_TCP, bindAddress))
		return -1;

	while (true)
		iocpInstance.Run();
	iocpInstance.Destroy();

	return 0;
}	