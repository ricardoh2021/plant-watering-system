/*
 * Application: Water 1.0
 * Author: Leo Marte
 *
 * Sets up a system to water a plant, by using sensors to probe water moisture
 * and opening a pump (via a relay) to water the plant when needed. Combination
 * of Botanicalls and Growduino project concepts.
 *
 * http://cs.gettysburg.edu/~martle02/cs450/
 *
 * Changelog:
 * - 2024-07-22: (Ricardo Hernandez) Optimized and refactored code for better readability and performance. Updated comments.
 * - 2024-08-16: (Ricardo Hernandez) Integrated the Load Cell with RGB Sensor into the automatic watering plant system. Merged Load Cell code into water.ino. Added docstrings for better documentation. Removed variables that were never used. 
 * 
 * -------------------------------- IMPORTANT ------------------------------------
 * IDE Setup Instructions: (as of July 23, 2024 using v2.3.2)
 * - Ensure you have the latest version of the Arduino IDE installed.
 * - Select the correct board and port:
 *   - Board: Arduino Duemilanove or Diecimila 
 *   - Port: Select the appropriate COM port
 *     - On Windows: Select the appropriate COM port (e.g., COM3, COM4, etc.)
 *     - On macOS: Select the appropriate port (e.g., /dev/tty.usbmodemxxxxx or /dev/cu.usbserial-xxxxx)
 * - Select the correct processor (CRUCIAL)
 *   - Select ATmega168 (Due to the age of the Arduino)
 * - Compile and upload the code to the Arduino board.
 */

#include "HX711.h"
#include <EEPROM.h>

// Pin definitions for the HX711 module
const int LOADCELL_DOUT_PIN = A4;  // Pin connected to DOUT of HX711 (Analog pin A4)
const int LOADCELL_SCK_PIN = A5;   // Pin connected to SCK of HX711 (Analog pin A5)

// Pin definitions for the RGB LED
const int RED_PIN = 10;    // Pin connected to the red leg of the LED
const int GREEN_PIN = 11;  // Pin connected to the green leg of the LED
const int BLUE_PIN = 12;   // Pin connected to the blue leg of the LED

// EEPROM memory address for storing the zero factor
const int EEPROM_ADDRESS = 0;

// Weight thresholds (in lbs)
const float EMPTY_WEIGHT_THRESHOLD = 1.50; // Under this is empty
const float LOW_WEIGHT_THRESHOLD = 3.15;
const float SUFFICIENT_WEIGHT_THRESHOLD = 4.00;

// Calibration factor for scaling the raw data to weight units
const float CALIBRATION_FACTOR = -94500; // Adjust this value to match your scale setup

HX711 scale;  // Create an instance of the HX711 class

/* DEFINE YOUR BOTANICAL PREFERENCES */

#define DRY 420
#define MOIST 710
#define SOAKED 1000

/* DEFINE CONSTANTS */

#define MOIST_CHECK_INTERVAL 10000   // milliseconds between checks
#define MOIST_SAMPLE_INTERVAL 2000   // milliseconds over which to average reading samples
#define WATER_INTERVAL 3000          // milliseconds to allow for water to flow
#define MOIST_SAMPLES 10             // # of samples to average when reading moisture
#define FAILSAFE_VALUE 200           // minimum difference in moisture indicating watering

/* PROGRAM VARIABLES AND BOARD PINS */

// LEDs
#define onLed 2
#define readLed 3
#define workLed 4
#define dryLed 5
#define moistLed 6
#define soakedLed 7

// probes
#define powerPin 8
#define relayPin 9
#define probePin 0

int moistValues[MOIST_SAMPLES];
unsigned long lastMoistTime = 0;
unsigned long lastWaterTime = 0;
int lastMoistAvg = 0;
int oldlastMoistAvg = 0;
boolean needServicing = false;
boolean wateredLast = false;
boolean pumpEnabled = true;
boolean lowWaterLevel = false;  // Global variable to track low water level status

/* HELPER METHODS */

/**
 * @brief Set the color of the RGB LED using RGB values.
 * 
 * @param R Red component (0-255).
 * @param G Green component (0-255).
 * @param B Blue component (0-255).
 */
void setColor(int R, int G, int B) {
  analogWrite(RED_PIN, R);
  analogWrite(GREEN_PIN, G);
  analogWrite(BLUE_PIN, B);
}

/**
 * @brief Function to blink the red LED.
 */
void blinkRed() {
  setColor(255, 0, 0); // Red
  delay(500);  // Wait 500 ms
  setColor(0, 0, 0); // Turn off the LED
  delay(500);  // Wait 500 ms
}

void resetLeds() {
  digitalWrite(dryLed, LOW);
  digitalWrite(moistLed, LOW);
  digitalWrite(soakedLed, LOW);
}

void insertionSort(int* arr, int n) {
  for (int i = 1; i < n; i++) {
    int key = arr[i];
    int j = i - 1;

    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j = j - 1;
    }
    arr[j + 1] = key;
  }
}

