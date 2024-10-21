#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// Pin Definitions for Ultrasonic Sensor
const int trigPin = 4;
const int echoPin = 16;

// Motor Pins
const int motorPin1 = 5;  // Forward
const int motorPin2 = 18; // Reverse

// Threshold distance (in cm) for collision avoidance
const int safeDistance = 20;

// Variables for timing the ultrasonic pulse
long duration;
int distance;

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Set up pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  // Start by stopping the motor
  stopMotor();
}

// Main loop
void loop() {
  // Measure distance using ultrasonic sensor
  distance = measureDistance();

  // If an obstacle is closer than the safe distance, stop the motor
  if (distance < safeDistance) {
    Serial.println("Obstacle detected! Stopping Blade Runner.");
    stopMotor();
  } else {
    // No obstacle detected, move forward
    Serial.println("No obstacle. Moving forward.");
    moveForward();
  }

  // Add a small delay to avoid excessive readings
  delay(100);
}

// Function to measure distance using the ultrasonic sensor
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

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  return distanceCm;
}

// Function to move the Blade Runner forward
void moveForward() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
}

// Function to stop the Blade Runner
void stopMotor() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
}
