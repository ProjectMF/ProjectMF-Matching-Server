#pragma once
#pragma comment(lib, "Rpcrt4.lib")
#include <rpc.h>
#include <iostream>

class CSystem {
public:
	uint16_t GenerateUUID();

};