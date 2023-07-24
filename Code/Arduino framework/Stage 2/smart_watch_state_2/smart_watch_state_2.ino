//STATE 2 Heart Rate and OXY readings

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

MAX30105 particleSensor;

TFT_eSPI tft = TFT_eSPI();  
Adafruit_MPU6050 mpu;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 

float beatsPerMinute;
int beatAvg;

const char* ssid     = "pranit";
const char* password = "1234567890";

#define NTP_OFFSET  19800 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "1.asia.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

GraphWidget gr = GraphWidget(&tft);
TraceWidget tr = TraceWidget(&gr);

const float gxLow  = 0.0;
const float gxHigh = 100.0;
const float gyLow  = 0.0;
const float gyHigh = 100.0;

long irMin = 0;
long irMax = 0;
long irAvg = 0;

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
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) 
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0); 

  //graph_init

  gr.createGraph(100, 40, 0x9fd4);
  gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);
  tr.startTrace(TFT_BLACK);
  gr.setGraphGrid(gxLow, 101.0, gyLow, 101.0, TFT_BLACK); 
  gr.drawGraph(14, 80);

}

void loop() {
  //tft.fillScreen(0x9fd4);
  tft.fillRect(0, 0, 128, 60, 0x9fd4);
  long irValue = particleSensor.getIR();

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
}


//intialise a maximum and minimum IR value reading then use that to normalise the gy value to be plotted or use some ort of calculus 
