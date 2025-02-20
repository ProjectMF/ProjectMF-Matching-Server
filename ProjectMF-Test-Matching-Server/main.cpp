#include <iostream>
#include "CIOCP.h"
#include <conio.h>
#include <ServerLibrary/Functions/Minidump/Minidump.hpp>

int main(int iArgc, char* sArgv[]) {
	CIOCP iocpInstance(sArgv[0]);

	SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress bindAddress("172.31.35.38", 19780);
	if (!iocpInstance.Initialize(EPROTOCOLTYPE::EPT_TCP, bindAddress))
		return -1;

	while (iocpInstance.GetIOCPRunState()) {
		if (_kbhit() != 0) {
			_getch();
			break;
		}
		iocpInstance.Run();
	}
	iocpInstance.Destroy();
	return 0;
}	