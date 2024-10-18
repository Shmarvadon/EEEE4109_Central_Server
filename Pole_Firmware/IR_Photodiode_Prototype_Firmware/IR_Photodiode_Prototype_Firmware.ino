/*
prototype IR photodiode pole firmware.

Author: ShmarvDogg
bla bla bla.
*/
#include "WiFi.h"
#include "AsyncUDP.h"
#include <driver/adc.h>
#include <driver/i2s.h>
#include <dsps_fft2r.h>
#include <Adafruit_ICM20948.h>
#include <CircularBuffer.hpp>
#include "../../PoleState.hpp"
#include "../../PoleCommsStructs.hpp"


/*      Global variables      */
#define POLE_HWID (uint64_t)0
#define POLE_TYPE (uint8_t)2
polestate PoleState;


/*      IR Photodiode sensor related variables      */
#define FFT_BUFF_LENGTH 128
size_t bytes_read;
uint16_t DMA_InBuff[FFT_BUFF_LENGTH];
float FFT_InBuff[FFT_BUFF_LENGTH*2];
float FFT_OutBuff[FFT_BUFF_LENGTH/2];


/*      IMU STUFF     */
Adafruit_ICM20948 icm;
Adafruit_Sensor *accelerometer;
CircularBuffer<float,100> ICM_Accel_Readings;
float Processed_IMU_Readings[100];


/*      VELOSTAT STUFF      */
CircularBuffer<uint16_t, 100> Velostat_Readings;
uint16_t Processed_Velostat_Readings[100];


/*      NETWORK COMMS STUFF     */

bool UDP_broadcast_successful = false;

uint32_t Session_Port;
String Server_IP_String;
IPAddress Server_IP;
char CommsBuffer[512];

AsyncUDP udp_handshaker;
AsyncUDP udp_communicator;


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}


// Sensor setup functions.

void setup_photodiodes(){

  // Setup FFT library.
  auto ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
  if (ret != ESP_OK){
    Serial.println("Error when Initialising FFT library");
    PoleState.IRBeam = sensor::status::error;
  }
  else {
    PoleState.IRBeam = sensor::status::disabled;
  }
}

void setup_imu(){

  // Attempt to find the IMU on the I2C bus, if not then mark it as error.
  if (!icm.begin_I2C(104)) {
    Serial.println("Error when Initialising IMU");
    PoleState.IMU = sensor::status::error;
  }
  else {
    accelerometer = icm.getAccelerometerSensor();

    PoleState.IMU = sensor::status::disabled;
  }
}

void setup_velostat(){

  // Function to check if velostat is connected or not.
  //Attempt to put 10mA into velostat.
  dacWrite(25, (255 * 100) / 3300);

  // Wait 1ms.
  delay(1);

  // If the required voltage is super high it means resistance is too high.
  if (analogRead(34) >= ((255 * 3000) / 3300)) {
    PoleState.Velostat = sensor::status::error;
  }
  else{
    // Turn off velostat.
    dacWrite(25,0);

    PoleState.Velostat = sensor::status::disabled;
  }
}

// Sensor start functions.

void start_photodiodes(){

  // Setup I2S DMA config to sample ADC @128kHz
    i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 128000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = FFT_BUFF_LENGTH,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_5);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

  // Put 3.3V across photodiodes.
  dacWrite(26, 3300);

  // Finally indicate photodiodes are enabled.
  PoleState.IRBeam = sensor::status::enabled;
}

void start_imu() {

  // Set sampling rate to 112.5 Hz for IMU.
  icm.setAccelRateDivisor(10);

  PoleState.IMU = sensor::status::enabled;
}

void start_velostat(){

  // Put a current through the velostat.
  dacWrite(25, (255 * 10 * PoleState.Settings.VelostatCurrent) / 3300);

  PoleState.Velostat = sensor::status::enabled;
}

// Sensor stop functions.

void stop_photodiodes(){
  // Uninstall the i2s driver.
  i2s_driver_uninstall(I2S_NUM_0);

  // Lower the voltage across the photodiodes.
  dacWrite(26,0);

  // Mark the sensor as disabled.
  PoleState.IRBeam = sensor::status::disabled;
}

void stop_imu(){

  // Set sample rate to very low on IMU.
  icm.setAccelRateDivisor(1124);
  PoleState.IMU = sensor::status::disabled;
}

void stop_velostat(){
  
  // Put 0 current through velostat.
  dacWrite(25, 0);

  PoleState.Velostat = sensor::status::disabled;
}

// Sensor loop functions.

