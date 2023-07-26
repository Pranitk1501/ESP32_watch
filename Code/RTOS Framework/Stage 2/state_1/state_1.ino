#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <TFT_eWidget.h>
#include "string.h"

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

TFT_eSPI tft = TFT_eSPI();  

const char* ssid = "robocon-ER";
const char* password = "12345678";

String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

#define NTP_OFFSET 19800 // In seconds (IST offset)
#define NTP_INTERVAL 60 * 1000 // In milliseconds
#define NTP_ADDRESS "time.google.com"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

int monthDay, currentMonth, currentYear;
String currentMonthName, weekDay, currentDate, formattedTime;

TaskHandle_t time_upd;
TaskHandle_t displayed;

static SemaphoreHandle_t mutex;

void time_update(void *paramerter)
{
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    formattedTime = timeClient.getFormattedTime();
    struct tm *ptm = localtime(&epochTime);

    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon + 1;
    currentMonthName = months[currentMonth - 1];
    currentYear = ptm->tm_year + 1900;
    weekDay = weekDays[ptm->tm_wday];

    xSemaphoreGive(mutex);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
  }
}

void display(void *paramerters)
{
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    tft.fillScreen(TFT_BLACK);
    tft.drawCentreString(formattedTime, 64, 50, 4);
    currentDate = String(monthDay) + "/" + String(currentMonth) + "/" + String(currentYear);
    tft.drawCentreString(currentDate, 64, 75, 2);
    tft.drawCentreString(weekDay, 64, 90, 2);

    Serial.print("Time: ");
    Serial.println(formattedTime);
    Serial.print("Date: ");
    Serial.print(currentDate);
    Serial.print(" (");
    Serial.print(weekDay);
    Serial.println(")");

    xSemaphoreGive(mutex);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
  }
}

void setup() {

  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  tft.fillRectVGradient(0, 0, 128, 160, TFT_BLACK, TFT_RED);
  tft.setTextColor(TFT_WHITE);

  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500)); // Delay 0.5 second
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  // Create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(time_update, "time", 4096, NULL, 1, &time_upd, app_cpu);
  xTaskCreatePinnedToCore(display, "display", 4096, NULL, 1, &displayed, app_cpu);
}

void loop() {
  // Nothing to be done in the loop for FreeRTOS
}
