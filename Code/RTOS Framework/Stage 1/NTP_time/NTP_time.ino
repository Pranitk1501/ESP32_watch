#include <WiFi.h>
#include "time.h"

const char* ssid       = "Pranit CE";
const char* password   = "1234567890";

const char* ntpServer = "asia.pool.ntp.org";
const long  gmtOffset_sec = 18900;
const int   daylightOffset_sec = 3600;


TaskHandle_t Timefetch;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void printTime(void *Paramerter)
{
  while(1)
  {
    printLocalTime();
    vTaskDelay(1000/portTICK_RATE_MS);
  }
}

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  xTaskCreatePinnedToCore(printTime, "Time Display", 4096, NULL, 0, &Timefetch, 1);

}

void loop() {
  // put your main code here, to run repeatedly:

}
