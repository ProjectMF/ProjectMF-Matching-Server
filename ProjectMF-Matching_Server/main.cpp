#include "CMatchingIOCP.h"
#include <Functions/Minidump/Minidump.hpp>

int main() {
	CMatchingIOCP iocpInstance;
	SocketAddress bindAddress("127.0.0.1", 3550);

	if (iocpInstance.Initialize(EPROTOCOLTYPE::EPT_TCP, bindAddress)) {
		while (iocpInstance.IOCPRunState())
			iocpInstance.Run();

		iocpInstance.Destroy();
	}
	return 0;
}