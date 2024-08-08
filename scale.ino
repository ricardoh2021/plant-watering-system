#include "HX711.h"
#include <EEPROM.h> // Include EEPROM library for storing the zero factor

// Pin definitions for the HX711 module
#define LOADCELL_DOUT_PIN  3  // Pin connected to DOUT of HX711
#define LOADCELL_SCK_PIN   2  // Pin connected to SCK of HX711

// EEPROM memory address for storing the zero factor
#define EEPROM_ADDRESS 0

HX711 scale;  // Create an instance of the HX711 class

// Calibration factor for scaling the raw data to weight units
float calibration_factor = -93999; // Adjust this value to match your scale setup

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
}

/**
 * @brief Arduino main loop function.
 * 
 * Continuously reads the weight from the scale, applies the calibration factor, 
 * and prints the results to the serial monitor. The calibration factor can be adjusted
 * through the serial interface.
 */
void loop() {
  scale.set_scale(calibration_factor);  // Apply the current calibration factor

  // Print the weight reading and the current calibration factor to the serial monitor
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 2);  // Display the weight with 2 decimal places
  Serial.print(" lbs");  // Display the units (lbs by default)
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  delay(1000);  // Delay for 1 second to allow stable readings
}