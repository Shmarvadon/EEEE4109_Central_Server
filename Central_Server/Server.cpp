#include "Server.hpp"

std::mutex m;

void server::_UDPListner() {
#ifdef _DEBUG
	std::cout << "Setting up UDP listner thread.\n";
#endif // _DEBUG

	/*   Setup the UDP port to listen for connections.   */
	SOCKET UDPListener = INVALID_SOCKET;

	char listnerBuff[UDP_BUFF_SIZE];
	ZeroMemory(listnerBuff, sizeof(char) * UDP_BUFF_SIZE);

	sockaddr_in listnerRecvAddr;
	listnerRecvAddr.sin_family = AF_INET;
	listnerRecvAddr.sin_port = htons(_UDPListnerPort);
	listnerRecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	WSABUF listnerDataBuff;
	listnerDataBuff.len = UDP_BUFF_SIZE;
	listnerDataBuff.buf = listnerBuff;

	DWORD listnerBytesReceived = 0;
	DWORD listnerFlags = 0;
	sockaddr_in listnerSenderAddress;
	int listnerSenderAddressSize = sizeof(listnerSenderAddress);

	{
		UDPListener = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, NULL);

		if (UDPListener == INVALID_SOCKET) {
			WSACleanup();
			throw std::runtime_error("Unable to setup UDP socket for listening.");
		}



		if (bind(UDPListener, (SOCKADDR*)&listnerRecvAddr, sizeof(listnerRecvAddr)) != 0) {
			closesocket(UDPListener);
			WSACleanup();
			throw std::runtime_error("Unable to bind UDP socket for listening.");
		}
	}

	/*   Setip the UDP port to send replies on.   */
	SOCKET UDPSender = INVALID_SOCKET;

	sockaddr_in senderRecieverAddress;
	senderRecieverAddress.sin_family = AF_INET;
	senderRecieverAddress.sin_port = htons(_UDPListnerPort - 1);
	

	sockaddr_in senderLocalAddr;
	senderLocalAddr.sin_family = AF_INET;
	senderLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	senderLocalAddr.sin_port = htons(_UDPListnerPort - 1);

	{
		UDPSender = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, NULL);

		if (UDPSender == INVALID_SOCKET) {
			WSACleanup();
			throw std::runtime_error("Unable to setup UDP socket for replying.");
		}

		if (bind(UDPSender, (sockaddr*)&senderLocalAddr, sizeof(senderLocalAddr)) != 0) {
			closesocket(UDPSender);
			WSACleanup();
			throw std::runtime_error("Unable to bind UDP socket for sending.");
		}
	}

	char senderBuff[UDP_BUFF_SIZE];
	WSABUF senderDataBuff;
	senderDataBuff.len = UDP_BUFF_SIZE;
	senderDataBuff.buf = senderBuff;
	DWORD senderBytesSent = 0;
	DWORD senderFlags = 0;


	/*   Listen for UDP broadcasts   */
	
	while (true) {
		ZeroMemory(listnerBuff, sizeof(char) * UDP_BUFF_SIZE);
		ZeroMemory(senderBuff, sizeof(char) * UDP_BUFF_SIZE);

		// If there is an error that isnt an IO pending.
		if (WSARecvFrom(UDPListener, &listnerDataBuff, 1, &listnerBytesReceived, &listnerFlags, (SOCKADDR*)&listnerSenderAddress, &listnerSenderAddressSize, NULL, NULL) != 0) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				closesocket(UDPListener);
				WSACleanup();
				throw std::runtime_error("WSA error. Unable to listen.");
			}
		}

#ifdef _DEBUG
		std::cout << "Recieved a UDP broadcast from a pole.\n";
#endif // _DEBUG

		//  Recieve the broadcast.
		std::lock_guard<std::mutex> lock(m);	// Acquire mutex lock untill scope exit to ensure no memory access clashes
		json handshake_data;
		try {
			handshake_data = json::parse(listnerBuff);
		}
		catch (std::exception e) {
			std::cout << e.what();
		}
#ifdef _DEBUG
		std::cout << "Pole data: \n" << handshake_data << "\n";
#endif // _DEBUG

		// Obtain a port that is usable for TCP.
		uint32_t polePort = (TCPportsrange.first + _poles.size() < TCPportsrange.second) ? TCPportsrange.first + _poles.size() : throw std::runtime_error("Ran out of valid TCP ports to use.");

		// Obtain a session ID for the pole.
		uint32_t poleSessionId = _poles.size();

#ifdef _DEBUG
		std::cout << "Parameters for pole configuration:\n\tTCP port: " << polePort << "\n\tSession id: " << poleSessionId << "\n";
#endif // _DEBUG


		// Pack reply data into buffer.
		memcpy(senderBuff, &polePort, sizeof(uint32_t));
		memcpy(&senderBuff[4], &poleSessionId, sizeof(uint32_t));

		// Place pointer to pole class into vector, it CAN NOT be the pole itself as SOCKET & std::thread dont play nicely with move or copying and I CBA to put in the effort to properly design the class.
		_poles.emplace_back(new pole(polePort, poleSessionId, std::stoi((std::string)handshake_data.at("HWID"))));

		// Send the reply.
		senderRecieverAddress.sin_addr.s_addr = listnerSenderAddress.sin_addr.S_un.S_addr;
		if (WSASendTo(UDPSender, &senderDataBuff, 1, &senderBytesSent, senderFlags, (SOCKADDR*)&senderRecieverAddress, sizeof(senderRecieverAddress), NULL, NULL) != 0) {
			closesocket(UDPSender);
			WSACleanup();
			throw std::runtime_error("Unable to send data over UDP socket.");
		}
		
	}

	/*   Thread termination cleanup   */

	// close the sockets.
	closesocket(UDPListener);
	closesocket(UDPSender);
}

server::~server() {
	_UDPListenerThread.join();

	WSACleanup();
}
