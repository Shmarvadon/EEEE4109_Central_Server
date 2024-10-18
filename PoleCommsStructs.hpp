#pragma once 

namespace ptt {
	enum PoleTransmissionType : char {

		/*			Normal data transmission enums	(High bit cleared)				*/	

		SyncToPole				=	0b00000001,
		SyncToServer			=	0b00000010,

		Settings				=	0b00000100,
		Events					=	0b00001000,
		Sensors					=	0b00010000,
		State					=	0b00100000,

		/*			Ping, keep alive, reconnect & other	enums	(High bit set)			*/

		Ping					=	0b10000001,	// Byte to send to check if pole is alive.
		Pong					=	0b10000010,	// Expected response from the pole.

		SomethingWentWrong		=	0b10000100
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