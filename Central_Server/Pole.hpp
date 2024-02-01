#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>
#include <vector>
#include <thread>

#define TCP_BUFF_SIZE 128

struct polestate {
	float velostatCurrent;
	uint16_t IRFrequency;
	float jerkThreashold;
	float batteryVoltage;

	bool IROn;
	bool VelostatOn;
	std::chrono::time_point<std::chrono::high_resolution_clock::time_point> lastUpdated;
};

struct polesenses {
	bool velostatHit;
	bool IRBreak;
	bool IMUTriggered;
	std::chrono::time_point<std::chrono::high_resolution_clock::time_point> lastUpdated;
};


class pole {
public:

	pole(int port, uint32_t id, uint32_t hwid) : _id(id), _TCPPort(port), _HWID(hwid), _TCPListnerThread(&pole::_setupTCPSocket, this) {
		ZeroMemory(_TCPCommsBuffer, TCP_BUFF_SIZE);
	};

	~pole() {
		_TCPListnerThread.join();
		closesocket(_TCPSocket);
	};


	void sendData(std::string data);

	void pollSensors();

	void pollSensor(uint8_t sensor);

	void pollState();

	/*   Methods for setting and updating stuff   */		// THESE ARE INCOMPLETE IMPLIMENT THEM ANOTHER TIME.

	void setVelostatCurrent(float current);

	float getVelostatCurrent();

	float getBatteryLife();

	bool getVelostatState();

	void setVelostatState(bool state);

	bool getIRState();

	void setIRState(bool state);

private:

	void _setupTCPSocket();
	void _main();
	SOCKET _TCPSocket = INVALID_SOCKET;
	std::thread _TCPListnerThread;
	uint32_t _id = 0;
	uint32_t _HWID = 0;
	int _TCPPort;
	char _TCPCommsBuffer[TCP_BUFF_SIZE];

	polestate _poleState;
	polesenses _poleSenses;
};

