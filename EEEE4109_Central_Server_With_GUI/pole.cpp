//#include "pole.h"
#include "polelist.h"

Pole::Pole(PoleDataModel* model, QVariantList data, Pole* parent) : _itemData(std::move(data)), _parentItem(parent), _Model(model) {}

Pole::Pole(PoleDataModel* model, Pole* parentItem, int port, int sessionId, uint64_t HWID, uint8_t type) : _sessionId(sessionId), _poleHWID(HWID), _poleType(type), _Model(model) {
	
	// Set default values for stuffs.
	_selected = false;

	// Configure visual information in the tree view.
	_itemData.resize(5);
	_itemData[0] = ("Pole " + std::to_string(sessionId)).c_str();


	// Configure the comms thread.
	_TCPListnerThread = new poletcpthread(this, port);
	this->connect(_TCPListnerThread, &poletcpthread::UpdatePoleConnectionStatus, this, &Pole::UpdatePoleConnectionStatus);
	this->connect(this, &Pole::updateVisual, model, &PoleDataModel::updateVisual);
	this->connect(this, &Pole::SyncPoleEventsDataToServer, _TCPListnerThread, &poletcpthread::SyncPoleEventsDataToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::SyncPoleSensorsDataToServer, _TCPListnerThread, &poletcpthread::SyncPoleSensorsDataToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::SyncPoleSettingsDataToServer, _TCPListnerThread, &poletcpthread::SyncPoleSettingsDataToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::SyncPoleSettingsDataToPole, _TCPListnerThread, &poletcpthread::SyncPoleSettingsDataToPole, Qt::BlockingQueuedConnection);
	_TCPListnerThread->start();
}

Pole::~Pole() {

	throw std::runtime_error("");

	_TCPListnerThread->quit();
}

/*			QT functions			*/

void Pole::appendChild(Pole* child) {
	_childItems.push_back(child);
	child->_parentItem = this;
}

Pole* Pole::child(int row) {
	return row >= 0 && row < childCount() ? _childItems.at(row) : nullptr;
}

int Pole::childCount() const {
	return int(_childItems.size());
}

int Pole::row() const {
	if (_parentItem == nullptr) return 0;

	const auto it = std::find_if(_parentItem->_childItems.cbegin(), _parentItem->_childItems.cend(), [this](const Pole* treeItem) {return treeItem == this; });

	if (it != _parentItem->_childItems.cend()) return std::distance(_parentItem->_childItems.cbegin(), it);
	Q_ASSERT(false);

	return -1;
}

int Pole::columnCount() const {
	return int(_itemData.count());
}

QVariant Pole::data(int column) const {
	return _itemData.value(column);
}

Pole* Pole::parentItem() {
	return _parentItem;
}


/*			My fcuntions :)			*/

void Pole::UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus) {

	//throw std::runtime_error("");
	if (connectionStatus & pcs::Connected) _itemData[2] = QVariant("Connected");
	if (connectionStatus & pcs::Disconnected) _itemData[2] = QVariant("Disconnected");
	if (connectionStatus & pcs::Unknown) _itemData[2] = QVariant("Unknown");
	if (connectionStatus & pcs::Reconnecting) _itemData[2] = QVariant("Reconnecting");
	emit updateVisual();
}

void Pole::setUISelection(bool selected) {
	_selected = selected;

	// Update the boxes to read out real values.
	if (selected == true) {
		VisualisePoleType(QString(std::to_string(_poleType).c_str()));
		VisualisePoleHWID(QString(std::to_string(_poleHWID).c_str()));

		VisualisePolePartner(QString(std::to_string(_polePartner).c_str()));
		VisualisePoleBattery(QString(std::to_string(_poleState.Sensors.batteryReading).c_str()));

		VisualisePolePosition(QString(std::to_string(_gateNumber).c_str()));
		VisualiseIRBeamFrequency(QString(std::to_string(_poleState.Settings.IRTransmitFreq).c_str()));

		VisualiseTouchSensitivity(QString(std::to_string(_poleState.Settings.velostatSensitivity).c_str()));
		VisualiseIMUSensitivity(QString(std::to_string(_poleState.Settings.IMUSensitivity).c_str()));
	}

}



