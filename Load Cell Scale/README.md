# `load_cell_rgb.ino` - README

## Overview

The `load_cell_rgb.ino` Arduino sketch interfaces with a 10kg load cell using the HX711 module and controls an RGB LED to indicate the water level in a pitcher.

**Author:** Ricardo Hernandez  
**Date:** August 2024

### Description

This sketch reads weight data from a 10kg load cell connected to the HX711 module. The load cell measures the weight of a water pitcher. Based on the weight, the RGB LED displays different colors to indicate the water level:

- **Red:** Empty or low water level
- **Yellow:** Sufficient water level
- **Green:** Full water pitcher

The code includes calibration and zero factor handling using EEPROM to ensure accurate weight measurements.

**Note:** This file is particularly useful if you want to isolate and troubleshoot issues related to the load cell and the RGB LED. It allows you to focus specifically on debugging these components without the complexity of the full system.

## Components

- **10kg Load Cell** with HX711 Amplifier
- **RGB LED**
- **Arduino Diecimila (ATmega168)**
- **Connecting Wires**

## Pin Definitions

- **Load Cell Pins:**
  - `LOADCELL_DOUT_PIN`: Analog pin A4 (DOUT of HX711)
  - `LOADCELL_SCK_PIN`: Analog pin A5 (SCK of HX711)
- **RGB LED Pins:**
  - `RED_PIN`: Digital pin 10
  - `GREEN_PIN`: Digital pin 11
  - `BLUE_PIN`: Digital pin 12

## Calibration

The code includes automatic calibration and zero factor handling:

1. **Initial Setup:**
   - The scale is tared (reset to zero).
   - The zero factor is stored in EEPROM if not already present.
2. **Calibration Factor:**
   - Adjust the `CALIBRATION_FACTOR` to match your scale setup.

## Thresholds

- **Empty Weight Threshold:** 1.50 lbs
- **Low Weight Threshold:** 3.50 lbs
- **Sufficient Weight Threshold:** 4.00 lbs

## LED Color Indications

- **Red:** Water pitcher is empty or low.
- **Yellow:** Water pitcher has sufficient weight.
- **Green:** Water pitcher is full.

## Functions

- `setColor(int R, int G, int B)`: Sets the color of the RGB LED.
- `blinkRed()`: Blinks the red LED to indicate an empty pitcher.

## Main Loop

The main loop:

1. Reads weight from the load cell.
2. Updates the RGB LED color based on weight thresholds.
3. Allows for serial input to reset the zero factor by sending 'z' or 'Z'.

## Troubleshooting

### Common Issues

1. **Wiring Problems:**
   - Ensure all connections are secure and correctly matched with the pins.
2. **Soldering Quality:**

   - **Note:** My soldering skills are beginner level at best. If you are experienced with soldering, consider fixing any wiring issues before changing the code, as most problems are often due to poor connections.

3. **Load Cell Issues:**

   - If you encounter load cell issues, upload the `load_cell_rgb.ino` file to the Arduino. This file focuses on debugging the load cell separately from the main code.

4. **EEPROM Issues:**
   - Check EEPROM functionality if there are problems with storing or retrieving the zero factor.

## Notes

- Adjust the `CALIBRATION_FACTOR` to match your specific scale setup for accurate readings.
- Regularly check and maintain the system to ensure reliable operation.
