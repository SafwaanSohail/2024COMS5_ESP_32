#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// UDP Configuration
WiFiUDP udp;
const char* CCP_IP = "10.20.30.1";  // Replace with the correct CCP IP
const int CCP_PORT = 3001;  // Example port, adjust based on your settings

// Pin Definitions
const int motorPin1 = 5;  // Forward motor control
const int motorPin2 = 18; // Reverse motor control
const int doorPin = 13;   // Door control pin (for opening/closing)
const int ledPin = 2;     // Status LED
const int trigPin = 4;    // Ultrasonic Trigger pin
const int echoPin = 16;   // Ultrasonic Echo pin

// Collision Avoidance Settings
const int safeDistance = 20;  // Safe distance in cm
long duration;
int distance;

// Variables for BR status
String brStatus = "STOPC";  // Initial status (stopped with doors closed)

// Setup function
void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(doorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Start by stopping the motor and closing doors
  stopMotor();
  closeDoors();

  // Connect to WiFi
  connectWiFi();

  // Initialize UDP
  udp.begin(CCP_PORT);
}

void loop() {
  // Check for incoming messages from CCP
  int packetSize = udp.parsePacket();
  if (packetSize) {
    handleCCPMessage();
  }

  // Perform collision avoidance
  distance = measureDistance();
  if (distance < safeDistance) {
    stopMotor();
    Serial.println("Obstacle detected. Stopping Blade Runner.");
  }
}

// Connect to WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

// Handle incoming CCP messages
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

  // Process command based on action from Table 2
  if (action == "STOPC") {
    stopMotor();
    closeDoors();
    brStatus = "STOPC";
  } else if (action == "STOPO") {
    stopMotor();
    openDoors();
    brStatus = "STOPO";
  } else if (action == "FSLOWC") {
    moveForwardSlow();
    brStatus = "FSLOWC";
  } else if (action == "FFASTC") {
    moveForwardFast();
    brStatus = "FFASTC";
  } else if (action == "RSLOWC") {
    moveBackwardSlow();
    brStatus = "RSLOWC";
  } else if (action == "DISCONNECT") {
    flashLED();
    brStatus = "OFLN";  // Offline status for disconnection
  }

  // Send status update back to CCP
  sendStatusUpdate();
}

// Function to measure distance using ultrasonic sensor
int measureDistance() {
  // Send a 10 microsecond pulse to the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the time for the echo pin to go HIGH
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance in cm (Speed of sound is ~343 m/s)
  int distanceCm = duration * 0.034 / 2;
  
  return distanceCm;
}

// Function to move Blade Runner forward slowly
void moveForwardSlow() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  Serial.println("Moving forward slowly.");
}

// Function to move Blade Runner forward fast
void moveForwardFast() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  Serial.println("Moving forward quickly.");
}

// Function to move Blade Runner backward slowly
void moveBackwardSlow() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  Serial.println("Moving backward slowly.");
}

// Function to stop the Blade Runner
void stopMotor() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  Serial.println("Blade Runner stopped.");
}

// Function to open doors
void openDoors() {
  digitalWrite(doorPin, HIGH);
  Serial.println("Doors opened.");
}

// Function to close doors
void closeDoors() {
  digitalWrite(doorPin, LOW);
  Serial.println("Doors closed.");
}

// Function to flash LED for disconnection
void flashLED() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(ledPin, HIGH);
    delay(250);
    digitalWrite(ledPin, LOW);
    delay(250);
  }
  Serial.println("Flashing LED for disconnection.");
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
