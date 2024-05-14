#include "pole.h"


void polecommsthread::run() {

	_setupUDPConnection();

	// Timer to handle keep alive of the link.
	_mainLoopTimer = new QTimer(this);
	_mainLoopTimer->setTimerType(Qt::PreciseTimer);
	connect(_mainLoopTimer, &QTimer::timeout, this, &polecommsthread::mainLoop, Qt::DirectConnection);

	_mainLoopTimer->start(1000);

	//Setup realtime stream timer.
	_realtimeStreamTimer = new QTimer(this);
	_realtimeStreamTimer->setTimerType(Qt::PreciseTimer);
	connect(_realtimeStreamTimer, &QTimer::timeout, this, &polecommsthread::_realtimeStreamingLoop, Qt::DirectConnection);

	//throw std::runtime_error("");
}

void polecommsthread::mainLoop() {
	// If the last transmission was more than 5000 ms ago then we ping to ask if the pole is alive.
	auto now = std::chrono::high_resolution_clock::now() - lastSynced;
	if ((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now).count() > 100) {
		//throw std::runtime_error("");
		_pingPole();
	}
	else emit UpdatePoleConnectionStatus(pcs::Connected);

	//throw std::runtime_error("");
}

void polecommsthread::_setupUDPConnection() {

	ZeroMemory(_CommsBuffer, sizeof(char) * COMMS_BUFF_SIZE);

	// Set the port on the remote UDP socket to send to.
	_Dest.sin_port = htons(_Port);

	// Local stuff.
	sockaddr_in RecvAddr;
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(_Port);
	RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);// inet_addr("127.0.0.1");

	WSABUF DataBuff;
	DataBuff.len = COMMS_BUFF_SIZE;
	DataBuff.buf = _CommsBuffer;

	DWORD listnerBytesReceived = 0;
	DWORD listnerFlags = 0;
	sockaddr_in listnerSenderAddress;
	int listnerSenderAddressSize = sizeof(listnerSenderAddress);

	{
		_UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);// WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, NULL); //socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (_UDPSocket == INVALID_SOCKET) {
			WSACleanup();
			throw std::runtime_error("Unable to setup pole comms socket.");
		}



		if (bind(_UDPSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr)) != 0) {
			closesocket(_UDPSocket);
			WSACleanup();
			throw std::runtime_error("Unable to setup poles comms socket.");
		}

		// FUCK YOU QT AND YOUR STUPID ASS FUNCTION NAMING CONFLICTING WITH WINSOCK.
		if ( :: connect(_UDPSocket, (SOCKADDR*)&_Dest, sizeof(_Dest)) != 0) {

			std::cout << WSAGetLastError() << std::endl;
			throw std::runtime_error("Failed to connect to pole.");
		}
	}

	// Make the socket non blocking.
	_setSocketBlockingMode(false, 0);

	// Set now as the last time the pole has been talked to.
	lastSynced = std::chrono::high_resolution_clock::now();

	emit UpdatePoleConnectionStatus(pcs::Connected);
}

bool polecommsthread::_pingPole() {
	bool poleStillHere = false;
	char pingPacket = ptt::Ping;



	//throw std::runtime_error("");

	int result = sendto(_UDPSocket, &pingPacket, 1, 0, (sockaddr*)&_Dest, sizeof(_Dest));

	//throw std::runtime_error("");

	// Make the socket blocking & set timeout to 50ms.
	_setSocketBlockingMode(true, 50);

	// call recv.
	int size = sizeof(_Dest);
	result = recvfrom(_UDPSocket, _CommsBuffer, COMMS_BUFF_SIZE, 0, (SOCKADDR*)&_Dest, &size);		// Problem is here.

	//throw std::runtime_error("");

	if (result > 0) {
		if (_CommsBuffer[0] == ptt::Pong) {

			lastSynced = std::chrono::high_resolution_clock::now();
			poleStillHere = true;

			//throw std::runtime_error("");
			emit UpdatePoleConnectionStatus(pcs::Connected);
			// We good.
		}
		else {
			// We bad. Might want to try and recover connection here.
			//throw std::runtime_error("");
			emit UpdatePoleConnectionStatus(pcs::Disconnected);
			poleStillHere = false;
		}
	}
	else {
		// We bad. Might want to try and recover connection here.
		//throw std::runtime_error("");
		emit UpdatePoleConnectionStatus(pcs::Disconnected);
		poleStillHere = false;
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return poleStillHere;
}

sensors polecommsthread::syncSensorReadingsToServer() {
	sensors newData;
	bool completedSuccessfully = false;

	// Make the socket blocking & set timeout to 10ms.
	_setSocketBlockingMode(true, 10);

	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	_CommsBuffer[0] = ptt::SyncToServer | ptt::Sensors;
	int result = sendto(_UDPSocket, _CommsBuffer, 1, 0, (sockaddr*)&_Dest, sizeof(_Dest));

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	int size = sizeof(_Dest);
	nBytesReceived = recvfrom(_UDPSocket, _CommsBuffer, COMMS_BUFF_SIZE, 0, (SOCKADDR*)&_Dest, &size);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_CommsBuffer[0] == ptt::SyncToServer | ptt::Sensors) {

			// memcopy the recieved data into the struct.
			memcpy(&newData.velostatReading, &_CommsBuffer[1], sizeof(float));
			memcpy(&newData.IRGateReading, &_CommsBuffer[5], sizeof(float));
			memcpy(&newData.IRCameraReading, &_CommsBuffer[9], sizeof(float));
			memcpy(&newData.IMUReading, &_CommsBuffer[13], sizeof(float));
			memcpy(&newData.batteryReading, &_CommsBuffer[17], sizeof(int));

			completedSuccessfully = true;
		}

		if (_CommsBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// Check not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully ? newData : throw std::runtime_error("Issue with syncing sensor data to server");
}

events polecommsthread::syncEventsToServer() {
	events newData = (events)0;
	bool completedSuccessfully = false;

	// Make the socket blocking & set timeout to 500ms.
	_setSocketBlockingMode(true, 100);

	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	_CommsBuffer[0] = ptt::SyncToServer | ptt::Events;
	int result = sendto(_UDPSocket, _CommsBuffer, 1, 0, (sockaddr*)&_Dest, sizeof(_Dest));

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	int size = sizeof(_Dest);
	nBytesReceived = recvfrom(_UDPSocket, _CommsBuffer, COMMS_BUFF_SIZE, 0, (SOCKADDR*)&_Dest, &size);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		completedSuccessfully = true;

		//char something = _CommsBuffer[0];

		//std::cout << (int)something << std::endl;

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_CommsBuffer[0] == ptt::SyncToServer | ptt::Events) {

			newData = (events)_CommsBuffer[1];

			completedSuccessfully = true;
		}

		if (_CommsBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// CHeck not strictly needed but could allow for future enhanced handling.
	}
	else {
		std::cout << "An Error occured.\n";

		int error = WSAGetLastError();
		std::cout << "Error is " << error << "\n";
	}


	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully ? newData : throw std::runtime_error("Issue with syncing events data to server");
}

