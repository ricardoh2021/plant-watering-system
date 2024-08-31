# Water 1.0

**Author:** Leo Marte (Optimized and refactored by Ricardo Hernandez `21)

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

## Future Modifications

- Improve soldering quality and wire management.
- Enhance calibration procedures for more accurate readings.
- Add additional sensors or features for extended functionality.
