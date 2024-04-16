#pragma once
#include <QtCore/QVariant>
#include <qapplication.h>
#include <qitemdelegate.h>

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <vector>
#include <qthread.h>
#include <qabstracteventdispatcher.h>
#include <qabstractitemmodel.h>
#include <qobject.h>

#define TCP_BUFF_SIZE 128

class PoleDataModel;
class Pole;

// Pole Transmission Type.
namespace ptt {
	enum PoleTransmissionType : char {

		/*			Normal data transmission enums	(High bit cleared)				*/	

		SyncToPole				=	0b00000001,
		SyncToServer			=	0b00000010,

		Configurables			=	0b00000100,
		Events					=	0b00001000,
		Sensors					=	0b00010000,

		/*			Ping, keep alive, reconnect & other	enums	(High bit set)			*/

		Ping					=	0b10000001,	// Byte to send to check if pole is alive.
		Pong					=	0b10000010,	// Expected response from the pole.

		StartEventFeed			=	0b10000100,	// Starts the realtime continuous streaming of scoring relevent data.
		StopEventFeed			=	0b10001000,	// Stops  the realtime continuous streaming of scoring relevent data.

		SomethingWentWrong		=	0b10010000
	};
}
 
namespace pcs {
	enum PoleConnectionStatus : uint8_t {
		Connected		=	0b00000001,
		Disconnected	=	0b00000010,
		Reconnecting	=	0b00000100,
		Unknown			=	0b00001000
	};
}

enum PoleType : uint8_t {
	LEDPole				=	0b00000001,
	PhotoDiodePole		=	0b00000010
};


struct sensors {
	// Sensor Readings.
	float	velostatReading;	// Velostat resistance.
	float	IRGateReading;		// IR gate detected freq.
	float	IRCameraReading;	// IR camera motion vector.
	float	IMUReading;			// IMU acceleration reading.
	float	batteryReading;		// Battery voltage reading.
};

struct events {
	// Events.
	bool IRBeamTriggered;
	bool knocked;
	bool velostatTriggered;
	bool IMUTriggered;
	bool IRCameraTriggered;
	uint8_t kayakerPassingDirection;
};

struct settings {
	// Configurables.
	uint16_t IRTransmitFreq;	// Frequency of IR LEDs.
	float IMUSensitivity;		// IMU sensitivity.
	float velostatSensitivity;	// Velostat sensitivity.
	uint8_t powerState;			// Set the power state of the pole.
};

struct polestate {
	settings	Settings;
	events		Events;
	sensors		Sensors;
};

class poletcpthread : public QThread {
	Q_OBJECT

public:
	explicit poletcpthread(QObject* parent, int TCPPort) : QThread(parent), _TCPPort(TCPPort), _RealtimeStreamEnabled(false) {};
	void run() override;

public slots:

	void StartRealtimeStream() { _RealtimeStreamEnabled = true; };
	void EndRealtimeStream() { _RealtimeStreamEnabled = false; };

	// From Pole class.
	bool SyncPoleSensorsDataToServer(sensors* newData);		// Needs to be blocking call.
	bool SyncPoleEventsDataToServer(events* newData);		// Needs to be blocking call.
	bool SyncPoleSettingsDataToServer(settings* newData);	// Needs to be blocking call.

	// From Pole class.
	bool SyncPoleSettingsDataToPole(settings* newData);	// Needs to be blocking call.

signals:

	void UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus);

private:

	void _setupTCPConnection();
	bool _pingPole();

	void _setSocketBlockingMode(bool block, int timeout);

	int _TCPPort;
	SOCKET _TCPSocket = INVALID_SOCKET;
	char _TCPRecieveBuffer[TCP_BUFF_SIZE];
	std::chrono::time_point<std::chrono::high_resolution_clock> lastSynced;
	bool _RealtimeStreamEnabled;
};

class Pole : public QObject {
	Q_OBJECT

public:
	Q_DISABLE_COPY_MOVE(Pole)

	/*			Constructors			*/	
	explicit Pole(PoleDataModel* model, QVariantList data, Pole* parentItem = nullptr);	// I do need this for the header.
	explicit Pole(PoleDataModel* model, Pole* parentItem, int port, int sessionId, uint64_t HWID, uint8_t type);

	~Pole();

	/*			QT methods			*/
	void appendChild(Pole* child);

	Pole* child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	Pole* parentItem();

	/*			My Methods :)			*/

	uint8_t getPoleType() { return _poleType; }
	int		getPoleSessionID() { return _sessionId; }

	//void setIRLEDFrequency(uint16_t freq);
	//void setIMUSensitivity(float sensitivity);
	//void setVelostatSensitivity(float sensitivity);

public slots:

	// From MainWindow.
	void setUISelection(bool selected);



	// From tcpthread.
	void UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus);
	void UpdateLocalPoleSensorData(sensors		newData) { _poleState.Sensors = newData;  }
	void UpdateLocalPoleEventdData(events		newData) { _poleState.Events = newData;   }
	void UpdateLocalPoleSettingsData(settings	newData) { _poleState.Settings = newData; }



	// From the UI inputs
	//void UIUpdatedPolePositionValue();

signals:

	// To TCP thread.
	bool SyncPoleSensorsDataToServer(sensors* newData);
	bool SyncPoleEventsDataToServer(events* newData);
	bool SyncPoleSettingsDataToServer(settings* newData);

	bool SyncPoleSettingsDataToPole(settings* newData);

	// To PoleTreeView Data Model class.
	void updateVisual();

	//ToUpdate UI Elements.
	void VisualisePoleType(QString text);
	void VisualisePoleHWID(QString text);

	void VisualisePolePartner(QString text);
	void VisualisePoleBattery(QString text);

	void VisualisePolePosition(QString text);
	void VisualiseIRBeamFrequency(QString text);
	
	void VisualiseTouchSensitivity(QString text);
	void VisualiseIMUSensitivity(QString text);

protected:

	/*			Stuff for QT UI			*/
	std::vector<Pole*> _childItems;
	QVariantList _itemData;
	Pole* _parentItem;

	/*			My Stuff :)			*/

	PoleDataModel* _Model;

	int _sessionId;
	int _gateNumber;
	int _polePartner;
	uint64_t _poleHWID;
	uint8_t _poleType;


	poletcpthread* _TCPListnerThread;
	bool _selected;

	polestate _poleState;
};