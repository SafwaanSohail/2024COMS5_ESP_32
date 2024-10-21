#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "ENGG2K3K";
const char* password = "";

// UDP and IP Configuration
WiFiUDP udp;
const char* CCP_IP = "10.20.30.1";
const int CCP_PORT = 3001; // Example port, adjust based on the IP table

// Status Variables
String brStatus = "STOPC"; // Blade Runner status

// GPIO pins for motor, sensors, etc.
const int motorPin1 = 5;
const int motorPin2 = 18;
const int irSensorPin = 34;
const int ledPin = 2; // For status indication

void setup() {
  Serial.begin(115200);
  
  // Setup motor, LED, IR sensor
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  
  // Connect to WiFi
  connectWiFi();

  // Initialize UDP
  udp.begin(CCP_PORT);
}

void loop() {
  // Handle incoming messages
  int packetSize = udp.parsePacket();
  if (packetSize) {
    handleCCPMessage();
  }

  // Handle sensors, movement, and actions
  checkIRSensor();
}

// Connect to WiFi function
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

// Handle incoming messages from CCP
void handleCCPMessage() {
  char incomingPacket[255];
  int len = udp.read(incomingPacket, 255);
  if (len > 0) {
    incomingPacket[len] = '\0';
  }
  
  // Parse JSON message
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, incomingPacket);
  
  String action = doc["action"];
  if (action == "FSLOWC") {
    moveForwardSlow();
  } else if (action == "STOPC") {
    stopBR();
  } // Add more commands as per Table 2 from the document
}

// Move Blade Runner forward slowly
void moveForwardSlow() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  brStatus = "FSLOWC";
  sendStatusUpdate();
}

// Stop Blade Runner
void stopBR() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  brStatus = "STOPC";
  sendStatusUpdate();
}

// Send status update to CCP
void sendStatusUpdate() {
  DynamicJsonDocument doc(256);
  doc["client_type"] = "CCP";
  doc["message"] = "STAT";
  doc["client_id"] = "BR01";  // Example BR ID
  doc["status"] = brStatus;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  
  udp.beginPacket(CCP_IP, CCP_PORT);
  udp.write(buffer, n);
  udp.endPacket();
}

// Check IR Sensor for station alignment
void checkIRSensor() {
  if (digitalRead(irSensorPin) == HIGH) {
    Serial.println("Station aligned.");
    // Perform actions for alignment
  }
}
