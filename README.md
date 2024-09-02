![System Flyer](/images/flyer_info.png)


# Water 1.0

**Author:** Ricardo Hernandez

## Table of Contents

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
9. [Nail Placement](#nail-placement)

## Description

Water 1.0 is an automated plant watering system that uses sensors to measure soil moisture and controls a pump via a relay to water the plant when needed. This project is inspired by the Botanicalls and Growduino concepts.

For information about the original project started by Leo Marte `09, visit: [Water 1.0 Project](http://cs.gettysburg.edu/~tneller/cs450/08fa/Physical_Computing/Plant_Watering.html)

## Changelog

- **2024-07-22:** (Ricardo Hernandez) Optimized and refactored code for better readability and performance. Updated comments.
- **2024-08-31:** (Ricardo Hernandez) Integrated Load Cell Scale, RGB LED, into the watering system.

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

## Plant Watering System Documentation

### Overview

This documentation describes the functionality and configuration of a plant watering system. The system uses a load cell to measure the weight of the water pitcher, moisture sensors to determine soil moisture, and an RGB LED for feedback. It also includes failsafe conditions to handle errors and abnormal situations.

### Components

- **Load Cell**: Measures the weight of the water pitcher.
- **HX711**: Amplifies and converts the load cell signal.
- **Moisture Sensors**: Measures soil moisture levels.
- **RGB LED**: Provides visual feedback about the system status.
- **Relay**: Controls the water pump.
- **EEPROM**: Stores the zero factor for the load cell.
- **Arduino**: Arduino Diecimila

### Pin Definitions

- **Load Cell DOUT Pin**: `A4`
- **Load Cell SCK Pin**: `A5`
- **Red LED Pin**: `10`
- **Green LED Pin**: `11`
- **Blue LED Pin**: `12`
- **Relay Pin**: `9`
- **Power Pin**: `8`
- **Moisture Probe Pin**: `0`
- **Status LEDs**: Pins `2-7`

### Arduino Wiring Schematic

Below is the wiring schematic for the plant watering system using Arduino.

![Arduino Wiring Schematic](/images/Arduino_Schematic.png)

### Configuration

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

### Functional Description

#### LED Indicators

- **Dry LED (Pin 5)**: Indicates that the soil is very dry.
- **Moist LED (Pin 6)**: Indicates that the soil is moist.
- **Soaked LED (Pin 7)**: Indicates that the soil is soaked.

#### LED Blinks

- **Purple Blink**: Indicates a load cell error.
- **Red Blink**: Indicates that the water level is too low.
- **Blue Blink**: Indicates a low water level warning.

#### Error Handling

- **Load Cell Error**: Detected if weight readings are outside a plausible range. The system blinks purple and sets `pumpEnabled` to `true`.
- **Low Water Level**: Detected if weight is below `EMPTY_WEIGHT_THRESHOLD` or `LOW_WEIGHT_THRESHOLD`. The system blinks red or blue, respectively, and sets `lowWaterLevel` to `true`.

### Functions

#### `setColor(int R, int G, int B)`

Sets the color of the RGB LED.

#### `blinkLed(int R, int G, int B, unsigned long duration, unsigned long blinkInterval)`

Blinks the RGB LED with the specified color, duration, and interval.

#### `blinkPurple()`

Blinks the LED purple to indicate an error.

#### `blinkRed()`

Blinks the LED red to indicate a low water level.

#### `blinkBlue()`

Blinks the LED blue to indicate a low water level warning.

#### `resetLeds()`

Turns off all moisture status LEDs.

#### `insertionSort(int* arr, int n)`

Sorts an array of integers using insertion sort.

#### `performMoistureReadings()`

Averages moisture sensor readings over a defined interval and updates the average moisture level.

#### `updateMoistureStatus()`

Updates the status LEDs and triggers watering based on the average moisture level.

#### `checkMoisture()`

Determines if the plant needs watering based on moisture sensor readings.

#### `waterPlant()`

Controls the relay to water the plant if conditions are met.

#### `checkLoadCellError(float weight)`

Checks for errors in load cell readings and adjusts the system state accordingly.

### Initialization

The initialization routine sets up pin modes, initial states, and EEPROM values.

### Main Loop

The main loop continuously checks moisture levels, updates system status, and triggers watering actions based on the current conditions.

### 3D Printed Encasings

#### Encasing for Scale and Pitcher

A custom 3D printed case was designed to hold the scale and water pitcher securely.

#### Encasing for Arduino and Wires

A separate 3D printed case was created to keep the Arduino and wires organized.

### Printing with Innovation Creativity Lab

The encasings were printed using the facilities at the Innovation Creativity Lab at Gettysburg College.

### 3D Printed Model Files

Model files for the 3D printed encasings are available in the 3D Printed Model Files Folder

### Known Issues and Troubleshooting

- **Incorrect Load Cell Readings**: Verify wiring connections and solder joints.
- **Moisture Sensor Calibration**: Check soil and adjust thresholds as needed.

### Failsafe Values

- **Default Zero Factor**: `28974`
- **Empty Weight Threshold**: `1.65 lbs`
- **Low Weight Threshold**: `3.00 lbs`
- **Sufficient Weight Threshold**: `4.00 lbs`

## General Tips

1. Regularly check soldering joints for loose connections.
2. Ensure the water pitcher is correctly placed on the scale.
3. Clean the moisture sensors periodically to avoid false readings.
4. Use waterproofing measures for the electronics in case of accidental water spills.

## Future Modifications

### Upgrade Considerations

#### Hardware Upgrade Suggestions

- Upgrade to a more accurate load cell with a higher weight capacity.
- Add additional sensors, such as a temperature or humidity sensor.

#### Possible Modifications for Indoor Plant Watering System

- Incorporate a Wi-Fi module to remotely monitor and control the system.
- Implement a mobile app for real-time notifications and control.

## Nail Placement

Place the nails at least 2-3 inches apart in the soil for more accurate moisture readings. Ensure they are inserted deep enough to measure the soil's moisture content effectively.
