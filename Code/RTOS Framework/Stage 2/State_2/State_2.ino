#include <TFT_eSPI.h>
#include <Wire.h>
#include <SPI.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <TFT_eWidget.h>

// #include <FreeRTOS_ARM.h>
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

MAX30105 particleSensor;
TFT_eSPI tft = TFT_eSPI();

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

long irValue = 0;
SemaphoreHandle_t irValueMutex;

GraphWidget gr = GraphWidget(&tft);
TraceWidget tr = TraceWidget(&gr);

const float gxLow  = 0.0;
const float gxHigh = 100.0;
const float gyLow  = 0.0;
const float gyHigh = 100.0;

long irMin = 0;
long irMax = 0;
long irAvg = 0;

void readSensorTask(void* pvParameters) {
  for (;;) {
    // Acquire the mutex to protect the irValue variable
    xSemaphoreTake(irValueMutex, portMAX_DELAY);

    irValue = particleSensor.getIR();
    if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

    // Release the mutex
    xSemaphoreGive(irValueMutex);

    vTaskDelay(pdMS_TO_TICKS(20)); // Delay for 20 milliseconds
  }
}

void displayTask(void* pvParameters) {
  for (;;) {
    // Acquire the mutex to access the irValue variable
    xSemaphoreTake(irValueMutex, portMAX_DELAY);

    tft.fillRect(0, 0, 128, 60, 0x9fd4);

  char beatAvgstr[10];
  itoa(beatAvg, beatAvgstr, 10);
  if (irValue < 50000) {
    tft.drawCentreString("No finger?", 64, 20, 2);
  } 
  else {
    tft.drawCentreString(beatAvgstr, 64, 20, 4);
  }
  tft.drawCentreString("Heart Rate", 64, 50, 2);
  delay(20);

  static float gx = 0.0, gy = 0.0;
  static float delta = 10.0;
  tr.addPoint(gx, gy);
  gx += 1.0;
  //gy = ((irValue - 90000.0) / 6200.0) * 100;
  gy = ((irValue - irMin) / 500.0) * 100.0;
  irAvg = (irAvg + irValue) / 2;
  if (gx > gxHigh) {
      gx = 0.0;
      gr.drawGraph(14, 80);
      tr.startTrace(TFT_BLACK);
      irMax = irAvg + 250;
      irMin = irAvg - 250;
    }
    Serial.println(irValue);

    // Release the mutex
    xSemaphoreGive(irValueMutex);

    vTaskDelay(pdMS_TO_TICKS(20)); // Delay for 20 milliseconds
  }
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.fillScreen(0x9fd4);
  tft.setRotation(0);
  tft.setTextWrap(true, true);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);

  //oxy_init

  Serial.println("Initializing Oxymeter...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  // Mutex initialization
  irValueMutex = xSemaphoreCreateMutex();

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
