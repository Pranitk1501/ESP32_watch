//STATE 1 Time and Date

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

const char* ssid     = "S.B";
const char* password = "sahil@123";

String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

#define NTP_OFFSET  19800 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "time.google.com"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void setup() {
  Serial.begin(115200); 

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();

  tft.init();
  tft.setRotation(0);
  tft.fillRectVGradient(0, 0, 128, 160, TFT_BLACK, TFT_RED);
  tft.setTextColor(TFT_WHITE);

}

void loop() {
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;
  String weekDay = weekDays[timeClient.getDay()];

  tft.drawCentreString(formattedTime, 64, 50, 4);
  String currentDate = String(monthDay) + "/" + String(currentMonth) + "/" + String(currentYear);
  tft.drawCentreString(currentDate, 64, 75, 2);
  tft.drawCentreString(weekDay, 64, 90, 2);
  delay(500);
  tft.fillRectVGradient(0, 0, 128, 160, TFT_BLACK, TFT_RED);
  Serial.println(formattedTime);
}
