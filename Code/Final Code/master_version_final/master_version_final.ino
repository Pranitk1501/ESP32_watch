//Master Version

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

int state = 1;

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

const char* ssid     = "GalaxyS21";
const char* password = "eany7165";

String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

#define NTP_OFFSET  19800 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "time.google.com"

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

int state2init = 0; //0 means watch state yet to be init
int buttonState = 0; //state of the button
const int stateChange =  34; //state change button set to 5

void setup() {
  Serial.begin(115200); 

  //state change button init

  pinMode(stateChange, INPUT);

  //tft_init for displaying initial sensor status

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextWrap(true, true);

  //Wifi_NTP_Client_init

  tft.print("Connecting to ");
  delay(1000);
  tft.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    tft.print(".");
  }
  tft.println("");
  tft.println("WiFi connected.");
  delay(1000);
  tft.println("IP address: ");
  delay(1000);
  tft.println(WiFi.localIP());
  timeClient.begin();

  //oxy_init

  tft.println("Initializing Oxymeter...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) 
  {
    tft.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  tft.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0);
  delay(2000);

  //graph_init

  gr.createGraph(100, 40, 0x9fd4);
  gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);
  tr.startTrace(TFT_BLACK);
  gr.setGraphGrid(gxLow, 101.0, gyLow, 101.0, TFT_BLACK); 

  //accelerometer_init

  if (!mpu.begin()) {
  tft.println("MPU Sensor init failed");
  while (1)
    yield();
  }
  else {
    tft.println("MPU init successfull");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  delay(2000);

  //Startup_init
  static uint32_t index = 0;
  for(int j = 160; j >= 0; j=j-5) {
    uint16_t fg_color = rainbow(index);
    uint16_t xcen = tft.width() / 2; 
    uint16_t ycen = tft.height() / 2;
    tft.drawSmoothCircle(xcen, ycen, j, fg_color, TFT_BLACK);
    index += 5;
    delay(100);
  }

  //state_1_bg_init

  tft.fillRectVGradient(0, 0, 128, 160, TFT_BLACK, TFT_RED);
  tft.setTextColor(TFT_WHITE);
}

void loop() {

  buttonState = digitalRead(stateChange);

  if(buttonState == 1) {
    state++;
    delay(3000);
    if(state > 3) {
      state = 1;
      state2init = 0;
    }
  }

  if(state == 1) {
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

  if(state == 2) {
    if(state2init == 0) {
      tft.fillScreen(0x9fd4);
      state2init = 1;
      delay(1000);
    }
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
      //Serial.println(irValue); --> used for debugging
  }

  if(state == 3) {
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
    String distance = "Distance: " + String(steps * 0.0009) + " km"; //for average of 0.9 m per step
    tft.drawCentreString(distance, 64, 140, 2);
    delay(1000);
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