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
  connect_ccp()
}

void loop() {
  // put your main code here, to run repeatedly:
}


bool connect_ccp() {
  int last_sent = millis();
  byte[] cmd msg;
  byte[] input = new

  do {
    if (millis() - last_sent >= 200) {
      udp.beginPacket(ccp_ID, port);

      cmd = generateCommand("INIT");
      udp.write(cmd, sizeof(cmd));
      udp.endPacket();
      last_sent = millis();
    }

    udp.parse
  } while ();
}


byte[] generateCommand(Char[] msg) {
  jsonDocument doc;

  doc["message"] = msg;
  byte[] jsonString;

  //might need to manuly translate char array to byte array
  serializeJson(doc, jsonString);

  return jsonString;
}