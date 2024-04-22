#include "pole.h"


void poletcpthread::run() {

	_setupTCPConnection();

	// Timer to handle keep alive of the link.
	_mainLoopTimer = new QTimer(this);
	_mainLoopTimer->setTimerType(Qt::PreciseTimer);
	connect(_mainLoopTimer, &QTimer::timeout, this, &poletcpthread::mainLoop, Qt::DirectConnection);

	_mainLoopTimer->start(1000);

	//Setup realtime stream timer.
	_realtimeStreamTimer = new QTimer(this);
	_realtimeStreamTimer->setTimerType(Qt::PreciseTimer);
	connect(_realtimeStreamTimer, &QTimer::timeout, this, &poletcpthread::_realtimeStreamingLoop, Qt::DirectConnection);
}

void poletcpthread::mainLoop() {
	// If the last transmission was more than 5000 ms ago then we ping to ask if the pole is alive.
	auto now = std::chrono::high_resolution_clock::now() - lastSynced;
	if ((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now).count() > 100) {
		_pingPole();
	}
}

void poletcpthread::_setupTCPConnection() {
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
	if (getaddrinfo(NULL, std::to_string(_TCPPort).c_str(), &hints, &result) != 0) {
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

	// Listen for connection from pole.
	if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(listeningSocket);
		WSACleanup();
		throw std::runtime_error("Listening on TCP socket failed.");
	}

	// Accept connection into to _TCPSocket.
	_TCPSocket = accept(listeningSocket, NULL, NULL);
	// Close temporary listening socket.
	closesocket(listeningSocket);

	if (_TCPSocket == INVALID_SOCKET) throw std::runtime_error("Something went wrong with TCP Socket.\n");

	// Make the socket non blocking.
	u_long blocking_mode = 1;
	ioctlsocket(_TCPSocket, FIONBIO, &blocking_mode);

	// Set now as the last time the pole has been talked to.
	lastSynced = std::chrono::high_resolution_clock::now();

	emit UpdatePoleConnectionStatus(pcs::Connected);
}

bool poletcpthread::_pingPole() {
	bool poleStillHere = false;
	char pingPacket = ptt::Ping;

	int result = send(_TCPSocket, &pingPacket, 1, 0);

	// Make the socket blocking & set timeout to 50ms.
	_setSocketBlockingMode(true, 50);

	// call recv.
	result = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);

	if (result > 0) {
		if (_TCPRecieveBuffer[0] == ptt::Pong) {

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

sensors poletcpthread::syncSensorReadingsToServer() {
	sensors newData;
	bool completedSuccessfully = false;

	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Sensors;
	int result = send(_TCPSocket, _TCPRecieveBuffer, sizeof(char) * TCP_BUFF_SIZE, 0);

	// Make the socket blocking & set timeout to 10ms.
	_setSocketBlockingMode(true, 10);

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	nBytesReceived = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_TCPRecieveBuffer[0] == ptt::SyncToServer | ptt::Sensors) {

			// memcopy the recieved data into the struct.
			memcpy(&newData.velostatReading, &_TCPRecieveBuffer[1], sizeof(float));
			memcpy(&newData.IRGateReading, &_TCPRecieveBuffer[5], sizeof(float));
			memcpy(&newData.IRCameraReading, &_TCPRecieveBuffer[9], sizeof(float));
			memcpy(&newData.IMUReading, &_TCPRecieveBuffer[13], sizeof(float));
			memcpy(&newData.batteryReading, &_TCPRecieveBuffer[17], sizeof(int));

			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// Check not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully ? newData : throw std::runtime_error("Issue with syncing sensor data to server");
}

events poletcpthread::syncEventsToServer() {
	events newData = (events)0;

	bool completedSuccessfully = false;

	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Events;
	int result = send(_TCPSocket, _TCPRecieveBuffer, sizeof(char) * TCP_BUFF_SIZE, 0);

	// Make the socket blocking & set timeout to 500ms.
	_setSocketBlockingMode(true, 1000);

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	nBytesReceived = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		completedSuccessfully = true;

		//char something = _TCPRecieveBuffer[0];

		//std::cout << (int)something << std::endl;

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_TCPRecieveBuffer[0] == ptt::SyncToServer | ptt::Events) {

			newData = (events)_TCPRecieveBuffer[1];

			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// CHeck not strictly needed but could allow for future enhanced handling.
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

settings poletcpthread::syncSettingsToServer() {
	settings newData;
	bool completedSuccessfully = false;

	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Configurables;
	int result = send(_TCPSocket, _TCPRecieveBuffer, sizeof(char) * TCP_BUFF_SIZE, 0);

	// Make the socket blocking & set timeout to 10ms.
	_setSocketBlockingMode(true, 10);

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	nBytesReceived = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_TCPRecieveBuffer[0] == ptt::SyncToServer | ptt::Configurables) {

			// memcpy the data into the struct.
			memcpy(&newData.IRTransmitFreq, &_TCPRecieveBuffer[1], sizeof(uint16_t));
			memcpy(&newData.IMUSensitivity, &_TCPRecieveBuffer[3], sizeof(float));
			memcpy(&newData.velostatSensitivity, &_TCPRecieveBuffer[7], sizeof(float));
			memcpy(&newData.powerState, &_TCPRecieveBuffer[11], sizeof(uint8_t));


			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// CHeck not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully ? newData : throw std::runtime_error("Issue with syncing settings data to server");
}

void poletcpthread::_setSocketBlockingMode(bool blocking, int timeout) {

	if (blocking) {
		// Make the socket blocking.
		u_long blocking_mode = 0;
		ioctlsocket(_TCPSocket, FIONBIO, &blocking_mode);

		// Set the timeout for blocking operations in milliseconds.
		int result = setsockopt(_TCPSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	}

	if (!blocking) {
		// Make the socket not blocking.
		u_long blocking_mode = 1;
		ioctlsocket(_TCPSocket, FIONBIO, &blocking_mode);
	}
}

bool poletcpthread::syncSettingsToPole(settings Settings) {
	bool completedSuccessfully = false;

	// Zero the buffer & pack data into it for transmission.
	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToPole | ptt::Configurables;
	memcpy(&_TCPRecieveBuffer[1], &Settings.IRTransmitFreq, sizeof(uint16_t));
	memcpy(&_TCPRecieveBuffer[3], &Settings.IMUSensitivity, sizeof(float));
	memcpy(&_TCPRecieveBuffer[7], &Settings.velostatSensitivity, sizeof(float));
	memcpy(&_TCPRecieveBuffer[11], &Settings.powerState, sizeof(uint8_t));

	// Transmit the data to the pole.
	int result = send(_TCPSocket, _TCPRecieveBuffer, sizeof(char) * TCP_BUFF_SIZE, 0);

	// Make the socket blocking & set timeout to 100ms.
	_setSocketBlockingMode(true, 100);

	// Recieve the reply from the server.
	int nBytesReceived(0);
	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	nBytesReceived = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);	// Blocking mode should be enabled.

	// If the pole has phoned home, process the data it has sent.
	if (nBytesReceived > 0) {

		// Set now as the last time the pole has been talked to.
		lastSynced = std::chrono::high_resolution_clock::now();

		// If the reply is a Pong then the server did well.
		if (_TCPRecieveBuffer[0] == ptt::Pong) {
			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// Check not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully;
}

void poletcpthread::_realtimeStreamingLoop() {
	UpdatePoleEvents(syncEventsToServer());
}