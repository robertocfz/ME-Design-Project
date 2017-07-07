// SAE GROUP SUMMER 2017
// Team Members: Rex Baxter, Roberto Salinas, Darren Tan, Zachary Zhou

//LIBRARIES
#include "Arduino.h"
#include "PinChangeInterrupt.h"
#include <EEPROM.h>

//PINOUTS
#define DATA  10        //Physical Pin 2 
#define LATCH  9        //Physical Pin 3
#define CLK  8          //Physical Pin 5
#define R_TURN_PIN 2    //Physical Pin 11
#define BRAKE_PIN  1    //Physical Pin 12
#define L_TURN_PIN  0   //Physical Pin 13


//PIN STATES
int R_TURN = 0;
int L_TURN = 0;
volatile int BRAKE = 0;
volatile int L_LEDS = 0;
volatile int R_LEDS = 0;


//INITIALIZE TIMING VARIABLES
unsigned long savedMillis = 0;    //milliseconds
unsigned long blinkPeriod = 525;    //milliseconds. Blinking Period
unsigned long caliTime = 10000;     //milliseconds  Calibration mode timeout
int blinkDelay = 75;                //milliseconds. Delay between segment blinks



//INITIALIZE OTHER VARIABLES
int brakeflag = 0;
int isFourWire = 1;     
int hazardflag = 0;
int caliWiringSuccess = 0;
int caliTimingSuccess = 0;



//LIGHTING STATES
const int leftHorn = 126;
const int rightHorn = 504;
const int leftPer = 14590;
const int rightPer = 5116;
const int leftBrake = 1792;
const int rightBrake = 3074;


