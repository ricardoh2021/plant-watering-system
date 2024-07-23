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

/* DEFINE YOUR BOTANICAL PREFERENCES */

//the botanicalls project defines by default
//the following threshold for your plant:
//#define MOIST 425             // minimum level of satisfactory moisture
//#define DRY 300               // maximum level of tolerable dryness      
//#define SOAKED 575            // minimum desired level after watering
//#define WATERING_CRITERIA 125 // minimum change in value that indicates watering

//I used the previous thresholds as a starting point
//to determine new ones for my pathos plant. below are the results:
#define DRY 420               // after not being watered for ~5 days
#define MOIST 710             // midpoint between DRY and SOAKED
#define SOAKED 1000           // least desired level after watering

//in order to figure out the bottom-line, or the DRY value, I left the pathos
//without water for about 5 days until the surface was completely dry, and the
//leaves started to get yellow (i know its cruel). At that point the reading
//was 420 on the analog input. I then poured a cup of water on it, and the
//reading value went up to about a 1000. I called that value my SOAKED value
//since I wouldn't want my plant to be more watered than that. Finally,
//I considered MOIST to be the midpoint between being too dry and soaked.
//Below is an example of my rationale:

//variation:
//1 cup of water -> 1000 (sensor reading) @ 7.30pm on 12/9/2008
//criteria of cup of water is: ~600 (reading increases by that amount)
//  if 1000 is SOAKED, then the midpoint between 420 and 1000: 710 is MOIST

/* DEFINE CONSTANTS */

//define here how often you would like things to happen, from moisture check
//frequency, to what do you want it to be the interval between reads to average,
//to how long to keep the water running. also select how many total samples you
//would like to collect to determine moisture levels every time the system checks

#define MOIST_CHECK_INTERVAL 20000   // milliseconds (1 minutes) in between checks
//#define MOIST_CHECK_INTERVAL 3600000 // milliseconds (1 hour) in between checks
#define MOIST_SAMPLE_INTERVAL 2000   // milliseconds over which to average reading samples
//#define MOIST_SAMPLE_INTERVAL 5000 // milliseconds over which to average reading samples

#define WATER_INTERVAL 3000          // milliseconds to allow for water to flow
// this value will change according to the kind of pump
// that you will use. some pumps will need more time
// to get  the water to the plant than the default 3 seconds.

#define MOIST_SAMPLES 10             // # of samples to average when reading moisture
#define FAILSAFE_VALUE 200           // minimum difference in moisture that indicates watering has happened

/* PROGRAM VARIABLES AND BOARD PINS */

//LEDs               //light up when:
#define onLed 2      //device is on
#define readLed 3    //device is reading
#define workLed 4    //device is watering
#define dryLed 5     //plant is anywhere below MOIST
#define moistLed 6   //plant is equal or higher than MOIST and less than SOAKED
#define soakedLed 7  //plant is equal or higher than SOAKED

//probes
#define powerPin 8 //sends current to plant
#define relayPin 9 //send current to activate relay
#define probePin 0 //analog 0: collects current from plant

int moistValues[MOIST_SAMPLES];  // array to store readings to average
unsigned long lastMoistTime=0;   // storage for millis of the most recent moisture reading
unsigned long lastWaterTime=0;   // storage for millis of the most recent watering reading
int lastMoistAvg=0;              // storage for moisture average of the most recent moisture reading
int oldlastMoistAvg=0;           // temp var for failsafe test
boolean needServicing = false;   // set if there is a faulty probe
boolean wateredLast = false;
/* HELPER METHODS */


/**
 * @brief Resets all plant status LEDs to the OFF state.
 *
 * This function turns off the dry, moist, and soaked LEDs to indicate that no
 * specific condition is currently being met. It helps in resetting the LED status
 * before making new checks or taking actions.
 */
void resetLeds(){
  //turn off all plant status leds
  digitalWrite(dryLed,     LOW);
  digitalWrite(moistLed,   LOW);
  digitalWrite(soakedLed,  LOW);
}

/**
 * @brief Sorts an array using the Insertion Sort algorithm.
 *
 * This function implements the Insertion Sort algorithm, which is efficient for
 * small arrays. It sorts the array in-place by building the sorted array one element
 * at a time, picking the next element and placing it at the correct position. Also space
 * complexity of O(1).
 * *
 * @note This function has a time complexity of O(n^2) in the worst case but performs well
 *       for small arrays.
 * @note (Ricardo Hernandez) Added this function to improve performance and readability for small datasets.
 */
