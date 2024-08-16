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
 * - 2024-08-16: (Ricardo Hernandez) Integrated the Load Cell with RGB Sensor into the automatic watering plant system. Merged Load Cell code into water.ino. Added docstrings for better documentation.
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
#include <EEPROM.h> // Include EEPROM library for storing the zero factor

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
const float LOW_WEIGHT_THRESHOLD = 3.50;
const float SUFFICIENT_WEIGHT_THRESHOLD = 4;
// const float FULL_PITCHER = 7.85;

// Calibration factor for scaling the raw data to weight units
const float CALIBRATION_FACTOR = -94500; // Adjust this value to match your scale setup

// Variables to store the last color values
int lastRed = 0;
int lastGreen = 0;
int lastBlue = 0;

HX711 scale;  // Create an instance of the HX711 class

/* DEFINE YOUR BOTANICAL PREFERENCES */
#define DRY 420
#define MOIST 710
#define SOAKED 1000

/* DEFINE CONSTANTS */
#define MOIST_CHECK_INTERVAL 10000   // milliseconds between checks
#define MOIST_SAMPLE_INTERVAL 200    // Reduced delay for sampling
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
int lastMoistAvg = 0;
int oldlastMoistAvg = 0;
bool needServicing = false;
bool wateredLast = false;

/* HELPER METHODS */


/**
 * @brief Sets the RGB LED color.
 * 
 * This function takes in three integer values corresponding to 
 * the red, green, and blue components of the color, and sets 
 * the LED to that color.
 * 
 * @param R Red intensity (0-255).
 * @param G Green intensity (0-255).
 * @param B Blue intensity (0-255).
 */
void setColor(int R, int G, int B) {
  lastRed = R;
  lastGreen = G;
  lastBlue = B;
  analogWrite(RED_PIN, R);
  analogWrite(GREEN_PIN, G);
  analogWrite(BLUE_PIN, B);
}


/**
 * @brief Resets all LED indicators.
 * 
 * This function turns off all moisture-related LED indicators 
 * (dry, moist, soaked).
 */
void resetLeds() {
  digitalWrite(dryLed, LOW);
  digitalWrite(moistLed, LOW);
  digitalWrite(soakedLed, LOW);
}



/**
 * @brief Sorts the moistValues array using the insertion sort algorithm.
 *
 * Insertion sort is a simple sorting algorithm that builds the final sorted array one item at a time.
 * It is much less efficient on large lists than more advanced algorithms such as quicksort, heapsort, or merge sort.
 * However, it is useful for small datasets or nearly sorted datasets.
 * The function sorts the array in place and prints the sorted array to the serial monitor.
 */
