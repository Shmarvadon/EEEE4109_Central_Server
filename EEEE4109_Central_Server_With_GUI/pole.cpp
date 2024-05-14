#include "polelist.h"

#include "gate.h"

Pole::Pole(PoleDataModel* model, TreeViewHeader* parentItem, sockaddr_in poleAddress, int port, int sessionId, uint64_t HWID, uint8_t type) : _sessionId(sessionId), _poleHWID(HWID), _poleType(type), _Model(model), _parentItem(parentItem) {
	
	// Set default values for stuffs.
	_selected = false;
	_Gate = nullptr;


	// Set connections to PoleDataModel class.
	connect(this, &Pole::findPartnerPole, model, &PoleDataModel::findPartnerToPole);
	connect(this, &Pole::updatetreeVisual, model, &PoleDataModel::updateVisual);

	// Configure the comms thread.
	polecommsthread *worker = new polecommsthread(this, poleAddress, port);

	worker->moveToThread(&_TCPListnerThread);
	
	this->connect(worker, &polecommsthread::UpdatePoleConnectionStatus, this, &Pole::UpdatePoleConnectionStatus);
	this->connect(worker, &polecommsthread::UpdatePoleEvents, this, &Pole::UpdatePoleEvents);
	this->connect(this, &Pole::startTCPThread, worker, &polecommsthread::run, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncEventsToServer, worker, &polecommsthread::syncEventsToServer , Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSensorReadingsToServer, worker, &polecommsthread::syncSensorReadingsToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSettingsToServer, worker, &polecommsthread::syncSettingsToServer, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::syncSettingsToPole, worker, &polecommsthread::syncSettingsToPole, Qt::BlockingQueuedConnection);
	this->connect(this, &Pole::StartRealtimeStream, worker, &polecommsthread::StartRealtimeStream, Qt::QueuedConnection);
	this->connect(this, &Pole::EndRealtimeStream, worker, &polecommsthread::EndRealtimeStream, Qt::QueuedConnection);
	this->connect(this, &Pole::getSocketPort, worker, &polecommsthread::getSocketPort, Qt::BlockingQueuedConnection);

	_TCPListnerThread.start(QThread::HighestPriority);

	startTCPThread();
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
	case 2: return (_connstat == pcs::Connected) ? QVariant("Connected") : (_connstat == pcs::Reconnecting) ? QVariant("Reconnecting") : (_connstat == pcs::Disconnected) ? QVariant("Disconnected") : QVariant("Connecting");
	case 4: return (_Gate == nullptr) ? QVariant("No Gate") : QVariant(std::to_string(_Gate->getGatePosition()).c_str());
	case 3: return (_Gate == nullptr) ? QVariant("None") : (_Gate->getPartnerPole(this) == nullptr) ? QVariant("None") : QVariant(std::to_string(_Gate->getPartnerPole(this)->getPoleSessionID()).c_str());
	}
}

TreeViewHeader* Pole::parentItem() {
	return _parentItem;
}


/*			My fcuntions :)			*/

void Pole::UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus) {
	_connstat = connectionStatus;

	if (_connstat & pcs::Reconnecting) {
		emit syncSettingsToPole(_poleState.Settings);
		if (_selected) StartRealtimeStream();

		_connstat = pcs::Connected;
	}

	if (_connstat == pcs::Disconnected && _selected) {
		grayOutPoleControls(true);
	}

	if (_connstat == pcs::Connected && _selected) {
		grayOutPoleControls(false);
	}
	emit updatetreeVisual();
}

void Pole::setUISelection(bool selected) {
	_selected = selected;

	// Update the boxes to read out real values.
	if (selected) {
		(_poleType == PhotoDiodePole) ? VisualisePoleType(QString("Left")) : VisualisePoleType(QString("Right"));
		VisualisePoleHWID(QString(std::to_string(_poleHWID).c_str()));

		(_Gate == nullptr) ? VisualisePolePartner(QString("None")) : (_Gate->getPartnerPole(this) == nullptr) ? VisualisePolePartner(QString("None")) : VisualisePolePartner(QString(std::to_string(_Gate->getPartnerPole(this)->getPoleSessionID()).c_str()));
		VisualisePoleBattery(QString(std::to_string(_poleState.Sensors.batteryReading).c_str()));

		(_Gate == nullptr) ? VisualisePoleGateNumber(QString("None")) : VisualisePoleGateNumber(QString(std::to_string(_Gate->getGatePosition()).c_str()));
		VisualiseIRBeamFrequency(QString(std::to_string(_poleState.Settings.IRTransmitFreq).c_str()));

		VisualiseTouchSensitivity(QString(std::to_string((int)_poleState.Settings.velostatSensitivity*100).c_str()));
		VisualiseIMUSensitivity(QString(std::to_string((int)_poleState.Settings.IMUSensitivity * 100).c_str()));

		if (_connstat != pcs::Connected) grayOutPoleControls(true);
		else grayOutPoleControls(false);

		// Start the realtime syncing of events data.
		StartRealtimeStream();
	}
	else {EndRealtimeStream();}
}

void Pole::UpdatePoleEvents(events newEventsData) { 
	_poleState.Events = newEventsData;

	if (_selected) updatePoleEventsVisualIndicators(newEventsData);
};

void Pole::UpdatePoleSensors(sensors newSensorData) {
	_poleState.Sensors = newSensorData;

	// Update the UI to reflect the new readings if selected.
	if (_selected) VisualisePoleBattery(QString(std::to_string(_poleState.Sensors.batteryReading).c_str()));

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
}

void Pole::setIMUSensitivity(float newSensitivity) {
	_poleState.Settings.IMUSensitivity = newSensitivity;

	syncSettingsToPole(_poleState.Settings);
}