#include "driver/rtc_io.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ------------------- 사용자 설정 -------------------
const char* ssid = "*";       // 와이파이 이름
const char* password = "*";               // 와이파이 비밀번호

// 원격 서버 주소 (AWS EC2)
const char* serverUrl = "http://*:8080/api/fsr_data";

// 깔창 구분 ("left" 또는 "right")
const char* insoleSide = "right";  

// ----------------------------------------------------
// GPIO 핀 매핑
#define FSR1_PIN 36  // ADC1_0
#define FSR2_PIN 39  // ADC1_3
#define FSR3_PIN 34  // ADC1_6
#define FSR4_PIN 35  // ADC1_7
#define FSR5_PIN 32  // ADC1_4
#define FSR6_PIN 33  // ADC1_5

void setup() {
  Serial.begin(115200);  // 시리얼 통신 속도 설정
  rtc_gpio_deinit(GPIO_NUM_32);  // RTC 기능 해제, 일반 GPIO로 사용
  rtc_gpio_deinit(GPIO_NUM_33);
}

void loop() {
  // WiFi 연결 상태 확인
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000);  // 재연결 대기
    return;
  }

  // 각 FSR에서 값 읽기 (0 ~ 4095)
  int fsr1_value = analogRead(FSR1_PIN);
  int fsr2_value = analogRead(FSR2_PIN);
  int fsr3_value = analogRead(FSR3_PIN);
  int fsr4_value = analogRead(FSR4_PIN);
  int fsr5_value = analogRead(FSR5_PIN);
  int fsr6_value = analogRead(FSR6_PIN);

  // 전압 값으로 변환 (ESP32의 ADC는 12비트, 3.3V 기준)
  float voltage1 = fsr1_value * (3.3 / 4095.0);
  float voltage2 = fsr2_value * (3.3 / 4095.0);
  float voltage3 = fsr3_value * (3.3 / 4095.0);
  float voltage4 = fsr4_value * (3.3 / 4095.0);
  float voltage5 = fsr5_value * (3.3 / 4095.0);
  float voltage6 = fsr6_value * (3.3 / 4095.0);

  // 총압력 계산 및 압력 비율(%) 구하기
  float totalVoltage = voltage1 + voltage2 + voltage3 + voltage4 + voltage5 + voltage6;

  float ratio1 = (totalVoltage > 0) ? (voltage1 / totalVoltage) * 100.0 : 0;
  float ratio2 = (totalVoltage > 0) ? (voltage2 / totalVoltage) * 100.0 : 0;
  float ratio3 = (totalVoltage > 0) ? (voltage3 / totalVoltage) * 100.0 : 0;
  float ratio4 = (totalVoltage > 0) ? (voltage4 / totalVoltage) * 100.0 : 0;
  float ratio5 = (totalVoltage > 0) ? (voltage5 / totalVoltage) * 100.0 : 0;
  float ratio6 = (totalVoltage > 0) ? (voltage6 / totalVoltage) * 100.0 : 0;

  // 시리얼 모니터 출력
  Serial.println("------------------------------");
  Serial.print("Insole Side: "); Serial.println(insoleSide);
  Serial.print("FSR 1 (GPIO36): "); Serial.print(fsr1_value); Serial.print(" | Voltage: "); Serial.print(voltage1); Serial.print(" V | Ratio: "); Serial.print(ratio1); Serial.println(" %");
  Serial.print("FSR 2 (GPIO39): "); Serial.print(fsr2_value); Serial.print(" | Voltage: "); Serial.print(voltage2); Serial.print(" V | Ratio: "); Serial.print(ratio2); Serial.println(" %");
  Serial.print("FSR 3 (GPIO34): "); Serial.print(fsr3_value); Serial.print(" | Voltage: "); Serial.print(voltage3); Serial.print(" V | Ratio: "); Serial.print(ratio3); Serial.println(" %");
  Serial.print("FSR 4 (GPIO35): "); Serial.print(fsr4_value); Serial.print(" | Voltage: "); Serial.print(voltage4); Serial.print(" V | Ratio: "); Serial.print(ratio4); Serial.println(" %");
  Serial.print("FSR 5 (GPIO32): "); Serial.print(fsr5_value); Serial.print(" | Voltage: "); Serial.print(voltage5); Serial.print(" V | Ratio: "); Serial.print(ratio5); Serial.println(" %");
  Serial.print("FSR 6 (GPIO33): "); Serial.print(fsr6_value); Serial.print(" | Voltage: "); Serial.print(voltage6); Serial.print(" V | Ratio: "); Serial.print(ratio6); Serial.println(" %");
  Serial.println("------------------------------");

  // JSON 데이터 생성
  JsonDocument doc;
  doc["side"] = insoleSide;
  doc["ratio1"] = ratio1;
  doc["ratio2"] = ratio2;
  doc["ratio3"] = ratio3;
  doc["ratio4"] = ratio4;
  doc["ratio5"] = ratio5;
  doc["ratio6"] = ratio6;

  String jsonString;
  serializeJson(doc, jsonString);

  // HTTP POST 요청
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  Serial.println("=== Sending data to server ===");
  Serial.print("Server URL: "); Serial.println(serverUrl);
  Serial.print("JSON Data: "); Serial.println(jsonString);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("HTTP Response code: "); Serial.println(httpResponseCode);
    Serial.print("Response from server: "); Serial.println(response);

    if (httpResponseCode == 200) {
      Serial.println("✅ Data sent successfully!");
    }
  } else {
    Serial.print("❌ Error on sending POST: ");
    Serial.println(httpResponseCode);
    Serial.print("Error details: ");
    if (httpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("Connection refused - Server may be down");
    } else if (httpResponseCode == HTTPC_ERROR_CONNECTION_LOST) {
      Serial.println("Connection lost");
    } else if (httpResponseCode == HTTPC_ERROR_READ_TIMEOUT) {
      Serial.println("Request timeout");
    } else {
      Serial.println("Unknown error");
    }
  }

  http.end();

  // 1초마다 데이터 전송
  delay(1000);
}
