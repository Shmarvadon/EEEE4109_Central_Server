//#include "WiFi.h"
//#include "AsyncUDP.h"
#include "../comms.hpp"
#include <driver/adc.h>
#include <driver/i2s.h>
#include <../imu.hpp>
#include <CircularBuffer.hpp>
//#include "../../PoleStateDataStructures.h"
#include "../../PoleState.hpp"
#include <driver/dac_continuous.h>


/*      MISCELLANEOUS GLOBAL VARIABLES & DEFINES      */
#define POLE_HWID 0
#define POLE_TYPE (uint8_t)1
polestate PoleState;








/*      IR LED STUFF     */
uint8_t IR_LED_WaveForm[80];
dac_continuous_handle_t* IR_LED_DMA_Handle = nullptr;
bool IR_Beam_On = false;



/*      VELOSTAT STUFF      */
bool Velostat_On = true;
CircularBuffer<uint16_t, 100> Velostat_Readings;
uint16_t Processed_Velostat_Readings[100];
int Velostat_Current = 10; //mA



/*      THERMAL CAMERA STUFF      */
#define D6T_ADDR          0x0A
#define D6T_READ_COMMAND  0x4C
CircularBuffer<float, 100> IR_Camers_Readings;
uint8_t IR_Camera_Receive_Buff[19];
int16_t IR_Camera_Pixel_Data[8];
int16_t IR_Camera_Weightings[8] = {-4, -3, -2, -1, 1, 2, 3, 4};







/*      CODE AND OTHER STUFF      */
void On_Packet(AsyncUDPPacket packet){

  Serial.println("UDP packet recieved.");

  uint8_t* data = packet.data();

  Serial.println(data[0]);

  // If the server pings.
  if (data[0] & ptt::Ping){
      //udp_communicator.write((char)130);

      Comms_Send_Buffer[0] = (char)130;


      udp_communicator.writeTo((uint8_t*)Comms_Send_Buffer, 1, packet.remoteIP(), packet.remotePort());


      Serial.println("Recieved ping from server");
      return;
    }

    // If the server asks for events.
    if (data[0] == (ptt::SyncToServer | ptt::Events)){
      //Serial.println("Server has asked for sync of events.");
      Comms_Send_Buffer[0] = data[0];
      Comms_Send_Buffer[1] = PoleStatus.Events;
      //udp_communicator.write((uint8_t*)Comms_Send_Buffer, sizeof(char) * 2);
      udp_communicator.writeTo((uint8_t*)Comms_Send_Buffer, 2, packet.remoteIP(), packet.remotePort());

      Serial.println("Server asked for events");
      return;
    }

    // If the server asks for a copy of the sensor values.
    if (data[0] == (ptt::SyncToServer | ptt::Sensors)){
      Comms_Send_Buffer[0] = data[0];
      memcpy(&Comms_Send_Buffer[1], &PoleStatus.Sensors.velostatReading, sizeof(float));
      memcpy(&Comms_Send_Buffer[5], &PoleStatus.Sensors.IRGateReading, sizeof(float));
      memcpy(&Comms_Send_Buffer[9], &PoleStatus.Sensors.IRCameraReading, sizeof(float));
      memcpy(&Comms_Send_Buffer[13], &PoleStatus.Sensors.IMUReading, sizeof(float));
      memcpy(&Comms_Send_Buffer[17], &PoleStatus.Sensors.batteryReading, sizeof(int));
      //udp_communicator.write((uint8_t*)Comms_Send_Buffer, sizeof(char) * 18);
      udp_communicator.writeTo((uint8_t*)Comms_Send_Buffer, 18, packet.remoteIP(), packet.remotePort());

      Serial.println("Server asked for sensor readings");

      return;
    }

    // If the server asks for a copy of the settings.
    if (data[0] == (ptt::SyncToServer | ptt::Configurables)){
      Comms_Send_Buffer[0] = data[0];
      memcpy(&Comms_Send_Buffer[1], &PoleStatus.Settings.IRTransmitFreq, sizeof(uint16_t));
      memcpy(&Comms_Send_Buffer[3], &PoleStatus.Settings.IMUSensitivity, sizeof(float));
      memcpy(&Comms_Send_Buffer[7], &PoleStatus.Settings.velostatSensitivity, sizeof(float));
      memcpy(&Comms_Send_Buffer[11], &PoleStatus.Settings.powerState, sizeof(uint8_t));

      //udp_communicator.write((uint8_t*)Comms_Send_Buffer, 12);
      udp_communicator.writeTo((uint8_t*)Comms_Send_Buffer, 12, packet.remoteIP(), packet.remotePort());

      Serial.println("Server asked for settings");
      return;
    }

    // If the server sends updated settings.
    if (data[0] == (ptt::SyncToPole | ptt::Configurables)){
      memcpy(&PoleStatus.Settings.IRTransmitFreq, &data[1], sizeof(uint16_t));
      memcpy(&PoleStatus.Settings.IMUSensitivity, &data[3], sizeof(float));
      memcpy(&PoleStatus.Settings.velostatSensitivity, &data[7], sizeof(float));
      memcpy(&PoleStatus.Settings.powerState, &data[11], sizeof(uint8_t));

      Serial.println("Server sent settings");

      // Call function to update power state stuffs.
      UpdatePowerState();

      return;
    }
}

