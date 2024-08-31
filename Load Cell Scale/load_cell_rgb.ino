#include "HX711.h"
#include <EEPROM.h> // Include EEPROM library for storing the zero factor

/**
 * @file load_cell_rgb.ino
 * @brief This Arduino sketch interfaces with a 10kg load cell using the HX711 module and controls an RGB LED to indicate the water level in a pitcher.
 * 
 * Author: Ricardo Hernandez
 * Date: August 2024
 *
 * Description:
 * This sketch reads weight data from a 10kg load cell connected to the HX711 module. The load cell measures the weight of a water pitcher. 
 * Based on the weight, the RGB LED displays different colors to indicate the water level:
 * - Red: Empty or low water level
 * - Yellow: Sufficient water level
 * - Green: Full water pitcher
 *
 * The code also includes calibration and zero factor handling using EEPROM to ensure accurate weight measurements.
 */

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

HX711 scale;  // Create an instance of the HX711 class

/**
 * @brief Arduino setup function.
 * 
 * Initializes serial communication, sets up the HX711 load cell, 
 * and loads the zero factor from EEPROM. If no valid zero factor is found,
 * the scale is tared, and the zero factor is stored in EEPROM.
 */
void setup() {
  Serial.begin(9600);  // Initialize serial communication at 9600 baud rate
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

  setColor(137, 207, 240); // Initial color (baby blue)
}

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

/**
 * @brief Arduino main loop function.
 * 
 * Continuously reads the weight from the scale, applies the calibration factor, 
 * and prints the results to the serial monitor. Additionally, changes the LED color
 * based on the weight.
 */
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

  // Set LED color based on weight conditions
  if (weight < EMPTY_WEIGHT_THRESHOLD) {
    Serial.println("Water pitcher is empty");
    blinkRed();  // Blink red LED if weight is less than 1.55 lbs

  } else if (weight >= EMPTY_WEIGHT_THRESHOLD && weight < LOW_WEIGHT_THRESHOLD) {
    Serial.println("Water pitcher is getting low");
    setColor(255, 0, 0); // Solid red if weight is between 1.55 and 3.15 lbs
  } else if (weight >= LOW_WEIGHT_THRESHOLD && weight < SUFFICIENT_WEIGHT_THRESHOLD) {
    Serial.println("Water pitcher has sufficient weight");
    setColor(255, 255, 0); // Yellow if weight is between 3.15 and 3.75 lbs
  } else {
    Serial.println("Water pitcher is full");
    setColor(0, 255, 0); // Green if weight is 3.75 lbs or more
  }
  
  // Check for serial input to reset the zero factor
  if (Serial.available() > 0) {
    char input = Serial.read();  // Read the input character
    if (input == 'z' || input == 'Z') {  // Check if the input is 'z' or 'Z'
      scale.tare();  // Reset the scale to zero
      long zero_factor = scale.read_average();  // Get the new zero factor
      EEPROM.put(EEPROM_ADDRESS, zero_factor);  // Store the new zero factor in EEPROM
      Serial.println("Zero factor reset and stored in EEPROM.");
      Serial.print("New zero factor: ");
      Serial.println(zero_factor);
    }
  }

  delay(1000);  // Delay for 1 second to allow stable readings
}