settings polecommsthread::syncSettingsToServer() {
	settings newData;
	bool completedSuccessfully = false;

	// Make the socket blocking & set timeout to 500ms.
	_setSocketBlockingMode(true, 1000);

	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	_CommsBuffer[0] = ptt::SyncToServer | ptt::Configurables;
	int result = sendto(_UDPSocket, _CommsBuffer, 1, 0, (sockaddr*)&_Dest, sizeof(_Dest));

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	int size = sizeof(_Dest);
	nBytesReceived = recvfrom(_UDPSocket, _CommsBuffer, COMMS_BUFF_SIZE, 0, (SOCKADDR*)&_Dest, &size);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_CommsBuffer[0] == ptt::SyncToServer | ptt::Configurables) {

			// memcpy the data into the struct.
			memcpy(&newData.IRTransmitFreq, &_CommsBuffer[1], sizeof(uint16_t));
			memcpy(&newData.IMUSensitivity, &_CommsBuffer[3], sizeof(float));
			memcpy(&newData.velostatSensitivity, &_CommsBuffer[7], sizeof(float));
			memcpy(&newData.powerState, &_CommsBuffer[11], sizeof(uint8_t));


			completedSuccessfully = true;
		}

		if (_CommsBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// CHeck not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully ? newData : throw std::runtime_error("Issue with syncing settings data to server");
}

void polecommsthread::_setSocketBlockingMode(bool blocking, uint32_t timeout) {

	if (blocking) {
		// Make the socket blocking.
		u_long blocking_mode = 0;
		ioctlsocket(_UDPSocket, FIONBIO, &blocking_mode);

		// Set the timeout for blocking operations in milliseconds.
		int result = setsockopt(_UDPSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	}

	if (!blocking) {
		// Make the socket not blocking.
		u_long blocking_mode = 1;
		ioctlsocket(_UDPSocket, FIONBIO, &blocking_mode);
	}
}

bool polecommsthread::syncSettingsToPole(settings Settings) {
	bool completedSuccessfully = false;

	// Zero the buffer & pack data into it for transmission.
	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	_CommsBuffer[0] = ptt::SyncToPole | ptt::Configurables;
	memcpy(&_CommsBuffer[1], &Settings.IRTransmitFreq, sizeof(uint16_t));
	memcpy(&_CommsBuffer[3], &Settings.IMUSensitivity, sizeof(float));
	memcpy(&_CommsBuffer[7], &Settings.velostatSensitivity, sizeof(float));
	memcpy(&_CommsBuffer[11], &Settings.powerState, sizeof(uint8_t));

	// Transmit the data to the pole.
	int result = sendto(_UDPSocket, _CommsBuffer, 12, 0, (sockaddr*)&_Dest, sizeof(_Dest));

	// Make the socket blocking & set timeout to 100ms.
	_setSocketBlockingMode(true, 100);

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_CommsBuffer, COMMS_BUFF_SIZE);
	int size = sizeof(_Dest);
	nBytesReceived = recvfrom(_UDPSocket, _CommsBuffer, 1, 0, (SOCKADDR*)&_Dest, &size);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		// If the reply is a Pong then the server did well.
		if (_CommsBuffer[0] == ptt::Pong) {
			completedSuccessfully = true;
		}

		if (_CommsBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// Check not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully;
}

void polecommsthread::_realtimeStreamingLoop() {
	UpdatePoleEvents(syncEventsToServer());
}