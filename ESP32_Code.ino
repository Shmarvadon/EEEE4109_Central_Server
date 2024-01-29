
#include "WiFi.h"
#include "AsyncUDP.h"

const char* ssid = "network21iot";
const char* password = "RCD-M40DAB";

bool UDP_broadcast_Successful = false;
uint32_t Session_TCP_Port, Session_ID;
String Server_IP_String;
IPAddress Server_IP;

AsyncUDP udp;
WiFiClient tcp;

void setup() {

  // Setup serial.
  Serial.begin(115200);

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
  if (udp.connect(IPAddress(255,255,255,255), 6309)){
    udp.onPacket([](AsyncUDPPacket packet){

      // Recieve UDP handshake reply from server and extract data from it.
      Serial.println("Recieved a UDP packet.");
      //Serial.write(packet.data(), packet.length());
      memcpy(&Session_TCP_Port, packet.data(), sizeof(uint32_t));
      memcpy(&Session_ID, (void*)&packet.data()[3], sizeof(uint32_t));
      Serial.println(Session_TCP_Port);
      Serial.println(Session_ID);
      Serial.println(packet.remoteIP().toString());
      Server_IP_String = packet.remoteIP().toString();
      Server_IP.fromString(Server_IP_String);
      delay(50);

      // Attempt to setup TCP connection.
      if(tcp.connect(Server_IP, Session_TCP_Port)){
        Serial.println("TCP connection successful.");
      }
      else{
        Serial.println("TCP connection unsuccessful.");
      }
      UDP_broadcast_Successful = true;
    });
  }

    if (!udp.listen(6308)){
      Serial.println("Unable to setup UDP socket to listen for server reply.");
      exit(0);
  }

  while (!UDP_broadcast_Successful){
    // Send the UDP broadcast.
    udp.broadcastTo("{\"HWID\" : \"001\"}", 6309);

    delay(1000);

  }
  Serial.println("UDP handshake with server successful. TCP connection established.");

  tcp.print("Hello Server!!! I am an ESP32 communicating over TCP!!!!");
}

void loop() {

  if (tcp.available() > 0){
    Serial.print((char)tcp.read());

    if (tcp.available() == 0) {Serial.println();}
  }
}
