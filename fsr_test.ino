#include "driver/rtc_io.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ------------------- 사용자 설정 -------------------
const char* ssid = "*";       // 와이파이 이름
const char* password = "*"; // 와이파이 비밀번호

// 스프링 부트 서버의 주소. (PC의 IP 주소로 변경)
const char* serverUrl = "http://*:8080/api/fsr_data"; 
// ----------------------------------------------------

// GPIO 핀 매핑
#define FSR1_PIN 36  // ADC1_0
#define FSR2_PIN 39  // ADC1_3
#define FSR3_PIN 34  // ADC1_6
#define FSR4_PIN 35  // ADC1_7
#define FSR5_PIN 32  // ADC1_4
#define FSR6_PIN 33  // ADC1_5

void setup() {
  Serial.begin(115200);      // 시리얼 통신 속도 설정

  rtc_gpio_deinit(GPIO_NUM_32); // RTC 기능 해제, 일반 GPIO로 사용 
  rtc_gpio_deinit(GPIO_NUM_33);

  // 와이파이 연결 시작
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // ESP32의 IP 주소 출력
}

void loop() {
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

  // 시리얼 모니터에 현재 값 출력
  Serial.println("------------------------------");
  Serial.print("FSR 1 (GPIO36): "); Serial.print(fsr1_value); Serial.print(" | Voltage: "); Serial.println(voltage1);
  Serial.print("FSR 2 (GPIO39): "); Serial.print(fsr2_value); Serial.print(" | Voltage: "); Serial.println(voltage2);
  Serial.print("FSR 3 (GPIO34): "); Serial.print(fsr3_value); Serial.print(" | Voltage: "); Serial.println(voltage3);
  Serial.print("FSR 4 (GPIO35): "); Serial.print(fsr4_value); Serial.print(" | Voltage: "); Serial.println(voltage4);
  Serial.print("FSR 5 (GPIO32): "); Serial.print(fsr5_value); Serial.print(" | Voltage: "); Serial.println(voltage5);
  Serial.print("FSR 6 (GPIO33): "); Serial.print(fsr6_value); Serial.print(" | Voltage: "); Serial.println(voltage6);
  Serial.println("------------------------------");

  // JSON 데이터 생성 (전압 값을 담도록 수정)
  JsonDocument doc;
  doc["voltage1"] = voltage1;
  doc["voltage2"] = voltage2;
  doc["voltage3"] = voltage3;
  doc["voltage4"] = voltage4;
  doc["voltage5"] = voltage5;
  doc["voltage6"] = voltage6;

  String jsonString;
  serializeJson(doc, jsonString);

  // HTTP POST 요청 보내기
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    Serial.println("Sending data to server...");
    Serial.println(jsonString); // 보내는 JSON 내용 확인

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response from server: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected. No data sent.");
  }
  
  // 1초마다 데이터 전송
  delay(1000); 
}
