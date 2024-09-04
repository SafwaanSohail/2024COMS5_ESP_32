#define motor_pin 25
#define dir_pin 24

#define max_speed 150

void setup() {
  pinMode(motor_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
}

void loop() {
  // going forward
  digitalWrite(dir_pin, HIGH);
  for (int i = 0; i < max_speed; i++) {
    analogWrite(motor_pin, i);
  }
  delay(2000);

  // going backwards
  digitalWrite(dir_pin, LOW);
  for (int i = 0; i < max_speed; i++) {
    analogWrite(motor_pin, i);
  }
  delay(2000);

}
