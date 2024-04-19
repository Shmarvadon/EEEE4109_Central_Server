#include "polelist.h"


Pole::Pole(PoleDataModel* model, TreeViewHeader* parentItem, int port, int sessionId, uint64_t HWID, uint8_t type) : _sessionId(sessionId), _poleHWID(HWID), _poleType(type), _Model(model), _parentItem(parentItem) {
	
	// Set default values for stuffs.
	_selected = false;
	_polePartner = -1;
	_gateNumber = -1;


	// Set connections to PoleDataModel class.
	connect(this, &Pole::findPartnerPole, model, &PoleDataModel::findPartnerToPole);
	connect(this, &Pole::updatetreeVisual, model, &PoleDataModel::updateVisual);

	// Configure the comms thread.
	poletcpthread *worker = new poletcpthread(this, port);

	//throw std::runtime_error("");
	worker->moveToThread(&_TCPListnerThread);

	
	this->connect(worker, &poletcpthread::UpdatePoleConnectionStatus, this, &Pole::UpdatePoleConnectionStatus);
	this->connect(this, &Pole::startTCPThread, worker, &poletcpthread::run, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncEventsToServer, worker, &poletcpthread::syncEventsToServer , Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSensorReadingsToServer, worker, &poletcpthread::syncSensorReadingsToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSettingsToServer, worker, &poletcpthread::syncSettingsToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSettingsToPole, worker, &poletcpthread::syncSettingsToPole, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::StartRealtimeStream, worker, &poletcpthread::StartRealtimeStream, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::EndRealtimeStream, worker, &poletcpthread::EndRealtimeStream, Qt::BlockingQueuedConnection);
	_TCPListnerThread.start();

	startTCPThread();

	this->setProperty("height", QVariant(500));

	
	//_poleState.Events = syncEventsToServer();
}

Pole::~Pole() {

	throw std::runtime_error("");

	_TCPListnerThread.quit();
}

/*			QT functions			*/

void Pole::appendChild(Pole* child) {
	_childItems.push_back(child);
}

Pole* Pole::child(int row) {
	return row >= 0 && row < childCount() ? _childItems.at(row) : nullptr;
}

int Pole::childCount() const {
	return int(_childItems.size());
}

int Pole::row() const {
	if (_parentItem == nullptr) return 0;

	const auto it = std::find_if(_parentItem->getChildItems().cbegin(), _parentItem->getChildItems().cend(), [this](const Pole* treeItem) {return treeItem == this; });

	if (it != _parentItem->getChildItems().cend()) return std::distance(_parentItem->getChildItems().cbegin(), it);
	Q_ASSERT(false);

	return -1;
}

int Pole::columnCount() const {
	return 5;
}

QVariant Pole::data(int column) const {

	switch (column){
	case 0: return QVariant(("Pole " + std::to_string(_sessionId)).c_str());
	case 1: return QVariant((std::to_string(_poleState.Sensors.batteryReading) + (const char *)"%").c_str());
	case 2: return (_connstat == pcs::Connected) ? QVariant("Connected") : QVariant("Disconnected");
	case 3: return (_polePartner == -1) ? QVariant("No Partner") : QVariant(std::to_string(_polePartner).c_str());
	case 4: return (_gateNumber == -1) ? QVariant("No Gate") : QVariant(std::to_string(_gateNumber).c_str());
	}
}

TreeViewHeader* Pole::parentItem() {
	return _parentItem;
}


/*			My fcuntions :)			*/

void Pole::UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus) {
	_connstat = connectionStatus;
	emit updatetreeVisual();
}

void Pole::setUISelection(bool selected) {
	_selected = selected;

	// Update the boxes to read out real values.
	if (selected) {
		VisualisePoleType(QString(std::to_string(_poleType).c_str()));
		VisualisePoleHWID(QString(std::to_string(_poleHWID).c_str()));

		VisualisePolePartner(QString(std::to_string(_polePartner).c_str()));
		VisualisePoleBattery(QString(std::to_string(_poleState.Sensors.batteryReading).c_str()));

		VisualisePolePosition(QString(std::to_string(_gateNumber).c_str()));
		VisualiseIRBeamFrequency(QString(std::to_string(_poleState.Settings.IRTransmitFreq).c_str()));

		VisualiseTouchSensitivity(QString(std::to_string(_poleState.Settings.velostatSensitivity).c_str()));
		VisualiseIMUSensitivity(QString(std::to_string(_poleState.Settings.IMUSensitivity).c_str()));

		// Start the realtime syncing of events data.
		StartRealtimeStream();
	}
	else EndRealtimeStream();

}

void Pole::UpdatePoleEvents(events newEventsData) { 
	_poleState.Events = newEventsData;

	if (_selected) updatePoleEventsVisualIndicators(newEventsData);
};

void Pole::UpdatePoleSensors(sensors newSensorData) {
	_poleState.Sensors = newSensorData;

	emit updatetreeVisual();
};


