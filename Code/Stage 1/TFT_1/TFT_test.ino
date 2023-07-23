#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>

const char* ssid     = "pranit";
const char* password = "1234567890";
TFT_eSPI tft = TFT_eSPI();  

#define NTP_OFFSET  19800 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "1.asia.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
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
  tft.setRotation(1);
  tft.setTextWrap(true, true);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  tft.setCursor(0, 0);
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  tft.println(formattedTime);
}
