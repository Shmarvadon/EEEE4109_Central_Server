#pragma once

#include <iostream>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <vector>
#include "Pole.hpp"
#include <string>
#include <thread>
#include <mutex>

#include "json.hpp"

using json = nlohmann::json;


#define UDP_BUFF_SIZE 128

class server {
public:

	server() : _UDPListnerPort(42069), TCPportsrange({ 2000,3000 }), _UDPListenerThread(&server::_UDPListner,  this) {
		
	};

	~server();

	pole& getPole(int i) { return *_poles[i]; }

	uint32_t numberOfPoles() { return _poles.size(); }
private:

	void _UDPListner();

	std::thread _UDPListenerThread;
	int _UDPListnerPort;

	std::vector<pole*> _poles;

	std::pair<uint32_t, uint32_t> TCPportsrange;
};

