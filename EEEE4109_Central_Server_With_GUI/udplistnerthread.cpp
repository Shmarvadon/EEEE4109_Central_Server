#include "udplistnerthread.h"

void udplistnerthread::run() {
	/*   Setup the UDP port to listen for connections.   */
	UDPListener = INVALID_SOCKET;

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

	/*   Setup the UDP port to send replies on.   */
	UDPSender = INVALID_SOCKET;

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
		int result = WSARecvFrom(UDPListener, &listnerDataBuff, 1, &listnerBytesReceived, &listnerFlags, (SOCKADDR*)&listnerSenderAddress, &listnerSenderAddressSize, NULL, NULL);
		if (result != 0) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				closesocket(UDPListener);
				WSACleanup();
				throw std::runtime_error("WSA error. Unable to listen.");
			}
		}

		/*			Recieve the broadcast			*/

		uint32_t polePort;

		// Obtain the poles HWID.
		uint64_t poleHWID;
		memcpy(&poleHWID, &listnerBuff[0], sizeof(uint64_t));

		// Obtain the poles type.
		uint8_t poleType = listnerBuff[8];

		// Set the address to reply to.
		senderRecieverAddress.sin_addr.s_addr = listnerSenderAddress.sin_addr.S_un.S_addr;

		// If the connecting device is of type pole.
		if (poleType == PoleType::LEDPole || poleType == PoleType::PhotoDiodePole) {
			// Check if the pole is one already connected attempting to reconnect.
			Pole* pPole = getPoleByHWID(poleHWID);

			// If the pole has already been connected this session then we tell it which port to say hi to.
			if (pPole != nullptr) {
				polePort = pPole->getSocketPort();
			}
			// If the pole has not been connected before in this session then we assign it to a new instance of the Pole class to handle it.
			else {
				polePort = (_TCPPortsRange.first + _NumberOfPolesConncted < _TCPPortsRange.second) ? _TCPPortsRange.first + _NumberOfPolesConncted : throw std::runtime_error("Ran out of valid TCP ports to use.");

				// Emit a signal to queue up the operation of appending the new pole to the _poles vector in the data model.
				emit appendNewPole(senderRecieverAddress, polePort, poleHWID, poleType);

				_NumberOfPolesConncted++;
			}
		}

		// If the connecting device is a start or finish gate.
		if (poleType == START_GATE_TYPE || poleType == FINSIH_GATE_TYPE) {

		}

		// Pack reply data into buffer.
		memcpy(senderBuff, &polePort, sizeof(uint32_t));

		// Send the reply.
		senderRecieverAddress.sin_addr.s_addr = listnerSenderAddress.sin_addr.S_un.S_addr;
		if (WSASendTo(UDPSender, &senderDataBuff, 1, &senderBytesSent, senderFlags, (SOCKADDR*)&senderRecieverAddress, sizeof(senderRecieverAddress), NULL, NULL) != 0) {
			closesocket(UDPSender);
			WSACleanup();
			throw std::runtime_error("Unable to send data over UDP socket.");
		}
	}

	/*   Thread termination cleanup   */
	// Hahahahaha this does not run :(

	// close the sockets.
	closesocket(UDPListener);
	closesocket(UDPSender);
}