void photodiode_loop_iter(){

  // Read the data from the I2S bus.
  if (i2s_adc_enable(I2S_NUM_0) != ESP_OK) {PoleState.IRBeam = sensor::status::error; return;}
  size_t opt = i2s_read(I2S_NUM_0, &DMA_InBuff, sizeof(DMA_InBuff), &bytes_read,100);
  i2s_adc_disable(I2S_NUM_0);

  // Move the numbers into FFT_InBuff to have FFT done on them.
  for (int i = 0; i < FFT_BUFF_LENGTH; i++){
    FFT_InBuff[i*2] = float((DMA_InBuff[i] & 4095) - 2048); // Normalise about 0.
    FFT_InBuff[i*2 + 1] = 0.0f; // No complex value so 0.
  }

  // Perform the FFT.
  dsps_fft2r_fc32(FFT_InBuff, FFT_BUFF_LENGTH);
  // Reverse the bits.
  dsps_bit_rev_fc32(FFT_InBuff, FFT_BUFF_LENGTH);
  // Split out into 2 complex buffers.
  dsps_cplx2reC_fc32(FFT_InBuff, FFT_BUFF_LENGTH);

  // Find the dominant frequency.
  int dominant_freq = 0;
  float dominant_mag = 0;
  for (int i = 1; i < FFT_BUFF_LENGTH/2; i++){    // We dont care about the DC value.
    FFT_OutBuff[i] = sqrt(FFT_InBuff[i * 2 + 0] * FFT_InBuff[i * 2 + 0] + FFT_InBuff[i * 2 + 1] * FFT_InBuff[i * 2 + 1]) / FFT_BUFF_LENGTH;
    if (FFT_OutBuff[i] > dominant_mag) {dominant_mag = FFT_OutBuff[i]; dominant_freq = (1000*2*i);}
  }

  // Update the sensor reading.
  PoleState.SensorReadings.IRGateFreqReading = dominant_freq;


  // If within margin or error (the bin width) , 250Hz of the expected frequency then we probably have contact with the other pole.
  if (abs(dominant_freq - PoleState.Settings.IRBeamFreq) <= 100) PoleState.PlayerEvents.IRBeamTriggered = false;
  else PoleState.PlayerEvents.IRBeamTriggered = true;
}

void imu_loop_iter(){
  // Get updated values from IMU.
  sensors_event_t accel;
  if (!accelerometer->getEvent(&accel)) {PoleState.IMU = sensor::status::error; return;}

  float val = sqrt((accel.acceleration.x * accel.acceleration.x) + (accel.acceleration.y * accel.acceleration.y) + (accel.acceleration.z * accel.acceleration.z));
  ICM_Accel_Readings.push(val);

  float mean = 0;
  float variance = 0;
  // Calculate the mean and variance.
  if (ICM_Accel_Readings.isFull()) {
    for (int i = 0; i < 100; i++){
      mean += ICM_Accel_Readings[i];
    }

    mean = mean / 100;

    for (int i = 0; i < 100; i++){
      variance += (ICM_Accel_Readings[i] - mean) * (ICM_Accel_Readings[i] - mean);
    }

    variance = variance / 100;

    // Record the sensor value.
    PoleState.SensorReadings.IMUAccelReading = val;

    if (variance > PoleState.Settings.IMUSensitivity) PoleState.PlayerEvents.IMUTriggered = true;
    else PoleState.PlayerEvents.IMUTriggered = false;
  }
}

void velostat_loop_iter(){

  // Read in the feedback voltage from VCCS to get a proxy for the velostats resistance.
  uint16_t val = analogRead(34);

  // Push the value to the circular buffer.
  Velostat_Readings.push(val);

  // Normalise to mean then calculate variance as with the IMU loop.
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

    // Log the current read sensor value.
    PoleState.SensorReadings.VelostatResistanceReading = val;

    // If the variance is too big (The resistance of the velostat is changing a bunch in the past 100 samples) we conclude we have been hit by measuring against threashold.
    if (variance > PoleState.Settings.VelostatSensitivity) PoleState.PlayerEvents.VelostatTriggered = true;
    else PoleState.PlayerEvents.VelostatTriggered = false;
  }
}

// System functions.

