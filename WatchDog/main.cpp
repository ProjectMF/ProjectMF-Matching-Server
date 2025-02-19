#include "WatchDog/WatchDog.h"
#include <conio.h>

int main() {
	SocketAddress bindAddress("127.0.0.1", 3590);
	CWatchDog watchdogInstance;

	if (watchdogInstance.Initialize(EPROTOCOLTYPE::EPT_TCP, bindAddress)) {
		while (true) {
			if (_kbhit() != 0) {
				_getch();
				watchdogInstance.Destroy();

				break;
			}
			watchdogInstance.Run();
		}
	}
	return 0;
}