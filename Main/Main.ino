#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "ENGG2K3K";
const char* password = "";


#define ID 26
#define Local_IP "10.20.30.1.26"

#define Fast 200
#define Slow 50
#define Stop 0

// UDP Configuration
WiFiUDP udp;
const char* CCP_IP = "10.20.30.1";  // Replace with the correct CCP IP
const int CCP_PORT = 3000 + ID;  // Example port, adjust based on your settings

// Pin Definitions
const int motorPwmPin = 24;  // Forward motor control
const int motorDirPin = 23; // Reverse motor control
const int led1Pin = 7;     // Status LED
const int led2Pin = 8;
const int led3Pin = 9;


const int trigPin = 13;    // Ultrasonic Trigger pin
const int echoPin = 12;   // Ultrasonic Echo pin
const int IRPTPin = 14;


// Collision Avoidance Settings
const int safeDistance = 20;  // Safe distance in cm
long duration;
int distance;

// Command variables based on Table 2
const String CMD_STOPC = "STOPC";      // Stop and close doors
const String CMD_STOPO = "STOPO";      // Stop and open doors
const String CMD_FSLOWC = "FSLOWC";    // Move forward slowly and keep doors closed
const String CMD_FFASTC = "FFASTC";    // Move forward fast with doors closed
const String CMD_RSLOWC = "RSLOWC";    // Move backward slowly and keep doors closed
const String CMD_DISCONNECT = "DISCONNECT";  // Flash LED and indicate removal from track

// Variables for BR status
String brStatus = "STOPC";  // Initial status (stopped with doors closed)

// Setup function
void setup() {
  Serial.begin(115200);
  Serial.println("Serial Open at 115200");

  // Initialize pins
  Serial.println("Attaching Pins");
  pinMode(motorPwmPin, OUTPUT);
  pinMode(motorDirPin, OUTPUT);
  //pinMode(led1Pin, OUTPUT);
  //pinMode(led2Pin, OUTPUT);
  //pinMode(led3Pin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Start by stopping the motor and closing doors
  //stopMotor();
  //closeDoors();

  // Connect to WiFi
  connectWiFi();

  // Initialize UDP
  udp.begin(CCP_PORT);

  initaliseCcpCON();
}

void loop() {
  // Check for incoming messages from CCP
  int packetSize = udp.parsePacket();
  if (packetSize) {
    handleCCPMessage();
  }

  // Perform collision avoidance
  // distance = measureDistance();
  // if (distance < safeDistance) {
  //   stopMotor();
  //   brStatus = CMD_STOPC;
  //   Serial.println("Obstacle detected. Stopping Blade Runner.");
  // }
}

// Connect to WiFi
void connectWiFi() {
  Serial.println("Attempting Wifi Connection");
  WiFi.config(Local_IP);
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
  
  String action = doc["message"];
  Serial.println("Incomming :" + action);

  // Process command based on action from Table 2
  if (action == CMD_STOPC) {
    stopMotor();
    //closeDoors();
    brStatus = CMD_STOPC;
  } else if (action == CMD_STOPO) {
    stopMotor();
    //openDoors();
    brStatus = CMD_STOPO;
  } else if (action == CMD_FSLOWC) {
    moveForwardSlow();
    brStatus = CMD_FSLOWC;
  } else if (action == CMD_FFASTC) {
    moveForwardFast();
    brStatus = CMD_FFASTC;
  } else if (action == CMD_RSLOWC) {
    moveBackwardSlow();
    brStatus = CMD_RSLOWC;
  } else if (action == CMD_DISCONNECT) {
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
  digitalWrite(motorDirPin, HIGH);
  analogWrite(motorPwmPin, Slow);
  Serial.println("Moving forward slowly.");
}

// Function to move Blade Runner forward fast
void moveForwardFast() {
  digitalWrite(motorDirPin, HIGH);
  analogWrite(motorPwmPin, Fast);
  Serial.println("Moving forward quickly.");
}

// Function to move Blade Runner backward slowly
void moveBackwardSlow() {
  digitalWrite(motorDirPin, LOW);
  analogWrite(motorPwmPin, Slow);
  Serial.println("Moving backward slowly.");
}

// Function to stop the Blade Runner
void stopMotor() {
  analogWrite(motorPwmPin, Stop);
  Serial.println("Blade Runner stopped.");
}

// // Function to open doors
// void openDoors() {
//   digitalWrite(doorPin, HIGH);
//   Serial.println("Doors opened.");
// }

// // Function to close doors
// void closeDoors() {
//   digitalWrite(doorPin, LOW);
//   Serial.println("Doors closed.");
// }

// Function to flash LED for disconnection
void flashLED() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(led1Pin, HIGH);
    delay(250);
    digitalWrite(led1Pin, LOW);
    delay(250);
  }
  Serial.println("Flashing LED for disconnection.");
}

// Send status update to CCP
void sendStatusUpdate() {
  DynamicJsonDocument doc(256);
  doc["client_type"] = "CCP";
  doc["message"] = "STAT";
  doc["client_id"] = "BR" + ID;
  doc["status"] = brStatus;

  uint8_t buffer[256];
  size_t n = serializeJson(doc, buffer);

  udp.beginPacket(CCP_IP, CCP_PORT);
  udp.write(buffer, n);
  udp.endPacket();
}


void initaliseCcpCON() {
  int last_sent = millis();

  while(true) {
    if (millis() - last_sent >= 1000) {
      Serial.println("Sending INIT Request");
      DynamicJsonDocument doc(256);
      doc["client_type"] = "CCP";
      doc["message"] = "INIT";
      doc["client_id"] = "BR" + ID;

      uint8_t buffer[256];
      size_t n = serializeJson(doc, buffer);
      udp.beginPacket(CCP_IP, CCP_PORT);
      udp.write(buffer, n);
      udp.endPacket();

      last_sent = millis();
    }

    int packetSize = udp.parsePacket();

    if (packetSize) {
      uint8_t incomingPacket[255];
      int len = udp.read(incomingPacket, 255);

      if (len > 0) {
          incomingPacket[len] = '\0';
      }

      // Parse JSON message
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, incomingPacket);

      if(doc["message"] == "INIT") {
        Serial.println("Connected to ccp");
        break;
      }
    }
  }
}