void setup() {

  // Setup serial.
  Serial.begin(115200);

  // Setup the IMU.
  Setup_IMU();

  // Setup the Velostat.
  Setup_Velostat(10);

  // Setup the WiFi.
  SetupWiFi(&On_Packet, POLE_HWID, POLE_TYPE);

  // Set I2C to 400khz.
  Wire.setClock(400000);

  Run_LEDs(10000, 40, 1300);
  PoleStatus.Settings.IMUSensitivity = 0.3;
  PoleStatus.Settings.velostatSensitivity = 0.3;
}

void loop() {
  // Time the loop.
  auto loopStart = micros();


  // Run the IMU stuff if turned on.
  if (IMU_On) (IMU_Loop()) ? PoleStatus.Events |= IMUTriggered : PoleStatus.Events ^= IMUTriggered;

  // Run the Velostat stuff if turned on.
  if (Velostat_On) (Velostat_Loop()) ? PoleStatus.Events |= VelostatTriggered : PoleStatus.Events ^= VelostatTriggered;

  // Time the loop.
  auto loopEnd = micros();
  delayMicroseconds( 10000 -loopEnd + loopStart);
  //Serial.println(loopEnd - loopStart);

}

bool Velostat_Loop(){

  // Read the value in.
  uint16_t val = analogRead(34);

  // Push it to circular buffer.
  Velostat_Readings.push(val);

  uint16_t mean = 0;
  float variance = 0;
  // Calculate the mean and variance.
  if (Velostat_Readings.isFull()) {
    for (int i = 0; i < 100; i++){
      mean += Velostat_Readings[i];
    }

    mean = mean / 100;

    for (int i = 0; i < 100; i++){
      variance += (Velostat_Readings[i] - mean) * (Velostat_Readings[i] - mean);
    }

    variance = variance / 100;

    // If the variance is too big (The resistance of the velostat is changing a bunch in the past 100 samples) we conclude we have been hit by measuring against threashold.
    if (variance > PoleStatus.Settings.velostatSensitivity) return true;
    else return false;
  }
}

void IR_Camera_Loop(){
  // Read the data from the IR camera.
  {
    memset(IR_Camera_Receive_Buff, 0, 19);
    Wire.beginTransmission(D6T_ADDR);
    Wire.write(D6T_READ_COMMAND);
    Wire.endTransmission();
    Wire.requestFrom(D6T_ADDR, 19);

    for (int i = 0; i < 19; i++){
      IR_Camera_Receive_Buff[i] = Wire.read();
    }

    // Process the pixel data into an array of int16_t (Dont need floats just yet & they are computationally expensive to deal with).
    for(int i = 1; i < 8 + 1; i++){
      IR_Camera_Pixel_Data[i-1] = [&](){return (int16_t)((uint16_t)IR_Camera_Receive_Buff[ i * 2] | (uint16_t)IR_Camera_Receive_Buff[(i * 2) + 1] << 8);}();
    }
  }

  // Calculate the weighted position sum.
  {
    float sum = 0;
    for (int i = 0; i < 8; i++){
      sum += IR_Camera_Weightings[i] * IR_Camera_Pixel_Data[i];
    }

    IR_Camers_Readings.push(sum);
  }

  // Perform linear regression on points to figure out which direction the kayaker is passing.
  {
    
  }
}

