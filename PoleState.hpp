#pragma once
#include "string"
#include <ArduinoJson.h>
// Find it C:\Users\harve\OneDrive\Documents\Arduino\libraries\ArduinoJson\src

enum poletype : uint8_t {
    LEDPole        = 1,
    PhotodiodePole = 2
};

namespace pps {
    enum polepowerstate : uint8_t {

        Hibernating = 1,
        MediumPower = 2,
        HighPower   = 3
    };
}

// Sensor Readings.
struct sensors {
	float	VelostatResistanceReading   = 0;	    // Velostat resistance.
	float	IRGateFreqReading           = 0;		// IR Gate detected freq.
	float	IRCameraMotionVectorReading = 0;	    // IR camera motion vector.
	float	IMUAccelReading             = 0;	    // IMU acceleration reading.
	int	    BatteryVoltageReading       = 0;		// Battery voltage reading.

    std::string serialise(){
        JsonDocument doc;

        doc[0] = VelostatResistanceReading;
        doc[1] = IRGateFreqReading;
        doc[2] = IRCameraMotionVectorReading;
        doc[3] = IMUAccelReading;
        doc[4] = BatteryVoltageReading;

        std::string output;
        serializeJson(doc, output);
        return output;
    }

    void deserialise(std::string input){
        JsonDocument doc;
        deserializeJson(doc, input);

        VelostatResistanceReading    = doc[0];
        IRGateFreqReading            = doc[1];
        IRCameraMotionVectorReading  = doc[2];
        IMUAccelReading              = doc[3];
        BatteryVoltageReading        = doc[4];
    }
};

namespace thermalcameraevents {
    enum ThermalCameraEvents : uint8_t {
        None = 0,
        LTR = 1,
        RTL = 2
    };
}

// Game events.
struct playerevents {
    bool IRBeamTriggered;
    bool VelostatTriggered;
    bool IMUTriggered;
    bool IRCameraTriggered;
    thermalcameraevents::ThermalCameraEvents ThermalCameraTriggered;

    std::string serialise(){
        JsonDocument doc;

        doc[0] = IRBeamTriggered;
        doc[1] = VelostatTriggered;
        doc[2] = IMUTriggered;
        doc[3] = IRCameraTriggered;
        doc[4] = ThermalCameraTriggered;

        std::string output;
        serializeJson(doc, output);
        return output;
    }

    void deserialise(std::string input){
        JsonDocument doc;
        deserializeJson(doc, input);

        IRBeamTriggered        = doc[0];
        VelostatTriggered      = doc[1];
        IMUTriggered           = doc[2];
        IRCameraTriggered      = doc[3];
        ThermalCameraTriggered = doc[4];
    }
};

namespace sensor {
    namespace status {
        enum sensorstatus : uint8_t {
            notpresent      = 0,
            enabled         = 1,
            disabled        = 2,
            error           = 3
        };
    }
}

// Pole settings
struct settings {
    uint16_t IRBeamFreq;
    float IMUSensitivity;
    float VelostatSensitivity;
    int VelostatCurrent; // Should be = 10 for 10mA.
    pps::polepowerstate PowerState;

    std::string WiFiSSID;
    std::string WiFiPassword;
    uint32_t UDPBroadcastPort;
    bool EnableRealtimeEventsReporting;

    std::string serialise(){
        JsonDocument doc;

        doc[0] = IRBeamFreq;
        doc[1] = IMUSensitivity;
        doc[2] = VelostatSensitivity;
        doc[3] = VelostatCurrent;
        doc[4] = PowerState;
        doc[5] = WiFiSSID;
        doc[6] = WiFiPassword;
        doc[7] = UDPBroadcastPort;
        doc[8] = EnableRealtimeEventsReporting;

        std::string output;
        serializeJson(doc, output);
        return output;
    }

    void deserialise(std::string input){
        JsonDocument doc;
        deserializeJson(doc, input);

        IRBeamFreq                     = doc[0];
        IMUSensitivity                 = doc[1];
        VelostatSensitivity            = doc[2];
        VelostatCurrent                = doc[3];
        PowerState                     = doc[4];
        WiFiSSID                       = std::string(doc[5]);
        WiFiPassword                   = std::string(doc[6]);
        UDPBroadcastPort               = doc[7];
        EnableRealtimeEventsReporting  = doc[8];
    }
};

