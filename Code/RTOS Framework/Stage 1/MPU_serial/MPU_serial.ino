
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define MPU6050_ADDRESS 0x68

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

TaskHandle_t mpu_task;

void read_mpu(void *Parameter)
{
  Adafruit_MPU6050 mpu;

  mpu.begin(MPU6050_ADDRESS);

  while (true) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float accelX = a.acceleration.x;
    float accelY = a.acceleration.y;
    float accelZ = a.acceleration.z;

    float gyroX = g.gyro.x;
    float gyroY = g.gyro.y;
    float gyroZ = g.gyro.z;

    float temperature = temp.temperature;

    Serial.print("Accel: X=");
    Serial.print(accelX);
    Serial.print(" Y=");
    Serial.print(accelY);
    Serial.print(" Z=");
    Serial.print(accelZ);

    Serial.print(" Gyro: X=");
    Serial.print(gyroX);
    Serial.print(" Y=");
    Serial.print(gyroY);
    Serial.print(" Z=");
    Serial.print(gyroZ);

    Serial.print(" Temp: ");
    Serial.println(temperature);
}
}


void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  xTaskCreatePinnedToCore(
    read_mpu,   //vTask Code
    "MPU6050Task", // Name
    4096, // Stack size
    NULL, // Parameter to be passed 
    0, // Task priority
    NULL, //  Task reference for further task
    1 // core ID - CORE 1 used here
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}
