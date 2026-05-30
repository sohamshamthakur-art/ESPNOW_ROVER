
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

typedef struct {
  float x;
  float y;
  float z;
  float xa;
  float ya;
  float za;
} DataPacket;

DataPacket received;
Servo myServo;

/* L298N pins */
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

const int ENB = 14;
const int IN3 = 12;
const int IN4 = 13;

/* PWM */
const int freq = 5000;
const int resolution = 8;
const int channelA = 2;
const int channelB = 3;

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  ledcWrite(channelA, 0);
  ledcWrite(channelB, 0);
}

void reverseMotors(int speedVal) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(channelA, speedVal);
  ledcWrite(channelB, speedVal);
}

void forwardMotors(int speedVal) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(channelA, speedVal);
  ledcWrite(channelB, speedVal);
}

void onReceive(const uint8_t *mac,
               const uint8_t *incomingData,
               int len) {

  if (len != sizeof(DataPacket)) {
    Serial.println("Wrong packet size");
    return;
  }

  memcpy(&received, incomingData, sizeof(received));

  // SERVO: use Y tilt
  float servoAngle = atan2(received.y, received.z) * 180.0 / PI;
  int servoPos = map((int)servoAngle, 75, -75, 0, 180);
  servoPos = constrain(servoPos, 0, 180);
  myServo.write(servoPos);

  // MOTORS: use X tilt
  float motorAngle = atan2(received.x, received.z) * 180.0 / PI;

  const float deadband = 10.0;
  const float maxAngle = 30.0;

  if (motorAngle > -deadband && motorAngle < deadband) {
    stopMotors();
  } else {
    float absAngle = fabs(motorAngle);
    if (absAngle > maxAngle) absAngle = maxAngle;

    int speedVal = (int)((absAngle - deadband) * 255.0 / (maxAngle - deadband));
    speedVal = constrain(speedVal, 0, 255);

    if (motorAngle >= deadband) {
      forwardMotors(speedVal);
    } else {
      reverseMotors(speedVal);
    }
  }

  Serial.print("Servo Angle: ");
  Serial.print(servoAngle);
  Serial.print("  Servo Pos: ");
  Serial.print(servoPos);

  Serial.print("  Motor Angle: ");
  Serial.println(motorAngle);

  delay(10);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  myServo.attach(33);
  myServo.write(90);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(channelA, freq, resolution);
  ledcSetup(channelB, freq, resolution);
  ledcAttachPin(ENA, channelA);
  ledcAttachPin(ENB, channelB);

  stopMotors();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  Serial.println("Receiver ready");
}

void loop() {}