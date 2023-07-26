#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <TFT_eWidget.h>
#include "string.h"
// #include <FreeRTOS_ARM.h>

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

TFT_eSPI tft = TFT_eSPI();
Adafruit_MPU6050 mpu;

int steps = 4000;
int angle = 0;
int calories = 0;

// Mutex for protecting steps variable
SemaphoreHandle_t stepsMutex;

void readSensorTask(void* pvParameters) {
  for (;;) {
    // Acquire the mutex to protect the steps variable
    xSemaphoreTake(stepsMutex, portMAX_DELAY);

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    if ((a.acceleration.x > 6) || (a.acceleration.x < -6)) {
      steps++;
    }

    // Release the mutex
    xSemaphoreGive(stepsMutex);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
  }
}

void displayTask(void* pvParameters) {
  for (;;) {
    // Acquire the mutex to access the steps variable
    xSemaphoreTake(stepsMutex, portMAX_DELAY);

    tft.fillScreen(TFT_BLACK);
    tft.drawSmoothArc(64, 60, 54, 48, 0, 360, TFT_WHITE, TFT_WHITE, true);
    angle = (steps / 10000.0) * 360;
    calories = 0.042985 * steps; // for average age of 22 Male, 77kg, and 177 cm height
    tft.drawSmoothArc(64, 60, 53, 49, 0, angle, TFT_BLUE, TFT_BLUE, true);
    tft.drawCentreString(String(steps), 64, 40, 4);
    tft.drawCentreString("Steps", 64, 70, 2);
    String cal = "Calories: " + String(calories) + " kcal";
    tft.drawCentreString(cal, 64, 120, 2);
    String distance = "Distance: " + String(steps * 0.0009) + " km";
    tft.drawCentreString(distance, 64, 140, 2);

    // Release the mutex
    xSemaphoreGive(stepsMutex);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
  }
}

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

  // Create the mutex
  stepsMutex = xSemaphoreCreateMutex();

  // Create tasks
  xTaskCreatePinnedToCore(
      readSensorTask,   // Function that implements the task
      "ReadSensorTask", // Name of the task
      2048,             // Stack size (words, not bytes)
      NULL,             // Task input parameter
      1,                // Priority
      NULL,             // Task handle
      app_cpu);

  xTaskCreatePinnedToCore(
      displayTask,      // Function that implements the task
      "DisplayTask",    // Name of the task
      4096,             // Stack size (words, not bytes)
      NULL,             // Task input parameter
      1,                // Priority
      NULL,             // Task handle
      app_cpu);
}

void loop() {
  // Empty loop. All processing is done in tasks.
}
