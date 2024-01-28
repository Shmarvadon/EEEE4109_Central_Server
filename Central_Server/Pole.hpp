#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>
#include <vector>

#define TCP_BUFF_SIZE 128

class pole {
public:

	pole(int port, uint32_t id, uint32_t hwid);

	void operator()(){
		_setupTCPSocket(_TCPPort);

		_main();
	}

	~pole();


	void sendData(std::string data);

private:

	void _setupTCPSocket(int port);
	void _main();
	SOCKET _TCPSocket = INVALID_SOCKET;
	uint32_t _id = 0;
	uint32_t _HWID = 0;
	int _TCPPort;
	char _TCPCommsBuffer[TCP_BUFF_SIZE];
};

