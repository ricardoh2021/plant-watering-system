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
const float EMPTY_WEIGHT_THRESHOLD = 1.65; // Under this is empty
const float LOW_WEIGHT_THRESHOLD = 3.00;
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

const long DEFAULT_ZERO_FACTOR = 28974;  // Default zero factor in case EEPROM read fails

boolean loadCellError = false;  // Global variable to track load cell errors

// Define a constant for the brightness level (percentage from 0 to 100)
const float BRIGHTNESS_LEVEL = 0.5; // 50% brightness


/* HELPER METHODS */

/**
 * @brief Sets the color of the RGB LED by adjusting the brightness of the red, green, and blue components.
 * 
 * This function takes in RGB values and scales them according to the specified brightness level.
 * The brightness level can be modified via the BRIGHTNESS_LEVEL constant.
 * 
 * @param R The intensity of the red component (0 to 255).
 * @param G The intensity of the green component (0 to 255).
 * @param B The intensity of the blue component (0 to 255).
 * 
 * The function scales each RGB value based on the BRIGHTNESS_LEVEL constant before writing them to the
 * respective RGB pins using analogWrite.
 */
void setColor(int R, int G, int B) {
    analogWrite(RED_PIN, R * BRIGHTNESS_LEVEL);
    analogWrite(GREEN_PIN, G * BRIGHTNESS_LEVEL);
    analogWrite(BLUE_PIN, B * BRIGHTNESS_LEVEL);
}

/**
 * @brief Function to blink an LED with customizable color and duration.
 * 
 * @param R Red component (0-255).
 * @param G Green component (0-255).
 * @param B Blue component (0-255).
 * @param duration Total duration for blinking (milliseconds).
 * @param blinkInterval Interval between blinks (milliseconds).
 */
void blinkLed(int R, int G, int B, unsigned long duration, unsigned long blinkInterval) {
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    setColor(R, G, B);
    delay(blinkInterval);
    setColor(0, 0, 0); // Turn off the LED
    delay(blinkInterval);
  }
}

/**
 * @brief Blink the RGB LED in purple color.
 * 
 * The LED blinks purple for a duration of 9000 milliseconds with a 500 milliseconds interval.
 */
void blinkPurple() {
  blinkLed(128, 0, 128, 9000, 500);
}

/**
 * @brief Blink the RGB LED in red color.
 * 
 * The LED blinks red for a duration of 9000 milliseconds with a 500 milliseconds interval.
 */
void blinkRed() {
  blinkLed(255, 0, 0, 9000, 500);
}

/**
 * @brief Blink the RGB LED in blue color.
 * 
 * The LED blinks blue for a duration of 9000 milliseconds with a 500 milliseconds interval.
 */
void blinkBlue() {
  blinkLed(0, 0, 255, 9000, 500);
}

/**
 * @brief Reset the state of the moisture status LEDs.
 * 
 * This function turns off all moisture status LEDs (dry, moist, soaked).
 */
void resetLeds() {
  digitalWrite(dryLed, LOW);
  digitalWrite(moistLed, LOW);
  digitalWrite(soakedLed, LOW);
}

/**
 * @brief Sort an array of integers using the insertion sort algorithm.
 * 
 * @param arr Pointer to the array to be sorted.
 * @param n Number of elements in the array.
 */
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

/**
 * @brief Perform moisture readings by averaging multiple samples.
 * 
 * Reads moisture sensor values over a defined interval, sorts the readings, and updates
 * the average moisture level.
 */
void performMoistureReadings() {
  for (int i = 0; i < MOIST_SAMPLES + 1; i++) {
    digitalWrite(powerPin, HIGH);
    int val = analogRead(probePin);
    digitalWrite(powerPin, LOW);

    if (i != 0) {
      moistValues[i - 1] = val;
      lastMoistTime = millis();
    }

    delay(MOIST_SAMPLE_INTERVAL);
  }

  insertionSort(moistValues, MOIST_SAMPLES);
  oldlastMoistAvg = lastMoistAvg;
  lastMoistAvg = moistValues[MOIST_SAMPLES / 2];
}

/**
 * @brief Update the moisture status and control the watering process.
 * 
 * Updates the status LEDs based on the average moisture level and triggers the watering
 * process if needed.
 */
void updateMoistureStatus() {
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
}

/**
 * @brief Perform a moisture check and update the plant watering status.
 * 
 * Reads the moisture sensor values, prints the average moisture level, and updates
 * the moisture status.
 */
void checkMoisture() {
  digitalWrite(readLed, HIGH);
  performMoistureReadings();
  Serial.print("median read: ");
  Serial.print(lastMoistAvg);
  Serial.print(" | ");
  updateMoistureStatus();
  digitalWrite(readLed, LOW);
}

/**
 * @brief Water the plant by activating the relay to control the pump.
 * 
 * If the moisture readings are not within the acceptable range or if the watering fails,
 * the system will enter a service mode. The pump will be activated to water the plant
 * for a defined interval.
 */
void waterPlant() {
  if(!pumpEnabled){
    return;
  }

  // Test for failsafe condition
  int diffMoist = lastMoistAvg - oldlastMoistAvg;

  if( wateredLast && diffMoist <= FAILSAFE_VALUE ){
    needServicing = true;
    Serial.println("");
    Serial.println("water 1.0 is suspending execution...");
    Serial.println("EQUIPMENT NEEDS SERVICING!");
    resetLeds();
    digitalWrite(readLed, LOW);
    while(true){
      // If sensor has an issue, flash onLed
      digitalWrite(onLed, LOW); 
      delay(1000);      
      digitalWrite(onLed, HIGH);
      delay(1000);  
    }
  }
  else{
    // Output info to serial
    Serial.print("watering started at: ");
    Serial.print(millis());
    digitalWrite(workLed, HIGH); // Light on working LED

    // Open relay, delay, and close
    digitalWrite(relayPin, HIGH);
    delay(WATER_INTERVAL);
    digitalWrite(relayPin, LOW);  // COMMENTED ONLY WHEN THERE IS NO RELAY ATTACHED

    // Output info to serial
    digitalWrite(workLed, LOW);     
    Serial.print(", it lasted: ");
    Serial.print(WATER_INTERVAL);
    Serial.print(" seconds");
    Serial.println("");
    
    wateredLast = true;
  }
}

