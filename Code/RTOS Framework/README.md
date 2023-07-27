## Features

- Real-time display of current time and date
- Heart rate monitoring using the MAX30105 pulse oximeter sensor
- Step count tracking using the MPU6050 accelerometer and gyroscope sensor
- Visualization of heart rate and oxygen saturation data on a graphical display
- Wi-Fi connectivity for synchronizing time and date with NTP server
- Multi-tasking using FreeRTOS

## Hardware Requirements

- ESP32 development board (e.g., ESP32 DevKitC)
- MAX30105 Pulse Oximeter Sensor
- MPU6050 Accelerometer and Gyroscope Sensor
- TFT Display (compatible with TFT_eSPI library)
- Push Button or Toggle Switch
- Power supply and wiring components

## Software Requirements

- Arduino IDE (or compatible development environment)
- Required Libraries (available in Arduino Library Manager):
  - Adafruit_MPU6050
  - Adafruit_Sensor
  - Wire
  - TFT_eSPI
  - SPI
  - MAX30105
  - heartRate
  - NTPClient
  - WiFiUdp
  - WiFi

## Setup

1. Connect the sensors and display to the ESP32 development board according to the provided pin configurations in the source code.
2. Install the required libraries in the Arduino IDE using the Library Manager.
3. Modify the Wi-Fi credentials (`ssid` and `password`) to match your network.
4. Upload the code to your ESP32 development board.

## Functionality

### Real-Time Clock and Date Display

The system synchronizes the time and date with an NTP server using Wi-Fi connectivity. The current time and date are displayed on the TFT screen.

### Heart Rate Monitoring

The MAX30105 pulse oximeter sensor measures the infrared (IR) reflection from the user's fingertip to calculate heart rate and oxygen saturation. The calculated heart rate is displayed on the TFT screen.

### Heart Rate Graph Visualization

The system graphically displays heart rate variations over time using a line graph on the TFT screen.

### Step Count Tracking

The MPU6050 accelerometer and gyroscope sensor track user movement and calculate the number of steps taken. The step count is displayed on the TFT screen.

### Step Count Progress Arc Visualization

A circular arc on the TFT screen visually represents the progress towards reaching a target step count.

### Task Switching

The system implements multi-tasking using FreeRTOS. A timer and an interrupt-driven mechanism allow for smooth and automatic switching between different tasks to display various information.

## Notes

- The heart rate monitoring and step count tracking are approximate and may not be as accurate as dedicated medical devices.
- The system is intended for educational and recreational purposes only and should not be used for medical diagnosis or treatment.

## Acknowledgments

This project was inspired by various health monitoring and fitness tracking systems. The libraries used are based on the work of the respective developers and contributors.