void insertionSort() {
  for (int i = 1; i < MOIST_SAMPLES; i++) {
    int key = moistValues[i];
    int j = i - 1;
    

    // Move elements of arr[0..i-1], that are greater than key,
    // to one position ahead of their current position
    while (j >= 0 && moistValues[j] > key) {
      moistValues[j + 1] = moistValues[j];
      j = j - 1;
    }
    moistValues[j + 1] = key;
  }

  // Print the sorted array
  Serial.print("sorted readings at end of insertionSort: ");
  for (int i = 0; i < MOIST_SAMPLES; i++) {
    Serial.print(moistValues[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

// void selectionSort(){
//   //sorts moistValues array using selection sort

//   int indexOfMin;  //vars
//   int pass;        //-
//   int j;           //-

//   //perform selection sort
//   for ( pass = 0; pass < MOIST_SAMPLES - 1; pass++ ) { 
//     indexOfMin = pass; 

//     for ( j = pass + 1; j < MOIST_SAMPLES; j++ ) 
//       if ( moistValues[j] < moistValues[pass] ) 
//         indexOfMin = j; 

//     int temp; 
//     temp = moistValues[pass]; 
//     moistValues[pass] = moistValues[indexOfMin]; 
//     moistValues[indexOfMin] = temp; 
//   }
 
//   Serial.print("sorted readings: ");
//   for(int i=0; i<MOIST_SAMPLES; i++){
//     Serial.print(moistValues[i]);
//     Serial.print(" ");
//   }
//   Serial.println(""); 
// }

/**
 * @brief Collects and processes moisture readings from the probe.
 *
 * This function collects multiple moisture readings, averages them, and determines
 * the plant's watering status based on the median value. It also sorts the readings
 * to compute the median value, which is used to make decisions on whether to water the plant.
 * The function then updates the status LEDs and calls the `waterPlant` function if necessary.
 */
void checkMoisture(){
  //collects 10 readings, averages them and 
  //determines an action depending on the result
  Serial.print("started reading at ");
  Serial.println(millis());
  digitalWrite(readLed, HIGH);         // sets the reading led on

  for(int i=0; i<MOIST_SAMPLES+1; i++){
    //perform reading  
    digitalWrite(powerPin, HIGH);      // sets the POWER on
    int val = analogRead(probePin);    // get probe reading 
    digitalWrite(powerPin, LOW);       // sets POWER off  


    //always discard first reading
    //probe will always return bogus value on first try
    if(i!=0){
      //otherwise, store data
      //and print data to serial port
      moistValues[i-1] = val;          // store values
      lastMoistTime = millis();          
      
      /*Serial.print("read #");          // print to serial
      Serial.print(i);
      Serial.print(" ");
      Serial.print("time: ");
      Serial.print(lastMoistTime);
      Serial.print(" ");
      Serial.print("read: ");
      Serial.println(val);*/
    }

    delay(MOIST_SAMPLE_INTERVAL); // wait to collect next reading
  }  

  
  //average all readings -- AVERAGE DEPRECATED
  /*int totalRead = 0;
  for(int i=0; i<MOIST_SAMPLES; i++){ 
   totalRead += moistValues[i]; 
  }
  lastMoistAvg = totalRead/MOIST_SAMPLES;
  Serial.print("average read: ");
  Serial.print(lastMoistAvg);
   */
  
  insertionSort();                    //sort results in array
  oldlastMoistAvg = lastMoistAvg;     //store old last moist value
  lastMoistAvg = (moistValues[(MOIST_SAMPLES/2)-1]); //pick median value
  Serial.print("median read: ");      //serial output
  Serial.print(lastMoistAvg);  
  Serial.print(" | ");
  
  
  //otherwise, test against thresholds
  //and if necessary perform watering  
  resetLeds();
  if(lastMoistAvg <= DRY){
    Serial.println(" status: VERY DRY - watering plant");
    digitalWrite(dryLed, HIGH);     //turn on dry led
    waterPlant();                   //do watering
    wateredLast = true;
  }
  else if(lastMoistAvg > DRY && lastMoistAvg < MOIST){
    Serial.println(" status: DRY - watering plant");
    digitalWrite(dryLed, HIGH);     //turn on dry led
    waterPlant();                   //do watering
    wateredLast = true;
  }
  else if(lastMoistAvg >= MOIST && lastMoistAvg < SOAKED){
    Serial.println(" status: MOIST - plant is just fine");
    digitalWrite(moistLed, HIGH);   //turn on moist led
    wateredLast = false;
  }
  else if(lastMoistAvg > SOAKED){
    Serial.println(" status: SOAKED - don't water plant!");
    digitalWrite(soakedLed, HIGH);  //turn on soaked led
    wateredLast = false;
  }

  digitalWrite(readLed, LOW);       // sets the reading led off    
  Serial.println("");
}

/**
 * @brief Manages the watering of the plant.
 *
 * This function activates the relay to turn on the pump and dispense water to the plant.
 * It includes a failsafe check to ensure that the watering process was effective.
 * If the difference in moisture levels before and after watering is below a threshold,
 * it triggers an alert for servicing.
 */
void waterPlant(){
  //opens a 5V line to feed a relay
  //that will power up a pump to dispense water
  //on the plant

    //do a test for failsafe
  //if the difference between the most recent median
  //reading, and the one before is less than the given
  //FAILSAFE_VALUE then raise the service flag
  int diffMoist = lastMoistAvg - oldlastMoistAvg;
  //Serial.print("diffMoist: ");
  //Serial.println(diffMoist);
  
  if( wateredLast && diffMoist <= FAILSAFE_VALUE ){
    needServicing = true;
    Serial.println("");
    Serial.println("water 1.0 is suspending execution...");
    Serial.println("EQUIPMENT NEEDS SERVICING!");
    resetLeds();
    digitalWrite(readLed, LOW);
    //for(int i=0; i < 1000; i++){
    while(true){
      //if sensor has issue, flash onLed
      digitalWrite(onLed, LOW); 
      delay(1000);      
      digitalWrite(onLed, HIGH);
      delay(1000);  
    }
  }
  else{
    //output info to serial
    Serial.print("watering started at: ");
    Serial.print(millis());
    digitalWrite(workLed, HIGH); //light on working led

    //open relay, delay and close
    digitalWrite(relayPin, HIGH);
    delay(WATER_INTERVAL);
    digitalWrite(relayPin, LOW);  //COMMENTED ONLY WHEN THERE IS NO RELAY ATTACHED

    //output info to serial
    digitalWrite(workLed, LOW);     //output info to serial
    Serial.print(", it lasted: ");
    Serial.print(WATER_INTERVAL);
    Serial.print(" seconds");
    Serial.println("");
    
    wateredLast = true;
  }
}

/* arduino methods */
/**
 * @brief Initializes the Arduino board and sets up the initial state.
 *
 * This function configures the pins for the LEDs, power, relay, and probe as outputs.
 * It also turns on the power LED and initializes serial communication for debugging.
 */
void setup()                    // run once, when the sketch starts
{
  // //set all led pins, power & relay pins
  // //as outputs
  // pinMode(onLed,      OUTPUT);  
  // pinMode(readLed,    OUTPUT);  
  // pinMode(workLed,    OUTPUT);  
  // pinMode(dryLed,     OUTPUT);  
  // pinMode(moistLed,   OUTPUT);  
  // pinMode(soakedLed,  OUTPUT);  
  // pinMode(powerPin,   OUTPUT);
  // pinMode(relayPin,   OUTPUT);  

  // Set all pins as outputs. Original way commented out above.
  for (int pin : {onLed, readLed, workLed, dryLed, moistLed, soakedLed, powerPin, relayPin}) {
    pinMode(pin, OUTPUT);
  }

  digitalWrite(onLed, HIGH);    // turns on power light
  Serial.begin(9600);           // open serial communications at 9600 bps  
  Serial.println("water 1.0 up and running...");
  Serial.println("");
}

/**
 * @brief Main loop for the Arduino program.
 *
 * This function repeatedly checks the moisture levels at a specified interval
 * and performs actions based on the readings. It includes a delay between checks
 * to control the frequency of moisture monitoring.
 */
void loop()                     // run over and over again
{
    //make a moisture check
    checkMoisture();

    //output info to serial
    Serial.print("delaying for ");
    Serial.print(MOIST_CHECK_INTERVAL/1000);
    Serial.println(" seconds");
    Serial.println("");
    
    delay(MOIST_CHECK_INTERVAL);
}
