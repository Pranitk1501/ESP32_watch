#define configCHECK_FOR_STACK_OVERFLOW 2 // added to check if any stack was overflowed in any task which leads to reboot


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
// using only 1 core
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// dispaly and sensor init
MAX30105 particleSensor;
TFT_eSPI tft = TFT_eSPI();  
Adafruit_MPU6050 mpu;

// wifi credentials
const char* ssid = "robocon-ER";
const char* password = "12345678";

String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

#define NTP_OFFSET 19800 // In seconds (IST offset)
#define NTP_INTERVAL 60 * 1000 // In milliseconds
#define NTP_ADDRESS "time.google.com"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
//state 1 variables;
int monthDay, currentMonth, currentYear;
String currentMonthName, weekDay, currentDate, formattedTime;


// max302 variables init
const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg;
long irValue = 0;
long irMin = 0;
long irMax = 0;
long irAvg = 0;


//graph widget init
GraphWidget gr = GraphWidget(&tft);
TraceWidget tr = TraceWidget(&gr);
const float gxLow  = 0.0;
const float gxHigh = 100.0;
const float gyLow  = 0.0;
const float gyHigh = 100.0;

//mpu variables
int steps = 4000;
int angle = 0;
int calories = 0;

//timer and ISR lock variables
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
static hw_timer_t *timer = NULL;

// button variables
int taskNumber = 1;
int pinValue = 0;
#define PIN_INPUT 23
#define TIMER_INTERVAL_MS 5000

// mutexes and semaphores
SemaphoreHandle_t taskSemaphore;

void tft_init()
{
  tft.init();
  tft.setRotation(0);
  tft.fillRectVGradient(0, 0, 128, 160, TFT_BLACK, TFT_RED);
  tft.setTextColor(TFT_WHITE);
}

void tft_set(int a, int b)
{
  tft.fillScreen(a);
  tft.setRotation(0);
  tft.setTextColor(b);
  tft.setCursor(0, 0);
}

void graph_init()
{
  gr.createGraph(100, 40, 0x9fd4);
  gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);
  tr.startTrace(TFT_BLACK);
  gr.setGraphGrid(gxLow, 101.0, gyLow, 101.0, TFT_BLACK); 
}

