# Fitness Watch Code

This repository contains the code for a Fitness Watch project. The code is designed to run on an Arduino-compatible microcontroller with various sensors and a TFT display. The fitness watch has three main functionalities: displaying time and date, measuring heart rate using a MAX30105 pulse oximeter sensor, and tracking steps and calories burned using an MPU6050 accelerometer.

## Features

1. Time and Date Display: The fitness watch connects to Wi-Fi to fetch the current time and date from an NTP server and displays it on the TFT screen.

2. Heart Rate Monitoring: The watch uses a MAX30105 pulse oximeter sensor to measure the heart rate. The heart rate is displayed on the TFT screen, and a real-time heart rate graph is plotted as well.

3. Step and Calorie Tracking: The watch utilizes an MPU6050 accelerometer to track steps. The number of steps is displayed on the TFT screen, and a circular progress bar indicates the percentage of steps completed towards a daily goal. Calories burned are also calculated and shown.

## Dependencies

The code requires the following libraries to be installed:

- Adafruit_MPU6050: Library for interfacing with the MPU6050 accelerometer.
- Adafruit_Sensor: Base library for the Adafruit sensor suite.
- Wire: Arduino's I2C library for communication with I2C devices.
- TFT_eSPI: Library for interfacing with the TFT display.
- SPI: Arduino's SPI library for SPI communication.
- MAX30105: Library for the MAX30105 pulse oximeter sensor.
- heartRate: Custom library for calculating heart rate from the MAX30105 sensor data.
- NTPClient: Library for connecting to an NTP server and fetching time and date.
- WiFiUdp: Arduino's UDP library for Wi-Fi communication.
- WiFi: Arduino's Wi-Fi library for connecting to a Wi-Fi network.
- TFT_eWidget: Custom library for drawing graphs and widgets on the TFT display.

## Hardware Setup

1. Connect the TFT display to the microcontroller using the SPI interface.
2. Connect the MAX30105 pulse oximeter sensor to the microcontroller using I2C.
3. Connect the MPU6050 accelerometer to the microcontroller using I2C.
4. Add a push-button to act as the state change button (connected to pin 34).
5. Ensure that the microcontroller has Wi-Fi connectivity.

## Usage

1. Upload the provided code to the microcontroller.
2. After powering on the fitness watch, it will attempt to connect to the specified Wi-Fi network and fetch the current time from the NTP server.
3. The watch will display the time and date on the TFT screen.
4. Press the state change button to switch between different states:
   - State 1: Time and Date Display.
   - State 2: Heart Rate Monitoring and Graph.
   - State 3: Step and Calorie Tracking.
5. In State 2, place your index finger on the pulse oximeter sensor to measure the heart rate. The heart rate value will be displayed on the screen, and a real-time heart rate graph will be plotted.
6. In State 3, the watch will track steps and display the step count, calories burned, and a circular progress bar indicating progress towards a daily step goal.

## Customization

- You can adjust the Wi-Fi credentials (SSID and password) in the code to match your network.
- The NTP server address can be changed if needed.
- The daily step goal and calorie calculation factor can be customized based on individual preferences.

## Troubleshooting

- If the MAX30105 pulse oximeter sensor is not found, check the wiring and power connections.
- If the MPU6050 accelerometer initialization fails, verify the connections and power supply.
- Ensure that the Wi-Fi credentials are correct, and the microcontroller has access to the network.

## Notes

- This code is provided "as-is" and may require further optimization or modification based on the specific hardware and requirements.
- Make sure to refer to the datasheets of the MAX30105 and MPU6050 sensors for more details on their usage and configuration.


Please feel free to reach out for any questions or assistance related to the Fitness Watch code. Happy making!
