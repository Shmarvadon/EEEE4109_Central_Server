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
        Hibernating     =   0b00000001,
        MediumPower     =   0b00000010,
        HighPower       =   0b00000100
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
	float	IRGateReading = 0;		// IR Gate detected freq.
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

inline events operator|(events a, events b) {
  return static_cast<events>((uint8_t)a | (uint8_t)b);
}

inline events operator^(events a, events b) {
  return static_cast<events>((uint8_t)a ^ (uint8_t)b);
}

inline events& operator|=(events& a, events b) {
  return a = a | b;
}

inline events& operator^=(events& a, events b) {
  return a = a ^ b;
}

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