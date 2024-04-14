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

enum PoleTransmissionType { 
	RequestPoleConfigurablesUpdate		= 0b00000010,
	RequestPoleEventsUpdate				= 0b00000100,
	RequestPoleSensorsUpdate			= 0b00001000,
	Ping								= 0b00000001,	// Byte to send to check if pole is alive.
	Pong								= 0b00010000	// Expected response from the pole.
};
 
namespace pcs {
	enum PoleConnectionStatus : uint8_t {
		Connected = 0b00000001,
		Disconnected = 0b00000010,
		Reconnecting = 0b00000100,
		Unknown = 0b00001000
	};
}


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
	uint8_t kayakerPassing;
};

struct settings {
	// Configurables.
	uint16_t IRTransmitFreq;	// Frequency of IR LEDs.
	float IMUSensitivity;		// IMU sensitivity.
	float velostatSensitivity;	// Velostat sensitivity.
	int gateNumber;				// Associated gate number.
	int poleNumber;				// Associated session number.
	int polePartner;			// Other pole comprising the gate.
};

struct polestate {
	settings	Settings;
	events		Events;
	sensors		Sensors;
};

class poletcpthread : public QThread {
	Q_OBJECT

public:
	explicit poletcpthread(QObject* parent, int TCPPort) : QThread(parent), _TCPPort(TCPPort) {};
	void run() override;

signals:
	void UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus);

private:

	void _setupTCPConnection();
	bool _pingPole();

	int _TCPPort;
	SOCKET _TCPSocket = INVALID_SOCKET;
	char _TCPRecieveBuffer[TCP_BUFF_SIZE];
	std::chrono::time_point<std::chrono::high_resolution_clock> lastSynced;
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

	//void setIRLEDFrequency(uint16_t freq);
	//void setIMUSensitivity(float sensitivity);
	//void setVelostatSensitivity(float sensitivity);

public slots:

	void UpdatePoleConnectionStatus(pcs::PoleConnectionStatus connectionStatus);

signals:
	void updateVisual();


protected:

	/*			Stuff for QT UI			*/
	std::vector<Pole*> _childItems;
	QVariantList _itemData;
	Pole* _parentItem;

	/*			My Stuff :)			*/

	PoleDataModel* _Model;
	int _sessionId;
	uint64_t _poleHWID;
	uint8_t _poleType;
	poletcpthread* _TCPListnerThread;

	polestate _poleState;
};