/*
 * Application: Water 1.0
 * Author: Leo Marte
 *
 * Sets up a system to water a plant, using sensors to probe water moisture
 * and opening a pump (via a relay) to water the plant when needed.
 */

#include "HX711.h"
#include <EEPROM.h> // Include EEPROM library for storing the zero factor

#define DRY 420
#define MOIST 710
#define SOAKED 1000
#define FAILSAFE_VALUE 200

#define MOIST_CHECK_INTERVAL 10000   // milliseconds (10 seconds) between checks
#define MOIST_SAMPLE_INTERVAL 2000   // milliseconds over which to average readings
#define WATER_INTERVAL 3000          // milliseconds to allow for water to flow
#define MOIST_SAMPLES 10             // Number of samples to average

// LED and pin definitions
#define onLed 2
#define readLed 3
#define workLed 4
#define dryLed 5
#define moistLed 6
#define soakedLed 7

#define powerPin 8
#define relayPin 9
#define probePin A0

int moistValues[MOIST_SAMPLES];
unsigned long lastMoistTime = 0;
unsigned long lastWaterTime = 0;
int lastMoistAvg = 0;
int oldlastMoistAvg = 0;
bool needServicing = false;
bool wateredLast = false;

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
const float EMPTY_WEIGHT_THRESHOLD = 1.55;
const float LOW_WEIGHT_THRESHOLD = 3.15;
const float SUFFICIENT_WEIGHT_THRESHOLD = 3.75;

// Calibration factor for scaling the raw data to weight units
const float CALIBRATION_FACTOR = -94500; // Adjust this value to match your scale setup

HX711 scale;  // Create an instance of the HX711 class

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

int getMedian(int arr[], int size) {
  int temp;
  // Copy and sort the array
  int sortedArr[size];
  memcpy(sortedArr, arr, size * sizeof(int));
  
  // Simple selection sort for small array
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (sortedArr[j] < sortedArr[i]) {
        temp = sortedArr[i];
        sortedArr[i] = sortedArr[j];
        sortedArr[j] = temp;
      }
    }
  }
  return sortedArr[size / 2]; // Return median
}

void checkMoisture() {
  Serial.print("started reading at ");
  Serial.println(millis());
  digitalWrite(readLed, HIGH);

  // Collect readings
  for (int i = 0; i < MOIST_SAMPLES; i++) {
    digitalWrite(powerPin, HIGH);
    moistValues[i] = analogRead(probePin);
    digitalWrite(powerPin, LOW);
    delay(MOIST_SAMPLE_INTERVAL);
  }

  lastMoistTime = millis();
  
  // Calculate median
  oldlastMoistAvg = lastMoistAvg;
  lastMoistAvg = getMedian(moistValues, MOIST_SAMPLES);
  
  Serial.print("median read: ");
  Serial.print(lastMoistAvg);
  Serial.println("");

  resetLeds();
  if (lastMoistAvg <= DRY) {
    Serial.println("status: VERY DRY - watering plant");
    digitalWrite(dryLed, HIGH);
    waterPlant();
    wateredLast = true;
  } else if (lastMoistAvg > DRY && lastMoistAvg < MOIST) {
    Serial.println("status: DRY - watering plant");
    digitalWrite(dryLed, HIGH);
    waterPlant();
    wateredLast = true;
  } else if (lastMoistAvg >= MOIST && lastMoistAvg < SOAKED) {
    Serial.println("status: MOIST - plant is just fine");
    digitalWrite(moistLed, HIGH);
    wateredLast = false;
  } else if (lastMoistAvg > SOAKED) {
    Serial.println("status: SOAKED - don't water plant!");
    digitalWrite(soakedLed, HIGH);
    wateredLast = false;
  }

  digitalWrite(readLed, LOW);
  Serial.println("");
}

void waterPlant() {
  int diffMoist = lastMoistAvg - oldlastMoistAvg;

  if (wateredLast && diffMoist <= FAILSAFE_VALUE) {
    needServicing = true;
    Serial.println("water 1.0 is suspending execution...");
    Serial.println("EQUIPMENT NEEDS SERVICING!");
    resetLeds();
    digitalWrite(readLed, LOW);
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
    Serial.println(" milliseconds");

    wateredLast = true;
  }
}

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
}

void loop() {
  checkMoisture();
  Serial.print("Delaying for ");
  Serial.print(MOIST_CHECK_INTERVAL / 1000);
  Serial.println(" seconds");

  scale.set_scale(CALIBRATION_FACTOR);  // Apply the current calibration factor

  float weight = scale.get_units();  // Get the weight from the scale

  // Print the weight reading and the current calibration factor to the serial monitor
  Serial.print("Reading: ");
  Serial.print(weight, 2);  // Display the weight with 2 decimal places
  Serial.print(" lbs");
  Serial.print(" Calibration Factor: ");
  Serial.println(CALIBRATION_FACTOR);

  // Set LED color based on weight conditions
  if (weight < 1.55) {
    setColor(255, 0, 0); // Solid red if weight is less than EMPTY_WEIGHT_THRESHOLD
  } else if (weight >= 1.55 && weight < 3.15) {
    setColor(255, 255, 0); // Yellow if weight is between EMPTY_WEIGHT_THRESHOLD and LOW_WEIGHT_THRESHOLD
  } else if (weight >= 3.15 && weight < 3.75) {
    setColor(0, 255, 0); // Green if weight is between LOW_WEIGHT_THRESHOLD and SUFFICIENT_WEIGHT_THRESHOLD
  } else {
    setColor(0, 255, 0); // Green if weight is SUFFICIENT_WEIGHT_THRESHOLD or more
  }
  
  delay(MOIST_CHECK_INTERVAL);
}

// void testSetColor() {
//   setColor(255, 0, 0); // Red
//   delay(1000);
//   setColor(0, 255, 0); // Green
//   delay(1000);
//   setColor(0, 0, 255); // Blue
//   delay(1000);
//   setColor(255, 255, 0); // Yellow
//   delay(1000);
// }

