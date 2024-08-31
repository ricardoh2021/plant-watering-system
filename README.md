# Water 1.0

**Author:** Leo Marte (Optimized and refactored by Ricardo Hernandez `21)

# Table of Contents

1. [Description](#description)
2. [Changelog](#changelog)
3. [IDE Setup Instructions](#ide-setup-instructions)
4. [Advice for Beginners](#advice-for-beginners)
5. [Plant Watering System Documentation](#plant-watering-system-documentation)
   - [Overview](#overview)
   - [Components](#components)
   - [Pin Definitions](#pin-definitions)
   - [Arduino Wiring Schematic](#arduino-wiring-schematic)
   - [Configuration](#configuration)
   - [Functional Description](#functional-description)
     - [LED Indicators](#led-indicators)
     - [LED Blinks](#led-blinks)
     - [Error Handling](#error-handling)
   - [Functions](#functions)
     - [`setColor(int R, int G, int B)`](#setcolorint-r-int-g-int-b)
     - [`blinkLed(int R, int G, int B, unsigned long duration, unsigned long blinkInterval)`](#blinkledint-r-int-g-int-b-unsigned-long-duration-unsigned-long-blinkinterval)
     - [`blinkPurple()`](#blinkpurple)
     - [`blinkRed()`](#blinkred)
     - [`blinkBlue()`](#blinkblue)
     - [`resetLeds()`](#resetleds)
     - [`insertionSort(int* arr, int n)`](#insertionsortint-arr-int-n)
     - [`performMoistureReadings()`](#performmoisturereadings)
     - [`updateMoistureStatus()`](#updatemoisturestatus)
     - [`checkMoisture()`](#checkmoisture)
     - [`waterPlant()`](#waterplant)
     - [`checkLoadCellError(float weight)`](#checkloadcellerrorfloat-weight)
   - [Initialization](#initialization)
   - [Main Loop](#main-loop)
   - [Known Issues and Troubleshooting](#known-issues-and-troubleshooting)
   - [Failsafe Values](#failsafe-values)
6. [General Tips](#general-tips)
7. [Future Modifications](#future-modifications)
8. [Upgrade Considerations](#upgrade-considerations)
   - [Hardware Upgrade Suggestions](#hardware-upgrade-suggestions)
     [Possible Modifications for Indoor Plant Watering System](#possible-modifications-for-indoor-plant-watering-system)

## Description

Water 1.0 is an automated plant watering system that uses sensors to measure soil moisture and controls a pump via a relay to water the plant when needed. This project is inspired by the Botanicalls and Growduino concepts.

For information about the original project started by Leo Marte `09, visit: [Water 1.0 Project](http://cs.gettysburg.edu/~tneller/cs450/08fa/Physical_Computing/Plant_Watering.html)

## Changelog

- **2024-07-22:** (Ricardo Hernandez) Optimized and refactored code for better readability and performance. Updated comments.
- **2024-08-31:** (Ricardo Hernandez) Integrated Load Cell Scale, RGB LED, into watering system.

## IDE Setup Instructions

(as of July 23, 2024 using v2.3.2)

1. Ensure you have the latest version of the Arduino IDE installed.
2. Select the correct board and port:
   - **Board:** Arduino Duemilanove or Diecimila
   - **Port:** Select the appropriate COM port
     - On Windows: Select the appropriate COM port (e.g., COM3, COM4, etc.)
     - On macOS: Select the appropriate port (e.g., /dev/tty.usbmodemxxxxx or /dev/cu.usbserial-xxxxx)
3. Select the correct processor:
   - **Processor:** ATmega168 (Due to the age of the Arduino)
4. Compile and upload the code to the Arduino board.

## Advice for Beginners

If you’re new to this and feeling overwhelmed, my advice is simple: just go for it! Don't be afraid to dive in and learn something new along the way. There are countless YouTube videos and tutorials on Arduino projects that can guide you. Break the problem down into smaller pieces and build from there. Start experimenting—it's the best way to learn.

This project is meant for you to expand on! Take advantage of all the tools Gettysburg College offers, like the Innovation and Creativity Lab. Don’t hesitate to ask whoever is in charge of the lab for help. Professor Neller is also a fantastic resource, so don’t be intimidated by all the wiring (my fault, haha) or if it seems complicated at first. Looks can be deceiving, but you can always start with a personal Arduino kit to get familiar with how things work. Take baby steps, and you’ll find yourself understanding more and more as you go.

Have fun!

# Plant Watering System Documentation

## Overview

This documentation describes the functionality and configuration of a plant watering system. The system uses a load cell to measure the weight of the water pitcher, moisture sensors to determine soil moisture, and an RGB LED for feedback. It also includes failsafe conditions to handle errors and abnormal situations.

## Components

- **Load Cell**: Measures the weight of the water pitcher.
- **HX711**: Amplifies and converts the load cell signal.
- **Moisture Sensors**: Measures soil moisture levels.
- **RGB LED**: Provides visual feedback about the system status.
- **Relay**: Controls the water pump.
- **EEPROM**: Stores the zero factor for the load cell.
- **Arduino**: Arduino Diecimila

## Pin Definitions

- **Load Cell DOUT Pin**: `A4`
- **Load Cell SCK Pin**: `A5`
- **Red LED Pin**: `10`
- **Green LED Pin**: `11`
- **Blue LED Pin**: `12`
- **Relay Pin**: `9`
- **Power Pin**: `8`
- **Moisture Probe Pin**: `0`
- **Status LEDs**: Pins `2-7`

## Arduino Wiring Schematic

Below is the wiring schematic for the plant watering system using Arduino.

![Arduino Wiring Schematic](/images/Arduino_Schematic.png)

## Configuration

- **Calibration Factor**: `-94500`
- **Weight Thresholds**:
  - **Empty Weight Threshold**: `1.65 lbs`
  - **Low Weight Threshold**: `3.00 lbs`
  - **Sufficient Weight Threshold**: `4.00 lbs`
- **Moisture Levels**:
  - **Dry**: `420`
  - **Moist**: `710`
  - **Soaked**: `1000`
- **Intervals**:
  - **Moisture Check Interval**: `10000 ms`
  - **Moisture Sample Interval**: `2000 ms`
  - **Water Interval**: `3000 ms`
  - **Failsafe Value**: `200`

## Functional Description

### LED Indicators

- **Dry LED (Pin 5)**: Indicates that the soil is very dry.
- **Moist LED (Pin 6)**: Indicates that the soil is moist.
- **Soaked LED (Pin 7)**: Indicates that the soil is soaked.

### LED Blinks

- **Purple Blink**: Indicates a load cell error.
- **Red Blink**: Indicates that the water level is too low.
- **Blue Blink**: Indicates a low water level warning.

### Error Handling

- **Load Cell Error**: Detected if weight readings are outside a plausible range. The system blinks purple and sets `pumpEnabled` to `true`.
- **Low Water Level**: Detected if weight is below `EMPTY_WEIGHT_THRESHOLD` or `LOW_WEIGHT_THRESHOLD`. The system blinks red or blue, respectively, and sets `lowWaterLevel` to `true`.

## Functions

### `setColor(int R, int G, int B)`

Sets the color of the RGB LED.

### `blinkLed(int R, int G, int B, unsigned long duration, unsigned long blinkInterval)`

Blinks the RGB LED with specified color, duration, and interval.

### `blinkPurple()`

Blinks the LED purple to indicate an error.

### `blinkRed()`

Blinks the LED red to indicate a low water level.

### `blinkBlue()`

Blinks the LED blue to indicate a low water level warning.

### `resetLeds()`

Turns off all moisture status LEDs.

### `insertionSort(int* arr, int n)`

Sorts an array of integers using insertion sort.

### `performMoistureReadings()`

Averages moisture sensor readings over a defined interval and updates the average moisture level.

### `updateMoistureStatus()`

Updates the status LEDs and triggers watering based on the average moisture level.

### `checkMoisture()`

Performs a moisture check and updates the watering status.

### `waterPlant()`

Activates the relay to water the plant if necessary. Includes a failsafe check.

### `checkLoadCellError(float weight)`

Checks for load cell errors by evaluating weight readings for stability and plausibility.

## Initialization

The system initializes the HX711 scale, sets the calibration factor, and reads the zero factor from EEPROM. If the EEPROM read fails, a default zero factor is used.

## Main Loop

The main loop reads the weight from the scale, checks for load cell errors, and updates the moisture status. It also includes a delay between iterations.

## Known Issues and Troubleshooting

- **Load Cell Error**: Ensure that the load cell connections are secure and the calibration factor is correctly set.
- **Moisture Sensor Issues**: Check the sensor connections and ensure they are properly calibrated.

## Failsafe Values

- **Default Zero Factor**: `28974` (used if EEPROM read fails).
- **Failsafe Value for Moisture**: `200` (minimum difference in moisture readings indicating a problem).

### General Tips

- **Wiring and Connections**: The most common issues are related to wiring and connections. Double-check all connections before modifying the code.
- **Soldering Skills**: My soldering skills are beginner level. If you are experienced at soldering, consider fixing any wiring issues before adjusting the code. Most issues are likely due to poor soldering.
- **Isolating Issues**: The best way to troubleshoot problems is to isolate the issue. I have added folders for each component, allowing you to use the code to troubleshoot specific parts. The RGB LED is connected to the Load Cell Sensor, while everything else is connected to the automatic watering system.

## Future Modifications

- Improve soldering quality and wire management.
- Enhance calibration procedures for more accurate readings.
- Add additional sensors or features for extended functionality.
- **Possible Update**: Consider upgrading to an Arduino Uno for improved performance and additional features.


## Upgrade Considerations

Upgrading from the Arduino Diecimila to the Arduino Uno offers significant benefits, including:

- **Increased Flash Memory**: 32KB vs. 16KB
- **More SRAM**: 2KB vs. 1KB
- **Enhanced USB Communication**: ATmega16U2 chip for more reliable connection
- **Cable Management**: Pretty self explanatory, cable management can be better. 

These improvements provide greater flexibility, better performance, and ongoing support, making the Arduino Uno a more robust choice for handling more complex projects and code.

### Hardware Upgrade Suggestions

- **Water Level Sensing**: Consider integrating a float switch or another non-invasive sensor for more accurate water level readings in the reservoir.
- **RGB LED**: Minimize the number of individual LEDs by using an RGB LED to indicate the soil moisture state, simplifying the circuit while retaining clear feedback.
- **Moisture Sensor**: Upgrade from galvanized nails to non-corrosive moisture sensors. Corrosion can affect readings over time, so using capacitive sensors or other advanced methods for measuring soil moisture would ensure more reliable and long-lasting performance.
  These improvements provide greater flexibility, better performance, and ongoing support, making the Arduino Uno a more robust choice for handling more complex projects and code.

# Possible Modifications for Indoor Plant Watering System

- **Wi-Fi/Bluetooth Connectivity**: Add modules to monitor and control the system via smartphone or web interface.
- **Push Notifications**: Receive alerts for low water levels, dry soil, or system errors on your phone.
- **Capacitive Moisture Sensors**: Upgrade to capacitive moisture sensors for more accurate soil moisture readings.
- **Temperature and Humidity Sensor**: Monitor indoor climate conditions to optimize watering.
- **Light Sensor**: Adjust watering based on the amount of light the plant receives.
- **Multi-Zone Watering**: Expand the system to support multiple plants, each with its own watering zone.
- **LCD Display**: Display real-time data, such as soil moisture levels and system status.
- **Button Controls**: Add buttons or a rotary encoder for manual adjustments and settings.
- **pH Sensor**: Monitor the pH of the water to ensure it's suitable for the plant.
- **EC Sensor**: Measure the electrical conductivity to assess nutrient concentration in the water.
- **Voice Assistant Integration**: Control the watering system using voice commands through a smart assistant like Alexa or Google Assistant.
- **Webcam Integration**: Set up a camera to visually monitor the plant remotely.
- **Fertilizer Pump**: Automatically dispense liquid fertilizer at set intervals.
- **Self-Diagnostics**: Implement a feature that regularly checks system components and notifies you of any malfunctions.
- **PC/Tablet Interface**: Develop a user-friendly interface for easier monitoring and configuration via a computer or tablet.
