# Water 1.0

**Author:** Leo Marte (Optimized and refactored by Ricardo Hernandez)

## Description

Water 1.0 is an automated plant watering system that uses sensors to measure soil moisture and controls a pump via a relay to water the plant when needed. This project is inspired by the Botanicalls and Growduino concepts.

For more information, visit: [Water 1.0 Project](http://cs.gettysburg.edu/~martle02/cs450/)

## Changelog

- **2024-07-22:** (Ricardo Hernandez) Optimized and refactored code for better readability and performance. Updated comments.

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

## Define Your Botanical Preferences

Adjust the moisture thresholds based on your plant's needs:

```cpp
#define DRY 420               // After not being watered for ~5 days
#define MOIST 710             // Midpoint between DRY and SOAKED
#define SOAKED 1000           // Least desired level after watering
