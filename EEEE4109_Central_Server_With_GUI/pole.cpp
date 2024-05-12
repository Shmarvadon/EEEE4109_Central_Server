#include "polelist.h"

#include "gate.h"

Pole::Pole(PoleDataModel* model, TreeViewHeader* parentItem, int port, int sessionId, uint64_t HWID, uint8_t type) : _sessionId(sessionId), _poleHWID(HWID), _poleType(type), _Model(model), _parentItem(parentItem) {
	
	// Set default values for stuffs.
	_selected = false;
	_Gate = nullptr;


	// Set connections to PoleDataModel class.
	connect(this, &Pole::findPartnerPole, model, &PoleDataModel::findPartnerToPole);
	connect(this, &Pole::updatetreeVisual, model, &PoleDataModel::updateVisual);

	// Configure the comms thread.
	poletcpthread *worker = new poletcpthread(this, port);

	//throw std::runtime_error("");
	worker->moveToThread(&_TCPListnerThread);

	
	this->connect(worker, &poletcpthread::UpdatePoleConnectionStatus, this, &Pole::UpdatePoleConnectionStatus);
	this->connect(worker, &poletcpthread::UpdatePoleEvents, this, &Pole::UpdatePoleEvents);
	this->connect(this, &Pole::startTCPThread, worker, &poletcpthread::run, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncEventsToServer, worker, &poletcpthread::syncEventsToServer , Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSensorReadingsToServer, worker, &poletcpthread::syncSensorReadingsToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSettingsToServer, worker, &poletcpthread::syncSettingsToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSettingsToPole, worker, &poletcpthread::syncSettingsToPole, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::StartRealtimeStream, worker, &poletcpthread::StartRealtimeStream, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::EndRealtimeStream, worker, &poletcpthread::EndRealtimeStream, Qt::BlockingQueuedConnection);
	_TCPListnerThread.start(QThread::HighestPriority);

	startTCPThread();

	//this->setProperty("height", QVariant(500));

	
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

	switch (column) {
	case 0: return QVariant(("Pole " + std::to_string(_sessionId)).c_str());
	case 1: return QVariant((std::to_string(_poleState.Sensors.batteryReading) + (const char*)"%").c_str());
	case 2: return (_connstat == pcs::Connected) ? QVariant("Connected") : QVariant("Disconnected");
	case 4: return (_Gate == nullptr) ? QVariant("No Gate") : QVariant(std::to_string(_Gate->getGatePosition()).c_str());
	case 3: if (_Gate != nullptr) return (_Gate->getPartnerPole(this) == nullptr) ? QVariant("None") : QVariant(std::to_string(_Gate->getPartnerPole(this)->getPoleSessionID()).c_str());
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
		(_poleType == PhotoDiodePole) ? VisualisePoleType(QString("Left")) : VisualisePoleType(QString("Right"));
		VisualisePoleHWID(QString(std::to_string(_poleHWID).c_str()));

		if (_Gate != nullptr) (_Gate->getPartnerPole(this) == nullptr) ? VisualisePolePartner(QString("None")) : VisualisePolePartner(QString(std::to_string(_Gate->getPartnerPole(this)->getPoleSessionID()).c_str()));
		VisualisePoleBattery(QString(std::to_string(_poleState.Sensors.batteryReading).c_str()));

		(_Gate == nullptr) ? VisualisePoleGateNumber(QString("None")) : VisualisePoleGateNumber(QString(std::to_string(_Gate->getGatePosition()).c_str()));
		VisualiseIRBeamFrequency(QString(std::to_string(_poleState.Settings.IRTransmitFreq).c_str()));

		VisualiseTouchSensitivity(QString(std::to_string((int)_poleState.Settings.velostatSensitivity*100).c_str()));
		VisualiseIMUSensitivity(QString(std::to_string((int)_poleState.Settings.IMUSensitivity * 100).c_str()));

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

void Pole::setIRFrequency(uint16_t newFreq) {
	_poleState.Settings.IRTransmitFreq = newFreq;

	syncSettingsToPole(_poleState.Settings);

	// If this is the selected pole then tell the other pole to also adjust its frequency.
	if (_selected && _Gate != nullptr) { if (_Gate->getPartnerPole(this) != nullptr) _Gate->getPartnerPole(this)->setIRFrequency(newFreq); }
}

void Pole::setVelostatSensitivity(float newSensitivity) {
	_poleState.Settings.velostatSensitivity = newSensitivity;

	syncSettingsToPole(_poleState.Settings);

	// If this is the selected pole then tell the other pole to also adjust its Sensitivity.
	if (_selected && _Gate != nullptr) { if (_Gate->getPartnerPole(this) != nullptr) _Gate->getPartnerPole(this)->setVelostatSensitivity(newSensitivity); }
}

void Pole::setIMUSensitivity(float newSensitivity) {
	_poleState.Settings.IMUSensitivity = newSensitivity;

	syncSettingsToPole(_poleState.Settings);

	// If this is the selected pole then tell the other pole to also adjust its Sensitivity.
	if (_selected && _Gate != nullptr) { if (_Gate->getPartnerPole(this) != nullptr) _Gate->getPartnerPole(this)->setIMUSensitivity(newSensitivity); }
}