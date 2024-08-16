/*
 * Application: Water 1.0
 * Author: Leo Marte
 * Refactor: Ricardo Hernandez
 *
 * Sets up a system to water a plant, by using sensors to probe water moisture
 * and opening a pump (via a relay) to water the plant when needed. Combination
 * of Botanicalls and Growduino project concepts.
 *
 * http://cs.gettysburg.edu/~martle02/cs450/
 */

/* DEFINE YOUR BOTANICAL PREFERENCES */

#define DRY 420               // after not being watered for ~5 days
#define MOIST 710             // midpoint between DRY and SOAKED
#define SOAKED 1000           // least desired level after watering

/* DEFINE CONSTANTS */

#define MOIST_CHECK_INTERVAL 10000   // milliseconds (1 minute) in between checks
#define MOIST_SAMPLE_INTERVAL 2000   // milliseconds over which to average reading samples
#define WATER_INTERVAL 3000          // milliseconds to allow for water to flow
#define MOIST_SAMPLES 10             // # of samples to average when reading moisture
#define FAILSAFE_VALUE 200           // minimum difference in moisture that indicates watering has happened

/* PROGRAM VARIABLES AND BOARD PINS */

//LEDs
#define onLed 2      // device is on
#define readLed 3    // device is reading
#define workLed 4    // device is watering
#define dryLed 5     // plant is anywhere below MOIST
#define moistLed 6   // plant is equal or higher than MOIST and less than SOAKED
#define soakedLed 7  // plant is equal or higher than SOAKED

//probes
#define powerPin 8   // sends current to plant
#define relayPin 9   // send current to activate relay
#define probePin 0   // analog 0: collects current from plant

int moistValues[MOIST_SAMPLES];  // array to store readings to average
unsigned long lastMoistTime = 0; // storage for millis of the most recent moisture reading
unsigned long lastWaterTime = 0; // storage for millis of the most recent watering reading
int lastMoistAvg = 0;            // storage for moisture average of the most recent moisture reading
int oldlastMoistAvg = 0;         // temp var for failsafe test
boolean needServicing = false;   // set if there is a faulty probe
boolean wateredLast = false;

/* HELPER METHODS */

void resetLeds() {
  // turn off all plant status leds
  digitalWrite(dryLed, LOW);
  digitalWrite(moistLed, LOW);
  digitalWrite(soakedLed, LOW);
}

void insertionSort() {
  // sorts moistValues array using insertion sort
  for (int i = 1; i < MOIST_SAMPLES; i++) {
    int key = moistValues[i];
    int j = i - 1;

    while (j >= 0 && moistValues[j] > key) {
      moistValues[j + 1] = moistValues[j];
      j--;
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

void checkMoisture() {
  // collects 10 readings, averages them, and determines an action depending on the result
  Serial.print("started reading at ");
  Serial.println(millis());
  digitalWrite(readLed, HIGH);  // sets the reading led on

  for (int i = 0; i < MOIST_SAMPLES + 1; i++) {
    // perform reading  
    digitalWrite(powerPin, HIGH);      // sets the POWER on
    int val = analogRead(probePin);    // get probe reading 
    digitalWrite(powerPin, LOW);       // sets POWER off  

    // always discard the first reading
    if (i != 0) {
      // store data
      moistValues[i - 1] = val;  // store values
      lastMoistTime = millis();
    }

    delay(MOIST_SAMPLE_INTERVAL); // wait to collect the next reading
  }

  insertionSort();                   // sort results in array
  oldlastMoistAvg = lastMoistAvg;    // store old last moist value
  lastMoistAvg = moistValues[MOIST_SAMPLES / 2];  // pick the median value
  Serial.print("median read: ");     // serial output
  Serial.print(lastMoistAvg);
  Serial.print(" | ");

  // test against thresholds and, if necessary, perform watering  
  resetLeds();
  if (lastMoistAvg <= DRY) {
    Serial.println(" status: VERY DRY - watering plant");
    digitalWrite(dryLed, HIGH);  // turn on dry led
    waterPlant();                // do watering
    wateredLast = true;
  } else if (lastMoistAvg > DRY && lastMoistAvg < MOIST) {
    Serial.println(" status: DRY - watering plant");
    digitalWrite(dryLed, HIGH);  // turn on dry led
    waterPlant();                // do watering
    wateredLast = true;
  } else if (lastMoistAvg >= MOIST && lastMoistAvg < SOAKED) {
    Serial.println(" status: MOIST - plant is just fine");
    digitalWrite(moistLed, HIGH);  // turn on moist led
    wateredLast = false;
  } else if (lastMoistAvg > SOAKED) {
    Serial.println(" status: SOAKED - don't water plant!");
    digitalWrite(soakedLed, HIGH);  // turn on soaked led
    wateredLast = false;
  }

  digitalWrite(readLed, LOW);  // sets the reading led off    
  Serial.println("");
}

void waterPlant() {
  // opens a 5V line to feed a relay that will power up a pump to dispense water
  // on the plant

  // do a test for failsafe
  int diffMoist = lastMoistAvg - oldlastMoistAvg;

  if (wateredLast && diffMoist <= FAILSAFE_VALUE) {
    needServicing = true;
    Serial.println("");
    Serial.println("water 1.0 is suspending execution...");
    Serial.println("EQUIPMENT NEEDS SERVICING!");
    resetLeds();
    digitalWrite(readLed, LOW);
    while (true) {
      // if sensor has an issue, flash onLed
      digitalWrite(onLed, LOW);
      delay(1000);
      digitalWrite(onLed, HIGH);
      delay(1000);
    }
  } else {
    // output info to serial
    Serial.print("watering started at: ");
    Serial.print(millis());
    digitalWrite(workLed, HIGH);  // light on working led

    // open relay, delay and close
    digitalWrite(relayPin, HIGH);
    delay(WATER_INTERVAL);
    digitalWrite(relayPin, LOW);  // COMMENTED ONLY WHEN THERE IS NO RELAY ATTACHED

    // output info to serial
    digitalWrite(workLed, LOW);  // output info to serial
    Serial.print(", it lasted: ");
    Serial.print(WATER_INTERVAL);
    Serial.print(" seconds");
    Serial.println("");

    wateredLast = true;
  }
}

/* arduino methods */
void setup() {  // run once, when the sketch starts
  // set all led pins, power & relay pins as outputs
  pinMode(onLed, OUTPUT);
  pinMode(readLed, OUTPUT);
  pinMode(workLed, OUTPUT);
  pinMode(dryLed, OUTPUT);
  pinMode(moistLed, OUTPUT);
  pinMode(soakedLed, OUTPUT);
  pinMode(powerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(onLed, HIGH);  // turns on power light
  Serial.begin(9600);         // open serial communications at 9600 bps  
  Serial.println("water 1.0 up and running...");
  Serial.println("");
}

void loop() {  // run over and over again
  // make a moisture check
  checkMoisture();

  // output info to serial
  Serial.print("delaying for ");
  Serial.print(MOIST_CHECK_INTERVAL / 1000);
  Serial.println(" seconds");
  Serial.println("");

  delay(MOIST_CHECK_INTERVAL);
}