
#include "WiFi.h"
#include "AsyncUDP.h"
#include <driver/adc.h>
#include <driver/i2s.h>
#include <dsps_fft2r.h>
#include <Adafruit_ICM20948.h>
#include <CircularBuffer.hpp>
#include "PoleStateDataStructures.h"



/*      MISCELLANEOUS GLOBAL VARIABLES & DEFINES      */
#define POLE_HWID (uint64_t)0
#define POLE_TYPE (uint8_t)2
polestate PoleStatus;



/*      NETWORK COMMS STUFF     */
const char* ssid = "network21iot";
const char* password = "RCD-M40DAB";

bool UDP_broadcast_Successful = false;
uint32_t UDP_broadcast_Port = 42069;

uint32_t Session_TCP_Port;
String Server_IP_String;
IPAddress Server_IP;
char Comms_Send_Buffer[512];

AsyncUDP udp_handshaker;
AsyncUDP udp_communicator;



/*      IR PHOTODIODE STUFF     */
#define FFT_BUFF_LENGTH 128
size_t bytes_read;
uint16_t DMA_InBuff[FFT_BUFF_LENGTH];
float FFT_InBuff[FFT_BUFF_LENGTH*2];
float FFT_OutBuff[FFT_BUFF_LENGTH/2];
bool IR_Beam_On = false;



/*      IMU STUFF     */
Adafruit_ICM20948 icm;
Adafruit_Sensor *accelerometer;
CircularBuffer<float,100> ICM_Accel_Readings;
float Processed_IMU_Readings[100];
bool IMU_On = false;



/*      VELOSTAT STUFF      */
bool Velostat_On = false;
CircularBuffer<uint16_t, 100> Velostat_Readings;
uint16_t Processed_Velostat_Readings[100];
int Velostat_Current = 10; //mA



/*      CODE AND OTHER STUFF      */


void setup() {

  // Setup serial.
  Serial.begin(115200);

  // Setup the IMU.
  Setup_IMU();

  // Setup the photodiodes.
  Setup_IR_Photodiodes();

  // Setup the Velostat.
  Setup_Velostat(10);

  // Setup the WiFi.
  SetupWiFi();


  //PoleStatus.Events = VelostatTriggered;


  // Set I2C to 400khz.
  Wire.setClock(400000);
}

void loop() {
  // Time the loop.
  auto loopStart = micros();
  
  // Run the photodiode stuff if turned on.
  if (IR_Beam_On) (Check_IR_Beam_Broken(nullptr, nullptr, 0)) ? PoleStatus.Events |= IRBeamTriggered : PoleStatus.Events ^= IRBeamTriggered;


  // Run the IMU stuff if turned on.
  if (IMU_On) (IMU_Loop()) ? PoleStatus.Events |= IMUTriggered : PoleStatus.Events ^= IMUTriggered;

  // Run the Velostat stuff if turned on.
  if (Velostat_On) (Velostat_Loop()) ? PoleStatus.Events |= VelostatTriggered : PoleStatus.Events ^= VelostatTriggered;

  // Time the loop.
  auto loopEnd = micros();
  delayMicroseconds( 10000 -loopEnd + loopStart);
  //Serial.println(loopEnd - loopStart);
}

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

bool IMU_Loop(){

  // Get updated values from IMU.
  sensors_event_t accel;
  accelerometer->getEvent(&accel);

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

    if (variance > PoleStatus.Settings.IMUSensitivity) return true;
    else return false;
  }
}

bool Velostat_Loop(){

  // Read the value in.
  uint16_t val = 0;//analogRead(34);

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

bool Check_IR_Beam_Broken(int* freqs, float* mags, int N){
  // Read the data from the I2S bus.
  i2s_adc_enable(I2S_NUM_0);
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

  // If the programmer wants an in order list of the highest amplitude frequencies present then we do this.
  if (N != 0){
    float test_mag = dominant_mag;
    float test_max_diff = test_mag;
    int test_max_diff_freq;

    // Loop to find N frequencies.
    for (int i = 0; i < N; i++){
      for (int j = 0; j < FFT_BUFF_LENGTH/2;j++){ // Loop over each item in the FFT output.

        if (test_mag - FFT_OutBuff[j] < test_max_diff){ // If the difference is less than the current smallest.
        test_max_diff = test_mag - FFT_OutBuff[j];
        test_max_diff_freq = j;
        }
      }

      freqs[i] = test_max_diff_freq * 1000 * 2;
      test_mag = FFT_OutBuff[test_max_diff_freq];
      mags[i] = FFT_OutBuff[test_max_diff_freq];
    }
  }

  // If within margin or error (the bin width) , 250Hz of the expected frequency then we probably have contact with the other pole.
  if (abs(dominant_freq - PoleStatus.Settings.IRTransmitFreq) <= 100) return true;
  else return false;
}

void Setup_IR_Photodiodes(){

  // Turn on the IR photodiodes by sending a voltage to them.
  dacWrite(26, 3300);
  
  // Setup FFT library.
  auto ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
  if (ret != ESP_OK){
    Serial.println("Error with Initialising FFT library.");
  }

  // Setup I2S DMA to do very very fast sampling of ADC
  // Config. Set sampling freq to 128kHz.
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

  // Initialise the inter IC sound bus to sample from ADC1 Channel 5. Also set attenuation of the pin to an appropriate value.
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_5);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

  // Try to determine what frequencies if any of the IR beam are currently detectable.
  float mags[40];
  int freqs[40];
  Check_IR_Beam_Broken(freqs, mags, 40);
}

