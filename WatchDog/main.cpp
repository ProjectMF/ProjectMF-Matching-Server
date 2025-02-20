#include "WatchDog/WatchDog.h"
#include <conio.h>

int main() {
	SocketAddress bindAddress("172.31.39.72", 19980);
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