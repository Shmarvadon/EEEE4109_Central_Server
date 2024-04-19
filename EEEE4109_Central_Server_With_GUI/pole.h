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
#include <qtimer.h>


#define TCP_BUFF_SIZE 128

class PoleDataModel;
class Pole;
class TreeViewHeader;

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

namespace pps{
	enum PolePowerState : uint8_t {
		IRBeamOn			=	0b00000001,
		IRCameraOn			=	0b00000010,
		VelostatOn			=	0b00000100,
		IMUOn				=	0b00001000,
		PoleHibernating		=	0b00010000
	};

	inline PolePowerState operator|(PolePowerState a, PolePowerState b) {
		return static_cast<PolePowerState>((uint8_t)a | (uint8_t)b);
	}

	inline PolePowerState operator^(PolePowerState a, PolePowerState b) {
		return static_cast<PolePowerState>((uint8_t)a ^ (uint8_t)b);
	}

	inline PolePowerState& operator|=(PolePowerState& a, PolePowerState b) {
		return a = a | b;
	}

	inline PolePowerState& operator^=(PolePowerState& a, PolePowerState b) {
		return a = a ^ b;
	}

}

struct sensors {
	// Sensor Readings.
	float	velostatReading = 0;	// Velostat resistance.
	float	IRGateReading = 0;		// IR gate detected freq.
	float	IRCameraReading = 0;	// IR camera motion vector.
	float	IMUReading = 0;			// IMU acceleration reading.
	int	batteryReading = 0;		// Battery voltage reading.
};

enum events : uint8_t {

	IRBeamTriggered					=	0b00000001,
	Knocked							=	0b00000010,
	VelostatTriggered				=	0b00000100,
	IMUTriggered					=	0b00001000,
	IRCameraTriggered				=	0b00010000,
	KayakerPassageDirectionLTR		=	0b00100000,		// Left To Right (LTR) w.r.t IR camera.
	KayakerPassageDirectionRTL		=	0b01000000,		// Right To Left (RTL) w.r.t IR camera.
	KayakerPassageDirectionNone		=	0b10000000,		// Unknown passage direction.

};

struct settings {
	// Configurables.
	uint16_t IRTransmitFreq = 0;				// Frequency of IR LEDs.
	float IMUSensitivity = 0;					// IMU sensitivity.
	float velostatSensitivity = 0;			// Velostat sensitivity.
	pps::PolePowerState powerState = (pps::PolePowerState)0;		// Set the power state of the pole.
};

struct polestate {
	settings	Settings;
	events		Events;
	sensors		Sensors;
};

class poletcpthread : public QObject {
	Q_OBJECT

public:
	explicit poletcpthread(QObject* parent, int TCPPort) : _TCPPort(TCPPort), _RealtimeStreamEnabled(false) {};
	
	void run();

	void mainLoop();

public slots:

	void StartRealtimeStream() { _RealtimeStreamEnabled = true; _realtimeStreamTimer->start(10); };
	void EndRealtimeStream() { _RealtimeStreamEnabled = false; _realtimeStreamTimer->start(10); };

	// From Pole class.
	sensors syncSensorReadingsToServer();				// Needs to be blocking call.
	events syncEventsToServer();						// Needs to be blocking call.

	settings syncSettingsToServer();					// Needs to be blocking call.
	bool syncSettingsToPole(settings newData);			// Needs to be blocking call.


signals:

	void UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus);
	void UpdatePoleEvents(events newEventsData);
	void UpdatePoleSensors(sensors newSensorData);

private:

	void _setupTCPConnection();
	bool _pingPole();
	void _setSocketBlockingMode(bool block, int timeout);

	void _realtimeStreamingLoop();


	int _TCPPort;
	SOCKET _TCPSocket = INVALID_SOCKET;
	char _TCPRecieveBuffer[TCP_BUFF_SIZE];
	std::chrono::time_point<std::chrono::high_resolution_clock> lastSynced;
	bool _RealtimeStreamEnabled;
	QTimer* _mainLoopTimer,
		  * _realtimeStreamTimer;
};

class Pole : public QObject {
	Q_OBJECT

public:
	Q_DISABLE_COPY_MOVE(Pole)

	/*			Constructors			*/	
	explicit Pole(PoleDataModel* model, TreeViewHeader* parentItem, int port, int sessionId, uint64_t HWID, uint8_t type);

	~Pole();

	/*			QT methods			*/
	void appendChild(Pole* child);

	Pole* child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	TreeViewHeader* parentItem();

	/*			My Methods :)			*/

	uint8_t		getPoleType() { return _poleType; }
	int			getPoleSessionID() { return _sessionId; }

	int			getPolePartnerID() { return _polePartner; }
	void		setPoleSessionID(int newId) { _sessionId = newId; }

	void		setPolePartnerID(int partnerId) { _polePartner = partnerId; }
	polestate*	getPoleState() { return &_poleState; }


public slots:

	// From MainWindow.
	void setUISelection(bool selected);



	// From tcpthread.
	void UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus);
	void UpdatePoleEvents(events newEventsData);
	void UpdatePoleSensors(sensors newSensorData);


	// From the UI inputs
	//void UIUpdatedPolePositionValue();

signals:

	// To TCP thread.
	sensors syncSensorReadingsToServer();
	events syncEventsToServer();

	settings syncSettingsToServer();
	bool syncSettingsToPole(settings newData);

	void startTCPThread();
	pcs::PoleConnectionStatus connectionStatus();

	void StartRealtimeStream();
	void EndRealtimeStream();

	// To PoleTreeView Data Model class.
	void updatetreeVisual();
	bool findPartnerPole(Pole* pPole);

	//To Update UI Elements.
	void VisualisePoleType(QString text);
	void VisualisePoleHWID(QString text);

	void VisualisePolePartner(QString text);
	void VisualisePoleBattery(QString text);

	void VisualisePolePosition(QString text);
	void VisualiseIRBeamFrequency(QString text);
	
	void VisualiseTouchSensitivity(QString text);
	void VisualiseIMUSensitivity(QString text);

	void updatePoleEventsVisualIndicators(events eventsData);
	void setPowerStateIndicator(pps::PolePowerState powerState);


protected:

	/*			Stuff for QT UI			*/
	std::vector<Pole*> _childItems;
	TreeViewHeader* _parentItem;

	/*			My Stuff :)			*/

	PoleDataModel* _Model;
	int _sessionId;
	int _gateNumber;
	int _polePartner;
	uint64_t _poleHWID;
	uint8_t _poleType;
	QThread _TCPListnerThread;
	bool _selected;
	pcs::PoleConnectionStatus _connstat;
	polestate _poleState;
};