void setup() {

  //INITIALIZE PINS
  pinMode(DATA,OUTPUT);
  pinMode(LATCH,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(R_TURN_PIN,INPUT);
  pinMode(BRAKE_PIN,INPUT);
  pinMode(L_TURN_PIN,INPUT);

  //INTIALIZE BRAKE INTERRUPT
  attachPCINT(digitalPinToPCINT(BRAKE_PIN), brakeON, CHANGE);

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


void calibrateWiring() {
  //detect 4 or 5 wire configuration
  //do this by checking when the brake pedal is depressed 
  //if both turn signals are on, it is 4 wire. otherwise it is 5
  int isFourWire = EEPROM.read(0);    //0th block of memory isFourWire

  savedMillis = millis();
  while ((millis() - savedMillis) < caliTime && BRAKE == HIGH) {                  //Timeout after 10 seconds
    //READ PINS
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {    //4 Wire 
      if (isFourWire != 1) {      //If the current calibration is NOT 4 wire, change to 4 wire
        EEPROM.write(0,1);        //Write to 0th memory block that isFourWire = True
        updateShift(4,128);
        caliWiringSuccess = 1;
        break;
      }
      
    }
    if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {    //5 Wire 
      if (isFourWire != 0) {      //If the current calibration is NOT 5 wire, change to 5 wire
        EEPROM.write(0,0);        //Write to 0th memory block that isFourWire = False
        updateShift(4,128);
        caliWiringSuccess = 1;
        break;
      }
    }
  }
}


void calibrateTiming(int PIN) {
  //Calibrate turn signal timing 
  //Timeout after 10 seconds (see pulseIn line)
  int storedTiming = EEPROM.read(1);    //1st block of memory is for blinker timing in milliseconds

  savedMillis = millis();
  while ((millis() - savedMillis) < caliTimeout && BRAKE == HIGH && caliTimingSuccess == 0) {
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (isFourWire == 1) {
      if (L_TURN == HIGH) {
        //Times signal pulse in microseconds, divide to get milliseconds. 
        blinkPeriod = (int) (pulseIn(PIN, HIGH,1000000000))/1000; //Timeout of 10 seconds
        
        if (blinkPeriod != 0) {
          caliTimingSuccess = 1;
        }
      }
    }

    if (isFourWire == 0) {
      if (R_TURN == HIGH) {
        //Times signal pulse in microseconds, divide to get milliseconds. 
        blinkPeriod = (int) (pulseIn(PIN, HIGH,1000000000))/1000; //Timeout of 10 seconds
       
        if (blinkPeriod != 0) {
          caliTimingSuccess = 1;
        }
      }
    }

  //If stored value is more than 25 milliseconds difference
  if (abs(blinkPeriod - storedTiming) > 25 && caliTimingSuccess == 1) {   
    EEPROM.write(1,blinkPeriod);                //Store new timing in 1st address
    blinkDelay = blinkPeriod/7;                   //Delay for horn run sequence
    updateShift(8,64);
  }
}


void runLeft() {
  //Blinks the left side of horns with a delay 
  //126 is value of all left horn LEDs
  updateShift(L_LEDS - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 64 -leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 96 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 112 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 120 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 124 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS,R_LEDS);
  delay(blinkDelay);
}

void runRight() {
  //Blinks the right side of horns with a delay 
  //504 is value of all right horn LEDs
  updateShift(L_LEDS, R_LEDS - rightHorn); 
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 8 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 24 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 56 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 120 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 248 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS);
  delay(blinkDelay);
}

void hazardLights() { 

  while (L_TURN == R_TURN) {
    //BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (L_TURN == HIGH && R_TURN == HIGH) {
      updateShift(leftPer + leftBrake, rightPer + rightBrake);
      delay(blinkPeriod);
      updateShift(0,0);
      delay(blinkPeriod);
    }
  }
}

void brakeON() {
  //Uses interrupt on BRAKE_PIN to switch brake signal
  if (brakeflag == 0) {
    
    L_LEDS = L_LEDS + leftBrake;
    R_LEDS = R_LEDS + rightBrake;
    brakeflag = 1;
  }

  else{
    
    L_LEDS = L_LEDS - leftBrake;
    R_LEDS = R_LEDS - rightBrake;   
    brakeflag = 0;
  }
}



//MAIN LOOP. RUNS INFINIETLY
void loop() {

  //READ PINS
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);
  

  //ENTER CALIBRATION 
  if (millis() < 500 && BRAKE == HIGH) {        //Brake must be held down when key on 500 ms leniency 
    savedMillis = millis();                   //Get current time processor has been on
    updateShift(2,256);

    //CALIBRATION LOOP
    while ((millis() - savedMillis) < caliTime && BRAKE == HIGH) {    //While less than 10s AND brake is pressed
      
      //WIRING CALIRATION
      if (caliWiringSuccess == 0){
        calibrateWiring();
      }

      //TIMING CALIBRATION
      if (caliTimingSuccess == 0) {
        calibrateTiming();
      }

      BRAKE = digitalRead(BRAKE_PIN);

      if (caliWiringSuccess == 1 && caliTimingSuccess == 1) {
        break;
      }
    }
  }


  //DEFAULT LIGHTING IS PERIPHERY
  if (brakeflag == 1) {
    L_LEDS = leftPer + leftBrake;               
    R_LEDS = rightPer + rightBrake;

  }
  else {
    L_LEDS = leftPer;
    R_LEDS = rightPer;
  }



  //FOUR WIRE SECTION
  if (isFourWire == 1) {

    if (L_TURN == HIGH && R_TURN == HIGH && BRAKE == LOW) {   //HAZARD LIGHTS
      hazardLights();
    }

    if (L_TURN == HIGH && BRAKE == LOW) {   //THIS IS FLAWED! What if you brake first then signal?! 
      runLeft();
    }

    if (R_TURN == HIGH && BRAKE == LOW) {
      runRight();
    }
  }



  //FIVE WIRE SECTION
  else {

    if (L_TURN == HIGH && R_TURN == HIGH) {   //HAZARD LIGHTS
      hazardLights();
    }

    if (L_TURN == HIGH) {
      runLeft();
    }
    
    if (R_TURN == HIGH) {
      runRight();
    }
  }


  updateShift(L_LEDS,R_LEDS);   //OUTPUT PERIPHERY OR BRAKE LIGHTS
  }
  








 

