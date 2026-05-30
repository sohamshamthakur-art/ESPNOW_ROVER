#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

/* RECEIVER MAC */
uint8_t receiverMAC[] = {/*MAC ID*/};

Adafruit_MPU6050 mpu;

/* Data to send */
typedef struct {
  float x;
  float y;
  float z;
  float xa;
  float ya;
  float za;
} accel_data_t;

accel_data_t data;

/* ESP-NOW send callback */
void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SEND OK" : "SEND FAIL");
}

void setup() {
  Serial.begin(115200);

  Wire.begin(14, 15);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("MPU6050 found");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  WiFi.mode(WIFI_STA);

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Sender ready");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  data.x  = a.acceleration.x;
  data.y  = a.acceleration.y;
  data.z  = a.acceleration.z;

  data.xa = g.gyro.x;
  data.ya = g.gyro.y;
  data.za = g.gyro.z;

  esp_now_send(receiverMAC, (uint8_t *)&data, sizeof(data));

  delay(25);
}