void Turn_On_IR_Photodiodes(){
  // Turn on the IR photodiodes by sending a voltage to them.
  dacWrite(26, 3300);
}

void Turn_Off_IR_Photodiodes(){
  dacWrite(26, 0);
}

void SetupWiFi(){
  // Setup WiFi Connection.
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print(".");
  }
  Serial.println();

  // Print connection info to terminal.
  Serial.println("Connected To WiFi.");
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println((String)"RSSI: " + WiFi.RSSI());

  // Setup UDP socket for broadcast.
  if (udp_handshaker.connect(IPAddress(255,255,255,255), UDP_broadcast_Port)){
    udp_handshaker.onPacket([](AsyncUDPPacket packet){

      // Recieve UDP handshake reply from server and extract data from it.
      Serial.println("Recieved a UDP packet.");
      memcpy(&Session_TCP_Port, packet.data(), sizeof(uint32_t));
      Serial.println(Session_TCP_Port);
      Serial.println(packet.remoteIP().toString());
      Server_IP_String = packet.remoteIP().toString();
      Server_IP.fromString(Server_IP_String);
      
      UDP_broadcast_Successful = true;
    });
  }

  if (!udp_handshaker.listen(UDP_broadcast_Port-1)){
    Serial.println("Unable to setup UDP socket to listen for server reply.");
    exit(0);
  }

  while (!UDP_broadcast_Successful){
    // Loop over sending out a broadcast with the poles HWID & Pole type information.

    Serial.println("Sending UDP broadcast Now.");

    // Populate buffer with information.
    Comms_Send_Buffer[0] = POLE_HWID;
    Comms_Send_Buffer[8] = POLE_TYPE;

    // Send the UDP broadcast.
    udp_handshaker.broadcastTo((uint8_t*)&Comms_Send_Buffer, (size_t)512, (uint16_t)UDP_broadcast_Port);
    delay(1000);

  }

  udp_handshaker.close();

  if (udp_communicator.connect(Server_IP.fromString(Server_IP_String), Session_TCP_Port)){
    udp_communicator.onPacket(On_Packet);
    Serial.println("Setup UDP socket to listen for messages from server.");
  }

  if (!udp_communicator.listen(Session_TCP_Port)) {Serial.println("Unable to listen for incoming UDP packets.");}
  Serial.println("UDP handshake with server successful. long living connection established.");
}

void Setup_IMU(){
  if (!icm.begin_I2C(104)){ Serial.println("Failed to find ICM chip on I2C bus."); while (true) delay(10);}
  Serial.println("Found ICM chip!");
  icm.setAccelRateDivisor(0);

  accelerometer = icm.getAccelerometerSensor();
}

void Setup_Velostat(int Current){
  dacWrite(25, (255 * Current) / 3300);
}

void UpdatePowerState(){

  if (PoleStatus.Settings.powerState & pps::HighPower){
    // Turn the photodiodes on.
    if (!IR_Beam_On) Turn_On_IR_Photodiodes();
    // Turn the IMU on.
    if (!IMU_On) IMU_On = true;
    // Also turn on velostat.
    if (!Velostat_On) {Velostat_On = true; Setup_Velostat(10);}
  }

  if (PoleStatus.Settings.powerState & pps::MediumPower){
    // Turn the photodiodes off.
    if (IR_Beam_On) Turn_Off_IR_Photodiodes();
    // Turn the IMU on.
    if (!IMU_On) IMU_On = true;
    // Also turn off velostat.
    if (Velostat_On) {Velostat_On = false; Setup_Velostat(0);}
  }

  if (PoleStatus.Settings.powerState & pps::Hibernating){
    // Turn the photodiodes off.
    if (IR_Beam_On) Turn_Off_IR_Photodiodes();
    // Turn the IMU off.
    if (IMU_On) IMU_On = false;
    // Also turn off velostat.
    if (Velostat_On) {Velostat_On = false; Setup_Velostat(0);}
  }
}
