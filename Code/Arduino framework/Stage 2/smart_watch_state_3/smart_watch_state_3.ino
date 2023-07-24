//STATE 3 Steps and calories

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <TFT_eWidget.h>
#include "string.h"

MAX30105 particleSensor;

TFT_eSPI tft = TFT_eSPI();  
Adafruit_MPU6050 mpu;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 

float beatsPerMinute;
int beatAvg;

int steps = 4000;
int angle = 0;
int calories = 0;

const char* ssid     = "pranit";
const char* password = "1234567890";

String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

#define NTP_OFFSET  19800 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "time.google.com"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);

  if (!mpu.begin()) {
  Serial.println("Sensor init failed");
  while (1)
    yield();
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  if((a.acceleration.x > 6) || (a.acceleration.x < -6)) {
    steps++;
  }
  tft.fillScreen(TFT_BLACK);
  tft.drawSmoothArc(64, 60, 54, 48, 0, 360, TFT_WHITE, TFT_WHITE, true);
  angle = (steps / 10000.0) * 360;
  calories = 0.042985 * steps; //for average age of 22 Male, 77kg and 177 cm height
  tft.drawSmoothArc(64, 60, 53, 49, 0, angle, TFT_BLUE, TFT_BLUE, true);
  tft.drawCentreString(String(steps), 64, 40, 4);
  tft.drawCentreString("Steps", 64, 70, 2);
  String cal = "Calories: " + String(calories) + " kcal";
  tft.drawCentreString(cal, 64, 120, 2);
  String distance = "Distance: " + String(steps * 0.0009) + " km";
  tft.drawCentreString(distance, 64, 140, 2);
  delay(1000);
}
