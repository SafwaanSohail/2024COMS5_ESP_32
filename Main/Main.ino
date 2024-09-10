#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define ID 26 // Can be 26 or 27 for the 2 blade runners
#define local_IP "10.20.30.1" + ID

//Wifi Credentials
const char * ssid = "ENGG2K3K";
const char * pwd = "";

// ccp ip and port
const char * ccp_IP = "10.20.30.1";
const int port = 3000 + ID;

//UDP instance
WiFiUDP udp;

// Time tracking
unsigned long lastMessageTime = 0; // Timestamp of the last received message
unsigned long heartbeatInterval = 5000; // Time in milliseconds before assuming disconnect

void setup() {
  WiFi.config(local_IP);
  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {
    // Handle waiting for connection, e.g., flashing LED
  }

  udp.begin(port);
  connect_ccp();
}

void loop() {
  // Listen for incoming UDP packets
  processHeartbeat();

  // Other tasks
}

// Function to check for heartbeat messages
void processHeartbeat() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Read incoming packet
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }

    // Parse the received message
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, incomingPacket);

    if (!error) {
      const char* messageType = doc["type"];
      if (strcmp(messageType, "HEARTBEAT") == 0) {
        // Update the last message time if heartbeat is received
        lastMessageTime = millis();
      }
    }
  }

  // Check if the time since the last message exceeds the threshold
  if (millis() - lastMessageTime > heartbeatInterval) {
    // No heartbeat received in the interval; assume disconnected
    handleDisconnection();
  }
}

// Function to handle the disconnection
void handleDisconnection() {
  // Take action, e.g., reset the connection, alert the user, etc.
  Serial.println("Connection lost, trying to reconnect...");
  connect_ccp(); // Attempt reconnection
}

bool connect_ccp() {
  unsigned long lastSent = millis();
  char cmd[255];

  do {
    if (millis() - lastSent >= 200) {
      udp.beginPacket(ccp_IP, port);
      generateCommand("INIT", cmd);
      udp.write(cmd, strlen(cmd));
      udp.endPacket();
      lastSent = millis();
    }

    // Check for an incoming response (not shown in detail)
    if (udp.parsePacket()) {
      return true; // Assume connection established
    }

  } while (millis() - lastSent < 5000); // Timeout after 5 seconds

  return false; // Failed to connect
}

// Function to generate the command in JSON format
void generateCommand(const char* msg, char* jsonString) {
  StaticJsonDocument<200> doc;
  doc["message"] = msg;

  serializeJson(doc, jsonString, 255);
}