void poletcpthread::run() {

	_setupTCPConnection();
	
	// Infinite loop.
	while (true) {

		// If we are now doing realtime pole to server streaming of realtime data.
		if (_RealtimeStreamEnabled) {}

		// Run any queued events on the thread.
		//this->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlags::fromInt(0));

		// If the thread is told to exit or interrupted.
		//if (this->currentThread()->isInterruptionRequested()) {

		//}
		

		//ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
		//_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Events;
		//int result = send(_TCPSocket, _TCPRecieveBuffer, sizeof(char) * TCP_BUFF_SIZE, 0);


		// Run the bit we want to run.
		int nBytesReceived(0);
		ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
		nBytesReceived = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);	// Non blocking mode should be enabled.

		// If the pole has phoned home, process the data it has sent.
		if (nBytesReceived > 0) {

			// Set now as the last time the pole has been talked to.
			lastSynced = std::chrono::high_resolution_clock::now();

		}

		// If the last transmission was more than 5000 ms ago then we ping to ask if the pole is alive.
		auto now = std::chrono::high_resolution_clock::now() - lastSynced;
		if ((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now).count() > 500) {
			if (!_pingPole()) break;
		}
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

	// Make the socket blocking & set timeout to 5000ms.
	_setSocketBlockingMode(true, 1000);

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

bool poletcpthread::SyncPoleSensorsDataToServer(sensors* pSensorData) {
	bool completedSuccessfully = false;

	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Sensors;
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

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_TCPRecieveBuffer[0] == ptt::SyncToServer | ptt::Sensors) {

			// memcopy the recieved data into the struct.
			memcpy(&pSensorData->velostatReading,	&_TCPRecieveBuffer[1],	sizeof(float));
			memcpy(&pSensorData->IRGateReading,		&_TCPRecieveBuffer[5],	sizeof(float));
			memcpy(&pSensorData->IRCameraReading,	&_TCPRecieveBuffer[9],	sizeof(float));
			memcpy(&pSensorData->IMUReading,		&_TCPRecieveBuffer[13],	sizeof(float));
			memcpy(&pSensorData->batteryReading,	&_TCPRecieveBuffer[17],	sizeof(float));

			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// Check not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully;
}

bool poletcpthread::SyncPoleEventsDataToServer(events* pEvents) {
	bool completedSuccessfully = false;

	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Events;
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

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_TCPRecieveBuffer[0] == ptt::SyncToServer | ptt::Events) {

			pEvents->IRBeamTriggered = _TCPRecieveBuffer[1];
			pEvents->knocked = _TCPRecieveBuffer[2];
			pEvents->velostatTriggered = _TCPRecieveBuffer[3];
			pEvents->IMUTriggered = _TCPRecieveBuffer[4];
			pEvents->IRCameraTriggered = _TCPRecieveBuffer[5];
			pEvents->kayakerPassingDirection = _TCPRecieveBuffer[6];

			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// CHeck not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully;
}

bool poletcpthread::SyncPoleSettingsDataToServer(settings* pSettings) {
	bool completedSuccessfully = false;

	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToServer | ptt::Configurables;
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

		// If the reply is a sync to server reply & it is to update events then we deserialise it into the updated events struct.
		if (_TCPRecieveBuffer[0] == ptt::SyncToServer | ptt::Configurables) {

			// memcpy the data into the struct.
			memcpy(&pSettings->IRTransmitFreq, &_TCPRecieveBuffer[1], sizeof(uint16_t));
			memcpy(&pSettings->IMUSensitivity, &_TCPRecieveBuffer[3], sizeof(float));
			memcpy(&pSettings->velostatSensitivity, &_TCPRecieveBuffer[7], sizeof(float));
			memcpy(&pSettings->powerState, &_TCPRecieveBuffer[11], sizeof(uint8_t));


			completedSuccessfully = true;
		}

		if (_TCPRecieveBuffer[0] == ptt::SomethingWentWrong) completedSuccessfully = false;	// CHeck not strictly needed but could allow for future enhanced handling.
	}

	// Make the socket not blocking.
	_setSocketBlockingMode(false, 0);

	return completedSuccessfully;
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

bool poletcpthread::SyncPoleSettingsDataToPole(settings* pSettings) {
	bool completedSuccessfully = false;

	// Zero the buffer & pack data into it for transmission.
	ZeroMemory(_TCPRecieveBuffer, TCP_BUFF_SIZE);
	_TCPRecieveBuffer[0] = ptt::SyncToPole | ptt::Configurables;
	memcpy(&_TCPRecieveBuffer[1], &pSettings->IRTransmitFreq, sizeof(uint16_t));
	memcpy(&_TCPRecieveBuffer[3], &pSettings->IMUSensitivity, sizeof(float));
	memcpy(&_TCPRecieveBuffer[7], &pSettings->velostatSensitivity, sizeof(float));
	memcpy(&_TCPRecieveBuffer[11], &pSettings->powerState, sizeof(uint8_t));

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