void checkMoisture() {
  if (!pumpEnabled) return;  // Skip moisture check if pump is disabled

  digitalWrite(readLed, HIGH);

  // Collect readings
  for (int i = 0; i < MOIST_SAMPLES + 1; i++) {
    digitalWrite(powerPin, HIGH);
    int val = analogRead(probePin);
    digitalWrite(powerPin, LOW);

    // Discard the first reading
    if (i != 0) {
      moistValues[i - 1] = val;
      lastMoistTime = millis();
    }

    delay(MOIST_SAMPLE_INTERVAL);
  }

  // Sort readings to get the median
  insertionSort(moistValues, MOIST_SAMPLES);
  oldlastMoistAvg = lastMoistAvg;
  lastMoistAvg = moistValues[MOIST_SAMPLES / 2];

  Serial.print("median read: ");
  Serial.print(lastMoistAvg);
  Serial.print(" | ");

  // Check moisture levels and decide if watering is needed
  resetLeds();
  if (lastMoistAvg <= DRY) {
    Serial.println(" status: VERY DRY - watering plant");
    digitalWrite(dryLed, HIGH);
    waterPlant();
  } else if (lastMoistAvg < MOIST) {
    Serial.println(" status: DRY - watering plant");
    digitalWrite(dryLed, HIGH);
    waterPlant();
  } else if (lastMoistAvg < SOAKED) {
    Serial.println(" status: MOIST - plant is just fine");
    digitalWrite(moistLed, HIGH);
  } else {
    Serial.println(" status: SOAKED - don't water plant!");
    digitalWrite(soakedLed, HIGH);
  }

  digitalWrite(readLed, LOW);
}

void waterPlant() {
  if (lowWaterLevel) {
    Serial.println("Pump deactivated due to low water level. Watering skipped.");
    return;  // Exit the function early if water level is too low
  }

  int diffMoist = lastMoistAvg - oldlastMoistAvg;

  if (wateredLast && diffMoist <= FAILSAFE_VALUE) {
    needServicing = true;
    Serial.println("");
    Serial.println("water 1.0 is suspending execution...");
    Serial.println("EQUIPMENT NEEDS SERVICING!");
    resetLeds();
    while (true) {
      digitalWrite(onLed, LOW);
      delay(1000);
      digitalWrite(onLed, HIGH);
      delay(1000);
    }
  } else {
    Serial.print("watering started at: ");
    Serial.print(millis());
    digitalWrite(workLed, HIGH);

    digitalWrite(relayPin, HIGH);
    delay(WATER_INTERVAL);
    digitalWrite(relayPin, LOW);

    digitalWrite(workLed, LOW);
    Serial.print(", it lasted: ");
    Serial.print(WATER_INTERVAL);
    Serial.print(" seconds");
    Serial.println("");

    wateredLast = true;
  }
}

/* Arduino methods */
void setup() {
  pinMode(onLed, OUTPUT);
  pinMode(readLed, OUTPUT);
  pinMode(workLed, OUTPUT);
  pinMode(dryLed, OUTPUT);
  pinMode(moistLed, OUTPUT);
  pinMode(soakedLed, OUTPUT);
  pinMode(powerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(onLed, HIGH);
  Serial.begin(9600);
  Serial.println("water 1.0 up and running...");
  Serial.println("");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  // Initialize the HX711 module
  scale.set_scale();  // Set the scale to the default calibration factor

  // Load zero factor from EEPROM
  long zero_factor;
  EEPROM.get(EEPROM_ADDRESS, zero_factor);
  if (zero_factor == 0xFFFFFFFF) {
    scale.tare();
    zero_factor = scale.read_average();
    EEPROM.put(EEPROM_ADDRESS, zero_factor);
  } else {
    scale.set_offset(zero_factor);
  }

  // Set the RGB pins as output
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  setColor(255, 0, 0);
}

void loop() {
  scale.set_scale(CALIBRATION_FACTOR);  // Apply the current calibration factor

  float weight = scale.get_units();  // Get the weight from the scale

  // Print the weight reading and the current calibration factor to the serial monitor
  Serial.print("Reading: ");
  Serial.print(weight, 2);  // Display the weight with 2 decimal places
  Serial.print(" lbs");
  Serial.print(" calibration_factor: ");
  Serial.print(CALIBRATION_FACTOR);
  Serial.println();

  // Set LED color based on weight conditions and check water level
  if (weight < EMPTY_WEIGHT_THRESHOLD) {
    blinkRed();
    lowWaterLevel = true;  // Set the flag indicating low water level
    Serial.println("Water level too low! Pump deactivated.");
  } else if (weight < LOW_WEIGHT_THRESHOLD) {
    setColor(255, 0, 0);  // Solid red
    lowWaterLevel = true;  // Set the flag indicating low water level
    Serial.println("Water level too low! Pump deactivated.");
  } else if (weight < SUFFICIENT_WEIGHT_THRESHOLD) {
    setColor(255, 255, 0);  // Yellow
    lowWaterLevel = false;  // Water level is sufficient, clear the flag
  } else {
    setColor(0, 255, 0);  // Green
    lowWaterLevel = false;  // Water level is sufficient, clear the flag
  }

  // If water level is too low, skip the moisture check and watering
  if (lowWaterLevel) {
    Serial.println("Skipping moisture check due to low water level.");
  } else {
    checkMoisture();
  }
  Serial.print("delaying for ");
  Serial.print(MOIST_CHECK_INTERVAL / 1000);
  Serial.println(" seconds");
  Serial.println("");

  delay(MOIST_CHECK_INTERVAL);
}