#include "CSystem.h"

uint16_t CSystem::GenerateUUID() {
	UUID userUUID;

	if (UuidCreate(&userUUID) != RPC_S_OK)
		return 0;

	RPC_STATUS result;
	auto iUUID = UuidHash(&userUUID, &result);

	if (result != RPC_S_OK)
		return 0;
	return iUUID;
}