void Run_LEDs(uint16_t Freq, uint16_t Duty_Cycle, uint16_t Amplitude){
  
  // Update global variables to allow querying of current freq & such of the output waveform.
  PoleStatus.Settings.IRTransmitFreq = Freq;

  Amplitude = (Amplitude * 255) / 3300;

  Duty_Cycle /= 10;

  // Populate the buffer with waveform ready for DMA
  for (int i = 0; i < 80; i++){
    if ( (i % 10) < Duty_Cycle)  IR_LED_WaveForm[i] = Amplitude;
    else IR_LED_WaveForm[i] = 0;
  }

  //Configure the I2S peripherals to be ready to transmit waveform data.
  dac_continuous_config_t i2s_config = {
        .chan_mask = DAC_CHANNEL_MASK_CH1,
        .desc_num = 2,
        .buf_size = 100,
        .freq_hz = Freq * 10,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
        .chan_mode = DAC_CHANNEL_MODE_SIMUL,
  };

  dac_continuous_new_channels(&i2s_config, IR_LED_DMA_Handle);

  dac_continuous_enable(*IR_LED_DMA_Handle);

  // Function to load the buffer into DMA and cyclically run the DAC conversion.
  dac_continuous_write_cyclically(*IR_LED_DMA_Handle, (uint8_t*)IR_LED_WaveForm, 100, NULL);
}

void Stop_LEDs(){

  // Should delete the cyclical DMA and stop it.
  dac_continuous_del_channels(*IR_LED_DMA_Handle);
  //dac_continuous_disable(IR_LED_DMA_HANDLE);

  // Consult https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/dac.html 
  // If this code does not work.

}



void Setup_Velostat(int Current){
  dacWrite(25, (255 * Current) / 3300);
}

void Setup_IR_Camera(){
    // Send initialization settings to D6T sensor
    Wire.beginTransmission(D6T_ADDR); // I2C slave address
    Wire.write(0x02);                  // D6T register
    Wire.write(0x00);                  // Write Data
    Wire.write(0x01);                  // Write Data
    Wire.write(0xEE);                  // Write Data
    Wire.endTransmission();            // I2C Stop
    Wire.beginTransmission(D6T_ADDR);  // I2C slave address
    Wire.write(0x05);                  // D6T register
    Wire.write(0x90);                  // Write Data
    Wire.write(0x3A);                  // Write Data
    Wire.write(0xB8);                  // Write Data
    Wire.endTransmission();            // I2C Stop
    Wire.beginTransmission(D6T_ADDR);  // I2C slave address
    Wire.write(0x03);                  // D6T register
    Wire.write(0x00);                  // Write Data
    Wire.write(0x03);                  // Write Data
    Wire.write(0x8B);                  // Write Data
    Wire.endTransmission();            // I2C Stop
    Wire.beginTransmission(D6T_ADDR);  // I2C slave address
    Wire.write(0x03);                  // D6T register
    Wire.write(0x00);                  // Write Data
    Wire.write(0x07);                  // Write Data
    Wire.write(0x97);                  // Write Data
    Wire.endTransmission();            // I2C Stop
    Wire.beginTransmission(D6T_ADDR);  // I2C slave address
    Wire.write(0x02);                  // D6T register
    Wire.write(0x00);                  // Write Data
    Wire.write(0x00);                  // Write Data
    Wire.write(0xE9);                  // Write Data
    Wire.endTransmission();            // I2C Stop
}

void UpdatePowerState(){

  if (PoleStatus.Settings.powerState & pps::HighPower){
    // Turn the LEDs on.
    if (!IR_Beam_On) Run_LEDs(PoleStatus.Settings.IRTransmitFreq, 40, 500);
    // Turn the IMU on.
    if (!IMU_On) IMU_On = true;
    // Also turn on velostat.
    if (!Velostat_On) {Velostat_On = true; Setup_Velostat(10);}
  }

  if (PoleStatus.Settings.powerState & pps::MediumPower){
    // Turn the LEDs off.
    if (IR_Beam_On) Stop_LEDs();
    // Turn the IMU on.
    if (!IMU_On) IMU_On = true;
    // Also turn off velostat.
    if (Velostat_On) {Velostat_On = false; Setup_Velostat(0);}
  }

  if (PoleStatus.Settings.powerState & pps::Hibernating){
    // Turn the LEDs off.
    if (IR_Beam_On) Stop_LEDs();
    // Turn the IMU off.
    if (IMU_On) IMU_On = false;
    // Also turn off velostat.
    if (Velostat_On) {Velostat_On = false; Setup_Velostat(0);}
  }
}
