// Must include if using EEPROM commands
#include <EEPROM.h>

// Variables with values of interest
// Note: EEPROM for the ATtiny84 is limited to 512 numeric bytes (1 numeric byte = 8 total bits = any one number between 0-255)
bool calibrated; // "calibrated" will be 1 of 2 variables in EEPROM storage. Cast as bool as the bool datatype is 1 numeric byte (True = 1, False = 0, nothing more)
unsigned int time; // Captures a specific point in time after the Arduino has been powered in unsigned integer form
unsigned int blinkerTime; // "blinkterTime" will be 1 of 2 variables in EEPROM storage, taking on the value of time at a specific point in time. Should be an unsigned int (same as time)
int eepromValue; // Variable for printing the stored EEPROM values in integer form
int randomVar = 256; // Random integer variable to show how EEPROM.get() works. Can be any data type (int, float, double, etc.)
int a = 0; // Generic counter

void setup() {
  Serial.begin(9600);
}

void loop() {
  time = (unsigned int) millis(); // in milliseconds, forces the time (millis() returns an unsigned long int) to be cast as an unsigned integer (values ranging from 0-65,535, no negative numbers) so that only 16-bits, or 2 bytes, of EEPROM memory is taken and is more predictable
  // Note: 65,535 milliseconds is 65.535 seconds, or a little over a minute. Most car blinkers will not exceed this, so using an unsigned int is safe
  
//  // For debugging
//  Serial.println(time);
  
  if (time == 2511 && a == 0) // Executes the following if time = 2.511 seconds (arbitrarily chosen) to show EEPROM commands, variable "a" is explained later
  {
    // Switch calibrated to True (i.e. a value of 1) and write to EEPROM (since bool (T=1, F=0) takes only taakes 1 byte of memory, .write() can be utilized)
    calibrated = true;
    EEPROM.write(0,calibrated);
    
    // Capture the time and saves to EEPROM via put() (NOT write(), since write() can essentially only write numbers between 0-255, or 1 byte, to EEPROM)
    // **Note: since blinkerTime is 2 bytes (unsigned int), then address position 1 and the next position (2) will both be written to in EEPROM and .put() must be used
    blinkerTime = time;
    EEPROM.put(1,blinkerTime);

    // Write a random value to EEPROM that takes up more than 1 byte (thus the use of put()) on address 3 + however many more addresses are needed to store the number (e.g. if the random variable number was a float (takes 4 bytes of memory), than addresses 3, 4, 5, and 6 would be written to)
    EEPROM.put(3,randomVar);

    // Retreive and print blinkerTime, calibrated, and randomVar stored in EEPROM
    // Note: since calibrated is 1 byte and is stored in address 0, than .read() can be used, otherwise use .get()
    Serial.print("calibrated = "); 
    eepromValue = EEPROM.read(0);
    Serial.println(eepromValue);
    
    Serial.print("blinkerTime = "); 
    eepromValue = EEPROM.get(1,blinkerTime);
    Serial.println(eepromValue);

    Serial.print("randomVar = "); 
    eepromValue = EEPROM.get(3,randomVar);
    Serial.println(eepromValue);
    
    // Prevent the loop from going so fast it counts a time of 2511 miliseconds more than once or any other glitchy business
    a = a+1;

//    // Start loop to display stored EEPROM values (limited to 512 since the ATtiny84 only has 512 bytes of EEPROM memory)
//    while (a <= 5)
//    {
//      // Read each EEPROM value and display to serial monitor
//      eepromValue = EEPROM.read(a);
//      Serial.println(eepromValue);
//      a = a+1;
//    }

  }
}
