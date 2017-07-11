// SAE GROUP SUMMER 2017
// Team Members: Rex Baxter, Roberto Salinas, Darren Tan, Zachary Zhou

//LIBRARIES
#include "Arduino.h"
#include <EEPROM.h>

//PINOUTS
#define DATA  10        //Physical Pin 2 
#define LATCH  9        //Physical Pin 3
#define CLK  8          //Physical Pin 5

//LIGHTING STATES
const int leftHorn = 126; // EEPROM.write()
const int rightHorn = 504; // EEPROM.put()

// Value of EEPROM in memory storage
int eepromValue; 

void setup() {

  //INITIALIZE PINS
  pinMode(DATA,OUTPUT);
  pinMode(LATCH,OUTPUT);
  pinMode(CLK,OUTPUT);

  //RESET BLINK
  updateShift(0,0);
  delay(250);
  updateShift(16383,8191);
  delay(250);
  updateShift(0,0);
  delay(250);
  updateShift(16383,8191);
  delay(250);
  updateShift(0,0);
  delay(500);

  // Test EEPROM storage capabilities

  // Left horn
  EEPROM.write(0,leftHorn);
  eepromValue = EEPROM.read(0); 
  updateShift(eepromValue,0);

  delay(1000);
  
  // Right horn
  EEPROM.put(1,rightHorn);
  eepromValue = EEPROM.get(1,rightHorn);
  updateShift(0,eepromValue);

  delay(1000);
  updateShift(0,0);
}

void updateShift(uint16_t left, uint16_t right){
  //Writes to both LED drivers. Use decimal format to address specific bits

  digitalWrite(LATCH,LOW);
  
  for(int i=0;i<16;i++){
    digitalWrite(DATA, !!(right & (1 << (16 - i))));
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);       
    }

  for(int i=0;i<16;i++){
    digitalWrite(DATA, !!(left & (1 << (16 - i))));
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);       
    }
    
   digitalWrite(LATCH,HIGH);
}


void loop() {
}