void mux302_init()
{
  Serial.println("Initializing Oxymeter...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
}

void mpu_init()
{
   if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    while (1)
      yield();
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void wifi_connect()
{
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

}

void start_init()
{
  static uint32_t index = 0;
  for(int j = 160; j >= 0; j=j-5) {
    uint16_t fg_color = rainbow(index);
    uint16_t xcen = tft.width() / 2; 
    uint16_t ycen = tft.height() / 2;
    tft.drawSmoothCircle(xcen, ycen, j, fg_color, TFT_BLACK);
    index += 5;
    delay(100);
  }
}

unsigned int rainbow(byte value)
{

  value = value%192;
  
  byte red   = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0; // Green is the middle 6 bits, but only top 5 bits used here
  byte blue  = 0; // Blue is the bottom 5 bits

  byte sector = value >> 5;
  byte amplit = value & 0x1F;

  switch (sector)
  {
    case 0:
      red   = 0x1F;
      green = amplit; // Green ramps up
      blue  = 0;
      break;
    case 1:
      red   = 0x1F - amplit; // Red ramps down
      green = 0x1F;
      blue  = 0;
      break;
    case 2:
      red   = 0;
      green = 0x1F;
      blue  = amplit; // Blue ramps up
      break;
    case 3:
      red   = 0;
      green = 0x1F - amplit; // Green ramps down
      blue  = 0x1F;
      break;
    case 4:
      red   = amplit; // Red ramps up
      green = 0;
      blue  = 0x1F;
      break;
    case 5:
      red   = 0x1F;
      green = 0;
      blue  = 0x1F - amplit; // Blue ramps down
      break;
  }
  return red << 11 | green << 6 | blue;
}

void State_1_1(void *paramerter)
{
  while (1) {
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    if(taskNumber==1){
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    formattedTime = timeClient.getFormattedTime();
    struct tm *ptm = localtime(&epochTime);

    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon + 1;
    currentMonthName = months[currentMonth - 1];
    currentYear = ptm->tm_year + 1900;
    weekDay = weekDays[ptm->tm_wday];
    }
    xSemaphoreGive(taskSemaphore);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
  }
}

void State_1_2(void *paramerters)
{
  while (1) {
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    if(taskNumber==1){
    tft.fillScreen(TFT_BLACK);
    tft.drawCentreString(formattedTime, 64, 50, 4);
    currentDate = String(monthDay) + "/" + String(currentMonth) + "/" + String(currentYear);
    tft.drawCentreString(currentDate, 64, 75, 2);
    tft.drawCentreString(weekDay, 64, 90, 2);

    // Serial.print("Time: ");
    // Serial.println(formattedTime);
    // Serial.print("Date: ");
    // Serial.print(currentDate);
    // Serial.print(" (");
    // Serial.print(weekDay);
    // Serial.println(")");
    }
    xSemaphoreGive(taskSemaphore);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
  }
}

void State_2_1(void* pvParameters) {
  while (1) {
    // Acquire the mutex to protect the irValue variable
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    if(taskNumber==2){
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
    }
    // Release the mutex
    xSemaphoreGive(taskSemaphore);

    vTaskDelay(pdMS_TO_TICKS(20)); // Delay for 20 milliseconds
  }
}

void State_2_2(void* pvParameters) {
 while (1) {
    // Acquire the mutex to access the irValue variable
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    if(taskNumber==2){
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
    // Serial.println(irValue);
    }
    // Release the mutex
    xSemaphoreGive(taskSemaphore);

    vTaskDelay(pdMS_TO_TICKS(20)); // Delay for 20 milliseconds
  }
}

void State_3_1(void* pvParameters) {
  while (1) {
    // Acquire the mutex to access the steps variable
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    if(taskNumber==3){
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
    Serial.print(cal);
    Serial.print("  ");
    Serial.println(distance);
    }
    // Release the mutex
    xSemaphoreGive(taskSemaphore);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
  }
}
void State_3_2(void* pvParameters) {
 while (1) {
    // Acquire the mutex to protect the steps variable
    xSemaphoreTake(taskSemaphore, portMAX_DELAY);
    if(taskNumber==3){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    if ((a.acceleration.x > 6) || (a.acceleration.x < -6)) {
      steps++;
    }
    }
    // Release the mutex
    xSemaphoreGive(taskSemaphore);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
  }
}


// timer to isr function
void IRAM_ATTR timerISR() {
  portENTER_CRITICAL_ISR(&timerMux);
  // Increment pinValue and handle rollover
  pinValue = (pinValue % 3) + 1;
  // Update the task number based on pinValue
  taskNumber = pinValue;
  // Serial.print(taskNumber);
  portEXIT_CRITICAL_ISR(&timerMux);
}

//attching timer and interrupt
void timer_isr()
{
  timer=timerBegin(0, 80, true); // Timer 0, Prescaler 80 (80 MHz / 80 = 1 MHz), count up
  timerAttachInterrupt(timer, &timerISR, false);
  timerAlarmWrite(timer, TIMER_INTERVAL_MS * 1000, true);
  timerAlarmEnable(timer);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  tft_init();
  wifi_connect();
  mux302_init();
  graph_init();
  mpu_init();
  start_init();
  Serial.println(" all init done");

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  taskSemaphore = xSemaphoreCreateCounting(1, 1);

  // Serial.println(" Semaphore created");

  xTaskCreatePinnedToCore(State_1_1,"Time Calulation", 2048, NULL, 1, NULL, app_cpu);
  // Serial.println(" 1 created");

  xTaskCreatePinnedToCore(State_1_2,"Time Display", 2048, NULL, 1, NULL, app_cpu);
  // Serial.println(" 2 created");

  xTaskCreatePinnedToCore(State_2_1,"Oxymeter Read", 2048,NULL,1,NULL,app_cpu);
  // Serial.println(" 3 created");

  xTaskCreatePinnedToCore(State_2_2,"Oxy Display",2048, NULL, 1, NULL, app_cpu);
  // Serial.println(" 4 created");

  xTaskCreatePinnedToCore(State_3_1,"Accel Read",2048,NULL,1,NULL,app_cpu);
  xTaskCreatePinnedToCore(State_3_2,"Accel Display",2048, NULL,1,NULL,app_cpu);

timer_isr();
}

void loop() {
  // put your main code here, to run repeatedly:

}
