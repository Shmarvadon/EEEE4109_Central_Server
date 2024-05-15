#include "polelist.h"
#include "gate.h"

PoleDataModel::PoleDataModel() : _rootItem(new TreeViewHeader(this, QVariantList{ tr("Pole Name"), tr("Battery"), tr("Status"), tr("Partner"), tr("Gate Number") })) {}


PoleDataModel::~PoleDataModel() {
}

QModelIndex PoleDataModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) return {};

	TreeViewHeader* parentItem = parent.isValid() ? static_cast<TreeViewHeader*>(parent.internalPointer()) : _rootItem;

	if (auto* childItem = parentItem->child(row)) return createIndex(row, column, childItem);
	return {};
}

QModelIndex PoleDataModel::parent(const QModelIndex& index) const {
	if (!index.isValid()) return {};
	return createIndex(0, 0, _rootItem);
}

int PoleDataModel::rowCount(const QModelIndex& parent) const {
	if (parent.column() > 0) return 0;

	const TreeViewHeader* parentItem = parent.isValid() ? static_cast<const TreeViewHeader*>(parent.internalPointer()) : _rootItem;

	return parentItem->childCount();
}

int PoleDataModel::columnCount(const QModelIndex& parent) const {
	return _rootItem->columnCount();
}

QVariant PoleDataModel::data(const QModelIndex& index, int role) const {
	
	if (role == Qt::TextAlignmentRole) return Qt::AlignHCenter;

	if (role == Qt::DisplayRole) {
		return static_cast<Pole*>(index.internalPointer())->data(index.column());
	}


	if (!index.isValid() || role != Qt::DisplayRole) return {};
}

Qt::ItemFlags PoleDataModel::flags(const QModelIndex& index) const {
	return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant PoleDataModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if (role == Qt::TextAlignmentRole) return Qt::AlignHCenter;
	return orientation == Qt::Horizontal && role == Qt::DisplayRole ? _rootItem->data(section) : QVariant{};
}

TreeViewHeader* PoleDataModel::getRootItem() {
	return _rootItem;
}

void PoleDataModel::appendNewPole(sockaddr_in poleAddress, int port, uint64_t HWID, uint8_t type){


	// Create the new pole.
	Pole* pole = new Pole(this, _rootItem, poleAddress, port, _poles.size(), HWID, type);

	// Update the model.
	beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());

	// Place pointer to pole class into vector, it CAN NOT be the pole itself as SOCKET & std::thread dont play nicely with move or copying and I CBA to put in the effort to properly design the class.
	_poles.push_back(pole);
	_rootItem->appendChild(pole);

	endInsertRows();

	//emit dataChanged(index(0, 0), index(rowCount(), columnCount()), { Qt::DecorationRole });
}

Pole* PoleDataModel::getPoleByHWID(uint64_t HWID) {
	for (auto pole : _poles) {
		if (pole->getPoleHWID() == HWID) return pole;
	}

	return nullptr;
}

bool PoleDataModel::findPartnerToPole(Pole* pPole) {

	// If LED pole.
	if (pPole->getPoleType() == LEDPole) {

		// Configure the LED pole to broadcast @15khz.
		pPole->getPoleState()->Settings.powerState |= pps::HighPower;
		pPole->getPoleState()->Settings.IRTransmitFreq = 15000;

		// Sync the configuration to the pole.
		pPole->syncSettingsToPole(pPole->getPoleState()->Settings);

		// Iterate over all Photodiode poles that do not have a partner yet.
		for (auto* pole : _poles) {
			if (pole->getPoleType() == PhotoDiodePole && pole->getPolePartner() == nullptr) {
				polestate* pPoleState = pole->getPoleState();

				// Setup this pole to recieve the transmission.
				pPoleState->Settings.powerState |= pps::HighPower;	// Turn the IR beam on.

				// Sync the new configuration to the pole.
				pole->syncSettingsToPole(pPoleState->Settings);

				// Retrieve the IR beam freq that the pole is seeing now.
				pPoleState->Sensors = pole->syncSensorReadingsToServer();

				// If the pole detects the IR beam from the LED pole.
				if (pPoleState->Sensors.IRGateReading >= 14500 && pPoleState->Sensors.IRGateReading <= 15500) {

					// Assign the poles as eachothers partner.
					_Gates.push_back(new Gate((Pole*)pole, (Pole*)pPole, &_Gates));

					// Turn off the IR LEDs on the LED pole.
					pPole->getPoleState()->Settings.powerState ^= pps::HighPower;
					pPole->syncSettingsToPole(pPole->getPoleState()->Settings);

					// Update the visual in the treeview.
					updateVisual();

					return true;
				}
			}
		}

	}
	// If Photodiode pole.
	if (pPole->getPoleType() == PhotoDiodePole) {

		//Configure the pole to be ready to read its IR photodiode sensor.
		pPole->getPoleState()->Settings.powerState |= pps::HighPower;
		pPole->syncSettingsToPole(pPole->getPoleState()->Settings);

		// Iterate over all poles that are of LED type and do not have a partner pole assigned to them yet.
		for (auto& pole : _poles) {
			if (pole->getPoleType() == LEDPole && pole->getPolePartner() == nullptr) {
				polestate* pPoleState = pole->getPoleState();

				pPoleState->Settings.IRTransmitFreq = 15000;	// Set frequency to 15khz.
				pPoleState->Settings.powerState |= pps::HighPower;	// Turn the IR beam on.

				// This makes the pole broadcast at this freq.
				pole->syncSettingsToPole(pPoleState->Settings);

				// Retrieve the pole thats searching for a partners updated sensor values.
				pPole->getPoleState()->Sensors = pPole->syncSensorReadingsToServer();

				// Turn off the IR beam on the LED pole now that we are done measuring the burst.
				pPoleState->Settings.powerState ^= pps::MediumPower;
				pole->syncSettingsToPole(pPoleState->Settings);

				// If it detects the freq then we have found a match.
				if (pPole->getPoleState()->Sensors.IRGateReading >= 14500 && pPole->getPoleState()->Sensors.IRGateReading <= 15500) {

					// Assign the poles as eachothers partner.
					_Gates.push_back(new Gate((Pole*)pole, (Pole*)pPole, &_Gates));

					// Update the visual in the treeview.
					updateVisual();

					return true;
				}

				// If we do not find the poles signal (I.E. It is probably not the partner pole) we move on to the next pole.
			}
		}
	}




	return false;
}
