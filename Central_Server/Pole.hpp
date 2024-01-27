#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>

class pole {
public:

	pole(int port, uint32_t id);

private:

	SOCKET _TCPSocket = INVALID_SOCKET;
	uint32_t _id;
	uint32_t _HWID;
};

