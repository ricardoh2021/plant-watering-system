# Water 1.0

**Author:** Ricardo Hernandez

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
   - [3D Printed Encasings](#3d-printed-encasings)
   - [Encasing for Scale and Pitcher](#encasing-for-scale-and-pitcher)
   - [Encasing for Arduino and Wires](#encasing-for-arduino-and-wires)
   - [Design Process in TinkerCAD](#design-process-in-tinkercad)
   - [Printing with Innovation Creativity Lab](#printing-with-innovation-creativity-lab)
   - [3D Printed Model Files](#3d-printed-model-files)
   - [Known Issues and Troubleshooting](#known-issues-and-troubleshooting)
   - [Failsafe Values](#failsafe-values)
6. [General Tips](#general-tips)
7. [Future Modifications](#future-modifications)
8. [Upgrade Considerations](#upgrade-considerations)
   - [Hardware Upgrade Suggestions](#hardware-upgrade-suggestions)
   - [Possible Modifications for Indoor Plant Watering System](#possible-modifications-for-indoor-plant-watering-system)
9. [Nail Placement](#moisture-sensor-placement)

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

Performs a moisture reading and updates the system status accordingly.

### `waterPlant()`

Activates the relay to turn on the water pump for a specified duration.

### `checkLoadCellError(float weight)`

Checks for load cell errors and performs error handling.

## Initialization

In the `setup()` function, the system initializes all sensors, LEDs, and relays. It also calibrates the load cell and reads the initial values from EEPROM.

## Main Loop

The `loop()` function continuously checks the moisture levels and load cell status, updates the LEDs, and controls the water pump as needed.

## 3D Printed Encasings

### Encasement for Scale and Pitcher

Designed to securely hold the load cell and water pitcher.

### Encasement for Arduino and Wires

Provides protection and organization for the Arduino and its connections.

## Design Process in TinkerCAD

Models and prototypes were created using TinkerCAD for visualizing and refining the enclosures.

## Printing with Innovation Creativity Lab

Models were printed using the resources available at the Innovation Creativity Lab.

## 3D Printed Model Files

- **Water Pitcher Case**: [Automatic_Watering_System.stl](/3D%20Printed%20Model%20Files/Automatic_Watering_System.stl)
- **Arduino Enclosure**: [Electronics_case.stl](/3D%20Printed%20Model%20Files/Electronics_case.stl)
- **Arduino Enclosure Lid**: [Electronics_case_lide.stl](/3D%20Printed%20Model%20Files/Electronics_case_lid.stl)

## Known Issues and Troubleshooting

- **Load Cell Inaccuracies**: Ensure proper calibration and check connections.
- **Moisture Sensor Errors**: Verify nail placement and connection stability.

## Failsafe Values

- **Default Weight**: `28974` (for EEPROM read/write failures)
- **Minimum Weight Threshold**: `1.0 lbs`

## General Tips

- **Check Connections**: Always verify connections and soldering before troubleshooting code issues.
- **Calibration**: Regularly calibrate the load cell to ensure accurate readings.

## Future Modifications

- **Enhance Error Handling**: Implement more sophisticated error detection and handling mechanisms.
- **Add Remote Monitoring**: Integrate wireless communication for remote monitoring and control.

## Upgrade Considerations

### Hardware Upgrade Suggestions

- **Upgrade to a Higher Precision Load Cell**: For more accurate weight measurements.
- **Add More Sensors**: To monitor additional environmental factors.

### Possible Modifications for Indoor Plant Watering System

- **Humidity Sensors**: Integrate sensors to measure indoor humidity.
- **Light Sensors**: Monitor light levels to optimize plant care.

## Moisture Sensor Placement

**Nail Placement**

- **Suggestion**: Place the nails for the moisture sensor at least 2-4 inches apart in the soil.

**Why Proper Placement is Important**

1. **Accurate Moisture Readings**: Placing nails too close together can cause overlapping sensor readings, leading to inaccurate moisture levels. By spacing them 2-4 inches apart, you ensure that each nail measures moisture in a distinct area of the soil, providing a more accurate representation of the soil's overall moisture content.

2. **Avoid Sensor Interference**: If nails are placed too close, the readings from one nail might affect the other, resulting in interference. Proper spacing minimizes this risk and ensures that each sensor's reading is independent and reliable.

3. **Improved Soil Coverage**: Spacing nails apart allows for better coverage of the soil, helping to identify varying moisture levels throughout the plant’s root zone. This is particularly useful for plants with larger root systems or uneven soil moisture distribution.

4. **Reduced Corrosion**: When nails are too close, they may corrode faster due to concentrated electrochemical reactions. Proper spacing helps to mitigate this issue, extending the life of your sensors.

By following these guidelines, you can enhance the performance and longevity of your moisture sensors, leading to more reliable and effective plant watering.
