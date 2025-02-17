#include "WatchDog/WatchDog.h"

int main() {
	SocketAddress bindAddress("127.0.0.1", 3590);
	CWatchDog watchdogInstance;
	if (watchdogInstance.Initialize(EPROTOCOLTYPE::EPT_TCP, bindAddress)) {
		while (true)
			watchdogInstance.Run();
		watchdogInstance.Destroy();
	}
	return 0;
}