struct polestate {
    sensors SensorReadings;
    playerevents PlayerEvents;
    settings  Settings;
    poletype PoleType;


    sensor::status::sensorstatus IMU;
    sensor::status::sensorstatus IRBeam;
    sensor::status::sensorstatus Velostat;
    sensor::status::sensorstatus IRCamera;

    // Could add a serialise and de-serialise function here.

    std::string serialise(){
        JsonDocument doc;

        // Serialise the sensor readings.
        doc["SensorReadings"][0] = SensorReadings.VelostatResistanceReading;
        doc["SensorReadings"][1] = SensorReadings.IRGateFreqReading;
        doc["SensorReadings"][2] = SensorReadings.IRCameraMotionVectorReading;
        doc["SensorReadings"][3] = SensorReadings.IMUAccelReading;
        doc["SensorReadings"][4] = SensorReadings.BatteryVoltageReading;

        // Serialise the player events.
        doc["PlayerEvents"][0] = PlayerEvents.IRBeamTriggered;
        doc["PlayerEvents"][1] = PlayerEvents.VelostatTriggered;
        doc["PlayerEvents"][2] = PlayerEvents.IMUTriggered;
        doc["PlayerEvents"][3] = PlayerEvents.IRCameraTriggered;
        doc["PlayerEvents"][4] = PlayerEvents.ThermalCameraTriggered;

        // Serialise Settings.
        doc["Settings"][0] = Settings.IRBeamFreq;
        doc["Settings"][1] = Settings.IMUSensitivity;
        doc["Settings"][2] = Settings.VelostatSensitivity;
        doc["Settings"][3] = Settings.VelostatCurrent;
        doc["Settings"][4] = Settings.PowerState;
        doc["Settings"][5] = Settings.WiFiSSID;
        doc["Settings"][6] = Settings.WiFiPassword;
        doc["Settings"][7] = Settings.UDPBroadcastPort;
        doc["Settings"][8] = Settings.EnableRealtimeEventsReporting;

        // Serialise the rest.
        doc["PoleType"] = PoleType;

        doc["IMUStatus"] = IMU;
        doc["IRBeamStatus"] = IRBeam;
        doc["VelostatStatus"] = Velostat;
        doc["IRCameraStatus"] = IRCamera;

        std::string output;
        serializeJson(doc, output);
        return output;
    }

    void deserialise(std::string input){

        // Deserialise the input string.
        JsonDocument doc;
        deserializeJson(doc, input);

        // Deserialise the sensor readings.
        SensorReadings.VelostatResistanceReading    = doc["SensorReadings"][0];
        SensorReadings.IRGateFreqReading            = doc["SensorReadings"][1];
        SensorReadings.IRCameraMotionVectorReading  = doc["SensorReadings"][2];
        SensorReadings.IMUAccelReading              = doc["SensorReadings"][3];
        SensorReadings.BatteryVoltageReading        = doc["SensorReadings"][4];

        // Deserialise the player events.
        PlayerEvents.IRBeamTriggered        = doc["PlayerEvents"][0];
        PlayerEvents.VelostatTriggered      = doc["PlayerEvents"][1];
        PlayerEvents.IMUTriggered           = doc["PlayerEvents"][2];
        PlayerEvents.IRCameraTriggered      = doc["PlayerEvents"][3];
        PlayerEvents.ThermalCameraTriggered = doc["PlayerEvents"][4];

        // Deserialise the settings.
        Settings.IRBeamFreq                     = doc["Settings"][0];
        Settings.IMUSensitivity                 = doc["Settings"][1];
        Settings.VelostatSensitivity            = doc["Settings"][2];
        Settings.VelostatCurrent                = doc["Settings"][3];
        Settings.PowerState                     = doc["Settings"][4];
        Settings.WiFiSSID                       = std::string(doc["Settings"][5]);
        Settings.WiFiPassword                   = std::string(doc["Settings"][6]);
        Settings.UDPBroadcastPort               = doc["Settings"][7];
        Settings.EnableRealtimeEventsReporting  = doc["Settings"][8];

        //Deserialise the rest.
        PoleType = doc["PoleType"];

        IMU = doc["IMUStatus"];
        IRBeam = doc["IRBeamStatus"];
        Velostat = doc["VelostatStatus"];
        IRCamera = doc["IRCameraStatus"];
    }
};