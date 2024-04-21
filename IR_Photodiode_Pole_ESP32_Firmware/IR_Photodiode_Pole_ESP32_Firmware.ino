
#include "WiFi.h"
#include "AsyncUDP.h"
#include <driver/adc.h>
#include <driver/i2s.h>
#include <dsps_fft2r.h>


/*      MISCELLANEOUS GLOBAL VARIABLES & DEFINES      */

#define POLE_HWID (uint64_t)0
#define POLE_TYPE (uint8_t)2


/*      NETWORK COMMS STUFF     */

const char* ssid = "network21iot";
const char* password = "RCD-M40DAB";

bool UDP_broadcast_Successful = false;
uint32_t UDP_broadcast_Port = 42069;

uint32_t Session_TCP_Port, Session_ID;
String Server_IP_String;
IPAddress Server_IP;
char TCP_RecvBuffer[512];

AsyncUDP udp;
WiFiClient tcp;

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
		StopEventFeed			=	0b10001000	// Stops  the realtime continuous streaming of scoring relevent data.
	};
}


/*      IR PHOTODIODE STUFF     */

#define FFT_BUFF_LENGTH 512
size_t bytes_read;
uint16_t DMA_InBuff[FFT_BUFF_LENGTH];
float FFT_InBuff[FFT_BUFF_LENGTH*2];
float FFT_OutBuff[FFT_BUFF_LENGTH/2];

float Detection_Freq;


/*      CODE AND OTHER STUFF      */


void setup() {

  // Setup serial.
  Serial.begin(115200);


  // Setup the WiFi.
  SetupWiFi();
  
  // Setup the photodiodes.
  Setup_IR_Photodiodes();

}

void loop() {


  if (tcp.available() > 0){
    // put the recieved packet data into buffer up to 512 bytes.
    for (int i = 0; i < 512; i++){
      if (tcp.available() > 0 )TCP_RecvBuffer[i] = (uint8_t)tcp.read();
      if (tcp.available() == 0) break;
    }

    Serial.println(TCP_RecvBuffer);

    if (TCP_RecvBuffer[0] & ptt::Ping){
      tcp.print((char)130);
      Serial.println("Recieved ping from server");
    }

    if (TCP_RecvBuffer[0] == (ptt::SyncToServer | ptt::Events)){
      Serial.println((int)TCP_RecvBuffer[0]);
      Serial.println("Server has asked for sync of events.");
    }

    for (int i = 0; i < 512; i++) TCP_RecvBuffer[i] = (char)0;
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
    if (FFT_OutBuff[i] > dominant_mag) {dominant_mag = FFT_OutBuff[i]; dominant_freq = (500*i);}
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

      freqs[i] = test_max_diff_freq * 500;
      test_mag = FFT_OutBuff[test_max_diff_freq];
      mags[i] = FFT_OutBuff[test_max_diff_freq];
    }
  }

  // If within margin or error (the bin width) , 250Hz of the expected frequency then we probably have contact with the other pole.
  if (abs(dominant_freq - Detection_Freq) <= 250) return true;
  else return false;
}

void Setup_IR_Photodiodes(){
  
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

  // Code here to tell server which channels are free to use and then try and locate master pole.

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
  if (udp.connect(IPAddress(255,255,255,255), UDP_broadcast_Port)){
    udp.onPacket([](AsyncUDPPacket packet){

      // Recieve UDP handshake reply from server and extract data from it.
      Serial.println("Recieved a UDP packet.");
      memcpy(&Session_TCP_Port, packet.data(), sizeof(uint32_t));
      memcpy(&Session_ID, (void*)&packet.data()[3], sizeof(uint32_t));
      Serial.println(Session_TCP_Port);
      Serial.println(Session_ID);
      Serial.println(packet.remoteIP().toString());
      Server_IP_String = packet.remoteIP().toString();
      Server_IP.fromString(Server_IP_String);
      
      UDP_broadcast_Successful = true;

      // Attempt to setup TCP connection.
      for (int i = 0; i < 20; i++){
        delay(100);
        if(tcp.connect(Server_IP, Session_TCP_Port)){
          Serial.println("TCP connection successful.");
          return;
        }
        else{
          Serial.println("TCP connection unsuccessful.");
        }
      }
      
    });
  }

  if (!udp.listen(UDP_broadcast_Port-1)){
    Serial.println("Unable to setup UDP socket to listen for server reply.");
    exit(0);
  }

  while (!UDP_broadcast_Successful){
    // Loop over sending out a broadcast with the poles HWID & Pole type information.

    Serial.println("Sending UDP broadcast Now.");

    // Populate buffer with information.
    TCP_RecvBuffer[0] = POLE_HWID;
    TCP_RecvBuffer[8] = POLE_TYPE;

    // Send the UDP broadcast.
    udp.broadcastTo((uint8_t*)&TCP_RecvBuffer, (size_t)512, (uint16_t)UDP_broadcast_Port);
    delay(1000);

  }
  Serial.println("UDP handshake with server successful. TCP connection established.");

  //udp.close();
}

