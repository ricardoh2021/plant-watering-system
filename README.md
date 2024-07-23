# Water 1.0

**Author:** Leo Marte (Optimized and refactored by Ricardo Hernandez)

## Description

Water 1.0 is an automated plant watering system that uses sensors to measure soil moisture and controls a pump via a relay to water the plant when needed. This project is inspired by the Botanicalls and Growduino concepts.

For more information, visit: [Water 1.0 Project]([http://cs.gettysburg.edu/~martle02/cs450/](http://cs.gettysburg.edu/~tneller/cs450/08fa/Physical_Computing/Plant_Watering.html))

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
```

## Define constants 
Configure timing and intervals for moisture checks and watering:

```cpp
#define MOIST_CHECK_INTERVAL 20000   // Milliseconds between checks (1 minute)
#define MOIST_SAMPLE_INTERVAL 2000   // Milliseconds for averaging readings
#define WATER_INTERVAL 3000          // Milliseconds to allow for water flow
#define MOIST_SAMPLES 10             // Number of samples to average
#define FAILSAFE_VALUE 200           // Minimum difference in moisture to indicate successful watering
```

## Program Variables and Board Pins

Configure the pins for LEDs, power, relay, and probe:

```cpp
// LEDs
#define onLed 2
#define readLed 3
#define workLed 4
#define dryLed 5
#define moistLed 6
#define soakedLed 7

// Probes
#define powerPin 8
#define relayPin 9
#define probePin 0
```

## Helper Methods

### resetLeds()

Resets all plant status LEDs to the OFF state.

### insertionSort()

Sorts the moisture readings array using the Insertion Sort algorithm for better performance and readability.

### checkMoisture()

Collects and processes moisture readings, determines watering status, and updates the status LEDs.

### waterPlant()

Manages the watering of the plant, activates the relay to dispense water, and includes a failsafe check for equipment servicing.

## Arduino Methods

### setup()

Initializes the Arduino board, sets up pins, and starts serial communication.

### loop()

Main loop that repeatedly checks moisture levels and performs actions based on the readings.

### License

This project is licensed under the MIT License. See the LICENSE file for details.
