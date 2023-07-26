//STATE 3 Steps and calories

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <TFT_eWidget.h>
#include "string.h"



TFT_eSPI tft = TFT_eSPI();  
Adafruit_MPU6050 mpu;


int steps = 4000;
int angle = 0;
int calories = 0;




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
