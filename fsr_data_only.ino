#include "driver/rtc_io.h"

// GPIO 핀 매핑
#define FSR1_PIN 36  // ADC1_0
#define FSR2_PIN 39  // ADC1_3
#define FSR3_PIN 34  // ADC1_6
#define FSR4_PIN 35  // ADC1_7
#define FSR5_PIN 32  // ADC1_4
#define FSR6_PIN 33  // ADC1_5

void setup() {
  // 시리얼 통신 속도 설정 (115200 bps)
  Serial.begin(115200);
  Serial.println("\n--- FSR 센서 데이터 측정 시작 ---");

  // GPIO 32와 33번 핀은 기본적으로 RTC(Deep Sleep 등) 기능에 사용될 수 있으므로,
  // 일반 ADC 핀으로 사용하기 위해 RTC 기능을 명시적으로 해제합니다.
  rtc_gpio_deinit(GPIO_NUM_32); 
  rtc_gpio_deinit(GPIO_NUM_33);

  // WiFi 관련 코드는 제거
}

void loop() {
  // 1. 각 FSR에서 아날로그 값 읽기 (0 ~ 4095)
  // ESP32의 ADC는 12비트(0~4095) 해상도를 가집니다.
  int fsr1_value = analogRead(FSR1_PIN);
  int fsr2_value = analogRead(FSR2_PIN);
  int fsr3_value = analogRead(FSR3_PIN);
  int fsr4_value = analogRead(FSR4_PIN);
  int fsr5_value = analogRead(FSR5_PIN);
  int fsr6_value = analogRead(FSR6_PIN);

  // 2. 아날로그 값을 전압 값으로 변환 (3.3V 기준)
  float voltage1 = fsr1_value * (3.3 / 4095.0);
  float voltage2 = fsr2_value * (3.3 / 4095.0);
  float voltage3 = fsr3_value * (3.3 / 4095.0);
  float voltage4 = fsr4_value * (3.3 / 4095.0);
  float voltage5 = fsr5_value * (3.3 / 4095.0);
  float voltage6 = fsr6_value * (3.3 / 4095.0);

  // 3. 총 전압 및 압력 비율 계산
  // 모든 센서의 전압 값을 더해 순간 총압력 (총 전압) 계산
  float totalVoltage = voltage1 + voltage2 + voltage3 + voltage4 + voltage5 + voltage6;

  // 각 센서가 총압력에서 차지하는 비율(%) 계산
  // (totalVoltage가 0일 때 0으로 나누는 것을 방지하기 위한 조건문 포함)
  float ratio1 = (totalVoltage > 0.01) ? (voltage1 / totalVoltage) * 100.0 : 0.0;
  float ratio2 = (totalVoltage > 0.01) ? (voltage2 / totalVoltage) * 100.0 : 0.0;
  float ratio3 = (totalVoltage > 0.01) ? (voltage3 / totalVoltage) * 100.0 : 0.0;
  float ratio4 = (totalVoltage > 0.01) ? (voltage4 / totalVoltage) * 100.0 : 0.0;
  float ratio5 = (totalVoltage > 0.01) ? (voltage5 / totalVoltage) * 100.0 : 0.0;
  float ratio6 = (totalVoltage > 0.01) ? (voltage6 / totalVoltage) * 100.0 : 0.0;
  
  // 4. 시리얼 모니터에 현재 값 출력
  Serial.println("----------------------------------------");
  Serial.print("Total Voltage: "); Serial.print(totalVoltage, 3); Serial.println(" V"); // 소수점 셋째 자리까지 출력
  Serial.println("----------------------------------------");
  
  // FSR 1 데이터 출력
  Serial.print("FSR 1 (GPIO36): Raw="); Serial.print(fsr1_value); 
  Serial.print(" | Voltage="); Serial.print(voltage1, 3); Serial.print(" V"); 
  Serial.print(" | Ratio="); Serial.print(ratio1, 1); Serial.println(" %"); // 소수점 첫째 자리까지 출력

  // FSR 2 데이터 출력
  Serial.print("FSR 2 (GPIO39): Raw="); Serial.print(fsr2_value); 
  Serial.print(" | Voltage="); Serial.print(voltage2, 3); Serial.print(" V"); 
  Serial.print(" | Ratio="); Serial.print(ratio2, 1); Serial.println(" %");

  // FSR 3 데이터 출력
  Serial.print("FSR 3 (GPIO34): Raw="); Serial.print(fsr3_value); 
  Serial.print(" | Voltage="); Serial.print(voltage3, 3); Serial.print(" V"); 
  Serial.print(" | Ratio="); Serial.print(ratio3, 1); Serial.println(" %");

  // FSR 4 데이터 출력
  Serial.print("FSR 4 (GPIO35): Raw="); Serial.print(fsr4_value); 
  Serial.print(" | Voltage="); Serial.print(voltage4, 3); Serial.print(" V"); 
  Serial.print(" | Ratio="); Serial.print(ratio4, 1); Serial.println(" %");

  // FSR 5 데이터 출력
  Serial.print("FSR 5 (GPIO32): Raw="); Serial.print(fsr5_value); 
  Serial.print(" | Voltage="); Serial.print(voltage5, 3); Serial.print(" V"); 
  Serial.print(" | Ratio="); Serial.print(ratio5, 1); Serial.println(" %");

  // FSR 6 데이터 출력
  Serial.print("FSR 6 (GPIO33): Raw="); Serial.print(fsr6_value); 
  Serial.print(" | Voltage="); Serial.print(voltage6, 3); Serial.print(" V"); 
  Serial.print(" | Ratio="); Serial.print(ratio6, 1); Serial.println(" %");
  Serial.println("----------------------------------------");
  
  // HTTP 통신 및 JSON 관련 코드는 모두 제거

  // 1초마다 데이터 측정 및 출력
  delay(1000); 
}