void on_packet(AsyncUDPPacket packet){

  // For debug purposes.
  Serial.println("UDP packet recieved.");

  // Cast the data to a pointer because it makes stuff faster (probably).
  uint8_t* data = packet.data();
  
  // Also for debug.
  Serial.println(data[0]);

  // If the server pings.
  if (data[0] & ptt::Ping){
    // Pack and send response.
    CommsBuffer[0] = (char)130;      
    udp_communicator.writeTo((uint8_t*)CommsBuffer, 1, packet.remoteIP(), packet.remotePort());
  }

  // If the server asks for events.
  if (data[0] == (ptt::SyncToServer | ptt::Events)){

    // Pack the reply.
    CommsBuffer[0] = data[0];
    std::string PlayerEventsSerialised = PoleState.PlayerEvents.serialise();
    memcpy((char*)PlayerEventsSerialised.c_str(), &CommsBuffer[1], sizeof(PlayerEventsSerialised));

    // Send the reply.
    udp_communicator.writeTo((uint8_t*)CommsBuffer, sizeof(CommsBuffer), packet.remoteIP(), packet.remotePort());

  }

  // If the server asks for sensor readings.
  if (data[0] == (ptt::SyncToServer | ptt::Sensors)){

    // Pack the reply.
    CommsBuffer[0] = data[0];
    std::string SensorReadingsSerialised = PoleState.SensorReadings.serialise();
    memcpy((char*)SensorReadingsSerialised.c_str(), &CommsBuffer[1], sizeof(SensorReadingsSerialised));

    // Send the reply.
    udp_communicator.writeTo((uint8_t*)CommsBuffer, sizeof(CommsBuffer), packet.remoteIP(), packet.remotePort());
  }

  // If the server asks for settings.
  if (data[0] == (ptt::SyncToServer | ptt::Settings)){

  }

  // If the server sends updated settings.
  if (data[0] == (ptt::SyncToPole | ptt::Settings)){

  }

  // If the server asks for pole state.
  if (data[0] == (ptt::SyncToServer | ptt::State)){

  }

  // If the server sends updated pole state.
  if (data[0] == (ptt::SyncToPole | ptt::State)){

  }

}

void wifi_loop_iter(){


  // If realtime reporting of events is enabled.
  if (PoleState.Settings.EnableRealtimeEventsReporting){

    // Pack the return thingy.
    CommsBuffer[0] = (ptt::SyncToServer | ptt::Events);

    // Serialise the pole state data.
    std::string PoleStateSerialised = PoleState.serialise();
    memcpy((char*)PoleStateSerialised.c_str(), &CommsBuffer[1], sizeof(PoleStateSerialised));

    udp_communicator.writeTo((uint8_t*)CommsBuffer, sizeof(PoleState.PlayerEvents) + 1, Server_IP, Session_Port);
  }
}

void setup_wifi(){

  // Begin the WiFi.
  WiFi.begin(PoleState.Settings.WiFiSSID.c_str(), PoleState.Settings.WiFiPassword.c_str());

  // Wait for connection to be established.
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print(".");
  }
  Serial.println();

  // Print out connection info to terminal.
  Serial.println("Connected To WiFi.");
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println((String)"RSSI: " + WiFi.RSSI());

  // Setup UDP socket for broadcast.
  if (udp_handshaker.connect(IPAddress(255,255,255,255), PoleState.Settings.UDPBroadcastPort)){

    // Lambda function to handle any replies to the broadcast.
    udp_handshaker.onPacket([](AsyncUDPPacket packet){

      // Decode the packet received.
      Serial.println("Recieved a UDP packet.");
      memcpy(&Session_Port, packet.data(), sizeof(uint32_t));
      Serial.println(Session_Port);
      Serial.println(packet.remoteIP().toString());
      Server_IP.fromString(packet.remoteIP().toString());

      // Indicate that the UDP broadcast has been successfull.
      UDP_broadcast_successful = true;
    });
  }

  // Configure the UDP socket to listen for packets from port UDP broadcast -1.
  if (udp_handshaker.listen(PoleState.Settings.UDPBroadcastPort - 1)){
    Serial.println("Unable to setup UDP socket to listen for server reply.");
    exit(0);
  }

  // Send the UDP broadcast untill we get a response from the server.
  do {

    Serial.println("Sending UDP broadcast now.");

    // Pack data ready to be sent.
    CommsBuffer[0] = POLE_HWID;
    CommsBuffer[8] = POLE_TYPE;

    // Send the data.
    udp_handshaker.broadcastTo((uint8_t*)&CommsBuffer, (size_t)512, (uint16_t)PoleState.Settings.UDPBroadcastPort);

    // Wait a second.
    delay(1000);
  } while (!UDP_broadcast_successful);

  // Close the udp handshaker if we successfully connect.
  udp_handshaker.close();

  // Connect to the server on the long lived connection port.
  if (udp_communicator.connect(Server_IP, Session_Port)){
    udp_communicator.onPacket(on_packet);
  }

  // Start listening for responses from the server.
  if (!udp_communicator.listen(Session_Port)) {Serial.println("Unable to listen for incoming UDP packets."); while (true) delay(10);}
  Serial.println("realtime comms socket opened with server.");

}