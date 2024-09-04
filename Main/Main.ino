#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define ID 26 // Can be 26 or 27 for the 2 blade runners
#define local_IP "10.20.30.1" + ID 

//Wifi Credentials
const char * ssid = "ENGG2K3K";
const char * pwd = "";


// ccp ip and port
const char * ccp_IP "10.20.30.1";
const int port = 3000 + ID;

//UDP instance
WiFiUDP udp;


void setup() {
  WiFi.config(local_IP);
  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {
    // do stuff
  }

  udp.begin(port);
}

void loop() {
  // put your main code here, to run repeatedly:

}
