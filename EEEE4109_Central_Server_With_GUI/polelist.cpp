#include "polelist.h"

PoleDataModel::PoleDataModel(int UDPListnerPort, std::pair<uint32_t, uint32_t> TCPPortRange) : _rootItem(new Pole(this, QVariantList{ tr("Pole Name"), tr("Battery"), tr("Status"), tr("Partner"), tr("Gate Number") })) {
	
	// Setup the UDP listner thread.
	_UDPListnerThread = new udplistnerthread(this, this, _rootItem, UDPListnerPort, TCPPortRange);
	connect(_UDPListnerThread, &udplistnerthread::appendNewPole, this, &PoleDataModel::appendNewPole);
	_UDPListnerThread->start();
}


PoleDataModel::~PoleDataModel() {

	_UDPListnerThread->quit();
}

QModelIndex PoleDataModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) return {};

	Pole* parentItem = parent.isValid() ? static_cast<Pole*>(parent.internalPointer()) : _rootItem;

	if (auto* childItem = parentItem->child(row)) return createIndex(row, column, childItem);
	return {};
}

QModelIndex PoleDataModel::parent(const QModelIndex& index) const {
	if (!index.isValid()) return {};

	auto* ChildItem = static_cast<Pole*>(index.internalPointer());
	Pole* parentItem = ChildItem->parentItem();

	return parentItem != _rootItem ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

int PoleDataModel::rowCount(const QModelIndex& parent) const {
	if (parent.column() > 0) return 0;

	const Pole* parentItem = parent.isValid() ? static_cast<const Pole*>(parent.internalPointer()) : _rootItem;

	return parentItem->childCount();
}

int PoleDataModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid()) return static_cast<Pole*>(parent.internalPointer())->columnCount();

	return _rootItem->columnCount();
}

QVariant PoleDataModel::data(const QModelIndex& index, int role) const {
	
	if (role == Qt::TextAlignmentRole) return Qt::AlignHCenter;

	if (role == Qt::DisplayRole) return static_cast<Pole*>(index.internalPointer())->data(index.column());


	if (!index.isValid() || role != Qt::DisplayRole) return {};

	return static_cast<Pole*>(index.internalPointer())->data(index.column());
}

Qt::ItemFlags PoleDataModel::flags(const QModelIndex& index) const {
	return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant PoleDataModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if (role == Qt::TextAlignmentRole) return Qt::AlignHCenter;
	return orientation == Qt::Horizontal && role == Qt::DisplayRole ? _rootItem->data(section) : QVariant{};
}

Pole* PoleDataModel::getRootItem() {
	return _rootItem;
}

void PoleDataModel::appendNewPole(int port, int sessionId, uint64_t HWID, uint8_t type){

	// Create the new pole.
	Pole* pole = new Pole(this, _rootItem, port, sessionId, HWID, type);

	// Update the model.
	beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());

	// Place pointer to pole class into vector, it CAN NOT be the pole itself as SOCKET & std::thread dont play nicely with move or copying and I CBA to put in the effort to properly design the class.
	_poles.push_back(pole);
	_rootItem->appendChild(pole);

	endInsertRows();

	//emit dataChanged(index(0, 0), index(rowCount(), columnCount()), { Qt::DecorationRole });
}



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

		//  Recieve the broadcast.

		uint32_t polePort = (_TCPPortsRange.first + _NumberOfPolesConncted < _TCPPortsRange.second) ? _TCPPortsRange.first + _NumberOfPolesConncted : throw std::runtime_error("Ran out of valid TCP ports to use.");

		// Obtain a session ID for the pole.
		uint32_t poleSessionId = _NumberOfPolesConncted;

		// Obtain the poles HWID.
		uint64_t poleHWID;
		memcpy(&poleHWID, &listnerBuff[0], sizeof(uint64_t));

		// Obtain the poles type.
		uint8_t poleType = listnerBuff[8];

		// Pack reply data into buffer.
		memcpy(senderBuff, &polePort, sizeof(uint32_t));
		memcpy(&senderBuff[4], &poleSessionId, sizeof(uint32_t));

		// Emit a signal to queue up the operation of appending the new pole to the _poles vector in the data model.
		emit appendNewPole(polePort, poleSessionId, poleHWID, poleType);

		// Send the reply.
		senderRecieverAddress.sin_addr.s_addr = listnerSenderAddress.sin_addr.S_un.S_addr;
		if (WSASendTo(UDPSender, &senderDataBuff, 1, &senderBytesSent, senderFlags, (SOCKADDR*)&senderRecieverAddress, sizeof(senderRecieverAddress), NULL, NULL) != 0) {
			closesocket(UDPSender);
			WSACleanup();
			throw std::runtime_error("Unable to send data over UDP socket.");
		}

		_NumberOfPolesConncted++;
	}

	/*   Thread termination cleanup   */
	// Hahahahaha this does not run :(

	// close the sockets.
	closesocket(UDPListener);
	closesocket(UDPSender);
}