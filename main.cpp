#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

#define BUZZER_PIN 13  // Pin Positif Buzzer

const float ANGULAR_THRESHOLD = 20.0; 

const float TIME_THRESHOLD = 5.0; 

float target_angle = 0.0;       
float current_angle = 0.0;     
unsigned long bad_posture_start_time = 0; 
bool is_slouching = false;     

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("Sensor MPU6050 tidak terdeteksi! Cek kabel.");
    while (1) {
      digitalWrite(BUZZER_PIN, HIGH); delay(100);
      digitalWrite(BUZZER_PIN, LOW); delay(100);
    }
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G); 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);   

  Serial.println("=========================================");
  Serial.println("   SMART POSTURE CORRECTOR (ESP32)      ");
  Serial.println("=========================================");
  Serial.println("Silakan duduk TEGAK ");
  Serial.println("Sistem akan merekam posisi ini dalam 5 detik...");
  
  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
  
  delay(3000); 
  
  float sum_angle = 0;
  int samples = 100;
  for(int i=0; i<samples; i++){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    float angle = atan2(a.acceleration.y, a.acceleration.z) * 57.296;
    sum_angle += angle;
    delay(10);
  }
  target_angle = sum_angle / samples;
  
  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); delay(100);
  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);

  Serial.print("Kalibrasi Selesai! Sudut Tegak: ");
  Serial.println(target_angle);
  Serial.println("Sistem MONITORING AKTIF.");
}

void loop() {
  // 1. BACA SENSOR
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 2. HITUNG SUDUT SAAT INI
  float raw_angle = atan2(a.acceleration.y, a.acceleration.z) * 57.296;
  
  // 3. HITUNG DEVIASI
  float deviation = abs(raw_angle - target_angle);

  // 4. LOGIKA DETEKSI BUNGKUK 
  if (deviation > ANGULAR_THRESHOLD) {
    
    if (!is_slouching) {
      bad_posture_start_time = millis();
      is_slouching = true;
    } else {
      unsigned long duration = millis() - bad_posture_start_time;
      
      if (duration > (TIME_THRESHOLD * 1000)) {
        Serial.println("!!! PERINGATAN: POSTUR BURUK !!!");
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100); 
        digitalWrite(BUZZER_PIN, LOW);  
        delay(50);
      }
    }
    
  } else {
    is_slouching = false;
    digitalWrite(BUZZER_PIN, LOW); 
  }

  // 5. DEBUGGING DI TERMINAL
  Serial.print("Target: "); Serial.print(target_angle, 1);
  Serial.print(" | Current: "); Serial.print(raw_angle, 1);
  Serial.print(" | Deviasi: "); Serial.print(deviation, 1);
  
  if (is_slouching) {
    Serial.print(" [BUNGKUK!!!] ");
    Serial.print((millis() - bad_posture_start_time)/1000.0, 1);
    Serial.println("s]");
  } else {
    Serial.println(" [TEGAK]");
  }

  delay(50); 
}
