#include "Pole.hpp"

pole::pole(int port, uint32_t id, uint32_t hwid) : _id(id), _TCPPort(port), _HWID(hwid) {};

pole::~pole() {
	closesocket(_TCPSocket);
}

void pole::sendData(std::string data) {

	// Check that the socket is indeed a socket and is actually a socket and is definitely a socket.
	if (_TCPSocket == INVALID_SOCKET) throw std::runtime_error("Socket is not initialised yet.");

	// If this happens I plan on quitting life. Not much I can do about the socket killing itself.
	if (send(_TCPSocket, data.c_str(), data.length(), 0) == SOCKET_ERROR) {
		throw std::runtime_error("Error in TCP packet send operation, error code: " + WSAGetLastError());
	}
}

void pole::_setupTCPSocket(int port) {

	SOCKET listeningSocket = INVALID_SOCKET;
	addrinfo* result = NULL, hints;

	// Zero the hints variable.
	ZeroMemory(&hints, sizeof(hints));

	// Configure the members of hints struct to tell WinSock to open a TCP port.
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// get the reply from WinSock on the actual parameters that we can use.
	if (getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result) != 0) {
		WSACleanup();
		throw std::runtime_error("Failed to get addrinfo for pole TCP comms port.");
	}

	// Create listening socket.
	listeningSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listeningSocket == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		throw std::runtime_error("Failed to create TCP socket for pole comms.");
	}

	// Bins listening socket.
	if (bind(listeningSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(listeningSocket);
		WSACleanup();
		throw std::runtime_error("Failed to bind TCP socket for pole comms.");
	}

	freeaddrinfo(result);

	std::cout << "Binded TCP Listening Socket.\n";

	// Listen for connection from pole.
	if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(listeningSocket);
		WSACleanup();
		throw std::runtime_error("Listening on TCP socket failed.");
	}

	std::cout << "Listening for TCP connection.\n";

	// Accept connection into to _TCPSocket.
	_TCPSocket = accept(listeningSocket, NULL, NULL);
	// Close temporary listening socket.
	closesocket(listeningSocket);

	std::cout << "Accepted TCP connection.\n";

	if (_TCPSocket == INVALID_SOCKET) std::cout << "Something went wrong with TCP Socket.\n";
}

void pole::_main() {

	int nBytesReceived;
	while (true) {
		nBytesReceived = recv(_TCPSocket, _TCPCommsBuffer, TCP_BUFF_SIZE, 0);
#ifdef _DEBUG
		if (nBytesReceived > 0) {
			std::cout << "Recieved data from pole " << _id << "\n";
			std::cout << _TCPCommsBuffer << std::endl;
			ZeroMemory(_TCPCommsBuffer, TCP_BUFF_SIZE);
			strcpy(_TCPCommsBuffer, "Hello There Pole ");
			std::strcat(_TCPCommsBuffer, (char*)std::to_string(_id).c_str());
			std::strcat(_TCPCommsBuffer, ".");
			std::cout << "Replying with: " << _TCPCommsBuffer << std::endl;
			send(_TCPSocket, _TCPCommsBuffer, TCP_BUFF_SIZE, 0);
		}

#endif // _DEBUG
	}
}