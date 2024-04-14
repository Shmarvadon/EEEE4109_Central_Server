//#include "pole.h"
#include "polelist.h"

Pole::Pole(PoleDataModel* model, QVariantList data, Pole* parent) : _itemData(std::move(data)), _parentItem(parent), _Model(model) {}

Pole::Pole(PoleDataModel* model, Pole* parentItem, int port, int sessionId, uint64_t HWID, uint8_t type) : _sessionId(sessionId), _poleHWID(HWID), _poleType(type), _Model(model) {
	
	// Configure visual information in the tree view.
	_itemData.resize(5);

	_itemData[0] = ("Pole " + std::to_string(sessionId)).c_str();
	// Configure the comms thread.

	_TCPListnerThread = new poletcpthread(this, port);

	this->connect(_TCPListnerThread, &poletcpthread::UpdatePoleConnectionStatus, this, &Pole::UpdatePoleConnectionStatus);
	this->connect(this, &Pole::updateVisual, model, &PoleDataModel::updateVisual);

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


void poletcpthread::run() {

	_setupTCPConnection();
	
	// Infinite loop.
	while (true) {

		// Run any queued events on the thread.
		//this->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlags::fromInt(0));

		// If the thread is told to exit or interrupted.
		//if (this->currentThread()->isInterruptionRequested()) {

		//}

		//std::string temp = "Hello there pole.";
		//int result = send(_TCPSocket, temp.c_str(), temp.length(), 0);


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
		if ((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now).count() > 5000) {
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
	char pingPacket = Ping;

	int result = send(_TCPSocket, &pingPacket, 1, 0);

	// Make the socket blocking.
	u_long blocking_mode = 0;
	ioctlsocket(_TCPSocket, FIONBIO, &blocking_mode);

	// Set the timeout for blocking operations in milliseconds.
	int timeout = 1000;
	result = setsockopt(_TCPSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	// call recv.
	result = recv(_TCPSocket, _TCPRecieveBuffer, TCP_BUFF_SIZE, 0);

	if (result > 0) {
		if (_TCPRecieveBuffer[0] == Pong) {

			lastSynced = std::chrono::high_resolution_clock::now();
			poleStillHere = true;
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
	blocking_mode = 1;
	ioctlsocket(_TCPSocket, FIONBIO, &blocking_mode);

	return poleStillHere;
}

void Pole::UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus) {

	//throw std::runtime_error("");
	if (connectionStatus == pcs::Connected) _itemData[2] = QVariant("Connected");
	if (connectionStatus == pcs::Disconnected) _itemData[2] = QVariant("Disconnected");
	if (connectionStatus == pcs::Unknown) _itemData[2] = QVariant("Unknown");
	if (connectionStatus == pcs::Reconnecting) _itemData[2] = QVariant("Reconnecting");
	emit updateVisual();
}