void insertionSort() {
  // sorts moistValues array using insertion sort
  for (int i = 1; i < MOIST_SAMPLES; i++) {
    int key = moistValues[i];
    int j = i - 1;
    
    // Move elements of moistValues[0..i-1], that are greater than key, to one position ahead of their current position
    while (j >= 0 && moistValues[j] > key) {
      moistValues[j + 1] = moistValues[j];
      j = j - 1;
    }
    moistValues[j + 1] = key;
  }

  Serial.print("sorted readings: ");
  for (int i = 0; i < MOIST_SAMPLES; i++) {
    Serial.print(moistValues[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

/**
 * @brief Checks the moisture level of the soil.
 * 
 * This function reads the moisture level from the soil, calculates 
 * the average moisture value, and determines if the plant needs watering. 
 * If the soil is too dry, it triggers the watering process.
 */
void checkMoisture() {
  // collects 10 readings, averages them and
  // determines an action depending on the result
  Serial.print("started reading at ");
  Serial.println(millis());
  digitalWrite(readLed, HIGH); // sets the reading led on

  for (int i = 0; i < MOIST_SAMPLES + 1; i++) {
    // perform reading                                                         
    digitalWrite(powerPin, HIGH);  // sets the POWER on
    int val = analogRead(probePin); // get probe reading 
    digitalWrite(powerPin, LOW);    // sets POWER off  

    // always discard first reading
    // probe will always return bogus value on first try
    if (i != 0) {
      // otherwise, store data
      // and print data to serial port
      moistValues[i - 1] = val; // store values
    }

    delay(MOIST_SAMPLE_INTERVAL); // wait to collect next reading
  }

  insertionSort(); // sort results in array using insertion sort
  oldlastMoistAvg = lastMoistAvg; // store old last moist value
  lastMoistAvg = (moistValues[(MOIST_SAMPLES / 2) - 1]); // pick median value
  Serial.print("median read: "); // serial output
  Serial.print(lastMoistAvg);
  Serial.print(" | ");

  // otherwise, test against thresholds
  // and if necessary perform watering  
  resetLeds();
  if (lastMoistAvg <= DRY) {
    Serial.println(" status: VERY DRY - watering plant");
    digitalWrite(dryLed, HIGH); // turn on dry led
    waterPlant(); // do watering
    wateredLast = true;
  }
  else if (lastMoistAvg > DRY && lastMoistAvg < MOIST) {
    Serial.println(" status: DRY - watering plant");
    digitalWrite(dryLed, HIGH); // turn on dry led
    waterPlant(); // do watering
    wateredLast = true;
  }
  else if (lastMoistAvg >= MOIST && lastMoistAvg < SOAKED) {
    Serial.println(" status: MOIST - plant is just fine");
    digitalWrite(moistLed, HIGH); // turn on moist led
    wateredLast = false;
  }
  else if (lastMoistAvg > SOAKED) {
    Serial.println(" status: SOAKED - don't water plant!");
    digitalWrite(soakedLed, HIGH); // turn on soaked led
    wateredLast = false;
  }

  digitalWrite(readLed, LOW); // sets the reading led off    
  Serial.println("");
}

/**
 * @brief Waters the plant based on moisture levels.
 * 
 * This function controls the relay to water the plant. It checks if the 
 * moisture level has changed significantly after watering. If not, it 
 * signals that the equipment may need servicing and suspends further execution.
 */
void waterPlant() {
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
/**
 * @brief Initializes the system.
 * 
 * This function sets up the necessary pins, initializes the serial communication, 
 * configures the HX711 scale, and loads the zero factor from EEPROM. It also 
 * sets the initial color of the RGB LED.
 */
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

  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  // Initialize the HX711 module
  scale.set_scale();  // Set the scale to the default calibration factor

  // Load zero factor from EEPROM
  long zero_factor;
  EEPROM.get(EEPROM_ADDRESS, zero_factor);  // Retrieve stored zero factor from EEPROM
  if (zero_factor == 0xFFFFFFFF) {  // Check if no valid zero factor is found in EEPROM
    Serial.println("No zero factor found in EEPROM. Please tare the scale manually.");
    scale.tare();  // Manually tare the scale (reset to 0)
    zero_factor = scale.read_average();  // Get a baseline reading for the zero factor
    EEPROM.put(EEPROM_ADDRESS, zero_factor);  // Save the new zero factor to EEPROM
  } else {
    scale.set_offset(zero_factor);  // Apply the stored zero factor to the scale
    Serial.println("Zero factor loaded from EEPROM.");
  }
  
  Serial.print("Zero factor: ");  // Display the loaded or newly set zero factor
  Serial.println(zero_factor);

  // Set the RGB pins as output
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  setColor(255, 0, 0); // Set initial LED color to red
}


/**
 * @brief Main loop of the program.
 * 
 * This function continuously checks the weight of the scale and the moisture level of the soil. 
 * It adjusts the LED color based on the weight reading and triggers the moisture check 
 * and watering process at specified intervals.
 */
void loop() {
  // Perform the weight measurement
  scale.set_scale(CALIBRATION_FACTOR);  // Apply the current calibration factor

  float weight = scale.get_units();  // Get the weight from the scale

  // Print the weight reading and the current calibration factor to the serial monitor
  Serial.print("Reading: ");
  Serial.print(weight, 2);  // Display the weight with 2 decimal places
  Serial.print(" lbs");
  Serial.print(" calibration_factor: ");
  Serial.print(CALIBRATION_FACTOR);
  Serial.println();

  // Set LED color based on weight conditions
  if (weight < EMPTY_WEIGHT_THRESHOLD) {
    setColor(255, 0, 0);  //  Red handled inside setColor now
  } else if (weight < LOW_WEIGHT_THRESHOLD) {
    setColor(255, 0, 0);  // Solid red
  } else if (weight < SUFFICIENT_WEIGHT_THRESHOLD) {
    setColor(255, 255, 0);  // Yellow
  } else {
    setColor(0, 255, 0);  // Green
  }

  checkMoisture();
  //output info to serial
  Serial.print("delaying for ");
  Serial.print(MOIST_CHECK_INTERVAL/1000);
  Serial.println(" seconds");
  Serial.println("");
    
  delay(MOIST_CHECK_INTERVAL);
}