/**
 * @brief Initialize the Arduino board and set up the HX711 scale.
 * 
 * Configures the pin modes for LEDs, relay, and other components. Initializes serial
 * communication, starts the HX711 scale, and loads the zero factor from EEPROM.
 * Sets the calibration factor and offset for the scale.
 */
void setup() {
  pinMode(onLed, OUTPUT);
  pinMode(readLed, OUTPUT);
  pinMode(workLed, OUTPUT);
  pinMode(dryLed, OUTPUT);
  pinMode(moistLed, OUTPUT);
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
  
  if (zero_factor == 0xFFFFFFFF || isnan(zero_factor) || zero_factor == 0) {
    Serial.println("EEPROM read failed or data is invalid. Using default zero factor.");
    zero_factor = DEFAULT_ZERO_FACTOR;  // Use default zero factor
    EEPROM.put(EEPROM_ADDRESS, zero_factor);  // Attempt to write default value back to EEPROM
  }
  
  scale.set_offset(zero_factor);  // Apply the zero factor

  // Set the RGB pins as output
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

/**
 * @brief Check for load cell errors by verifying if the weight readings are within a plausible range.
 * 
 * Evaluates weight readings for stability and plausibility over a defined interval. If the readings
 * consistently fall outside the plausible range, an error is flagged.
 * 
 * @param weight Current weight reading from the scale.
 * @return true if a load cell error is detected; otherwise, false.
 */
bool checkLoadCellError(float weight) {
  // Define plausible weight range (adjust as needed)
  const float MIN_PLAUSIBLE_WEIGHT = -0.02;  // Minimum plausible weight
  const float MAX_PLAUSIBLE_WEIGHT = 10.0;   // Maximum plausible weight

  // Example stability check: verify if weight readings are consistently out of bounds
  static float prevWeight = 0;
  static unsigned long lastCheckTime = 0;
  const unsigned long CHECK_INTERVAL = 5000;  // Check interval in milliseconds
  const int STABILITY_COUNT = 10;  // Number of consecutive out-of-bound readings for error

  if (millis() - lastCheckTime > CHECK_INTERVAL) {
    lastCheckTime = millis();
    prevWeight = weight;  // Update previous weight reading
    return false;  // No error detected yet
  }

  static int outOfBoundCount = 0;

  if (weight < MIN_PLAUSIBLE_WEIGHT || weight > MAX_PLAUSIBLE_WEIGHT) {
    outOfBoundCount++;
    if (outOfBoundCount >= STABILITY_COUNT) {
      loadCellError = true;
      return true;  // Error detected
    }
  } else {
    outOfBoundCount = 0;  // Reset count if within bounds
  }

  loadCellError = false;
  return false;  // No error detected
}

/**
 * @brief Main loop of the Arduino program that performs regular tasks.
 * 
 * Continuously reads the weight from the HX711 scale, checks for load cell errors, and updates
 * the status of the plant watering system based on the weight and moisture readings. 
 * Includes a delay between iterations.
 */
void loop() {
  scale.set_scale(CALIBRATION_FACTOR);  // Apply the current calibration factor

  float weight = scale.get_units();  // Get the weight from the scale
  Serial.print("Weight is: ");
  Serial.print(weight);
  Serial.print(" lbs.");
  Serial.println();

  // Check for load cell errors
  if (checkLoadCellError(weight)) {
    Serial.println("Load cell error detected. Pump will be enabled regardless of water level.");
    blinkPurple(); // Blink purple to indicate error

    // Default mode: Enable pump regardless of water level
    pumpEnabled = true;
    lowWaterLevel = false;  // Clear the low water level flag in error state
    setColor(255, 0, 0);    // Optional: Indicate error with red LED
  } else {
    // Set LED color based on weight conditions and check water level
    if (weight < EMPTY_WEIGHT_THRESHOLD) {
      blinkRed();
      lowWaterLevel = true;  // Set the flag indicating low water level
      Serial.println("Water level too low! May harm pump.");
      pumpEnabled = false;
    } else if (weight < LOW_WEIGHT_THRESHOLD) {
      blinkBlue();
      lowWaterLevel = true;  // Set the flag indicating low water level
      Serial.println("Water level too low! Pump deactivated.");
    } else if (weight < SUFFICIENT_WEIGHT_THRESHOLD) {
      setColor(255, 255, 0);  // Yellow
      lowWaterLevel = false;  // Water level is sufficient, clear the flag
      pumpEnabled = true;
    } else {
      setColor(0, 255, 0);  // Green
      lowWaterLevel = false;  // Water level is sufficient, clear the flag
      pumpEnabled = true;
    }
  }

  // If water level is too low, load cell error detected, or pump failure detected, skip the moisture check
  if (lowWaterLevel || loadCellError) {
    Serial.println("Issues have occurred. Check Water Level. Otherwise, issue with Load Cell Scale.");
  }

  checkMoisture();

  Serial.print("delaying for ");
  Serial.print(MOIST_CHECK_INTERVAL / 1000);
  Serial.println(" seconds");
  Serial.println("");

  delay(MOIST_CHECK_INTERVAL);
}