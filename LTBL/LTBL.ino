// SAE GROUP SUMMER 2017
// Team Members: Rex Baxter, Roberto Salinas, Darren Tan, Zachary Zhou

//LIBRARIES
#include "Arduino.h"
#include "PinChangeInterrupt.h"
#include <EEPROM.h>


//PINOUTS
const int DATA = 10;        //Physical Pin 2
const int LATCH = 9;        //Physical Pin 3
const int CLK = 8;          //Physical Pin 5
const int R_TURN_PIN = 2;   //Physical Pin 11
const int BRAKE_PIN = 1;    //Physical Pin 12
const int L_TURN_PIN = 0;   //Physical Pin 13

//PIN STATES
int R_TURN = 0;
int L_TURN = 0;
int BRAKE = 0;
volatile int L_LEDS = 0;
volatile int R_LEDS = 0;



//INITIALIZE TIMING VARIABLES
unsigned long calibrationStopWatch = 0; //milliseconds. Used for calibration timing
unsigned long currentTime = 0;
unsigned int blinkPeriod = 7000;                  //milliseconds. Blinking Period
unsigned long caliTimeout = 10000;      //milliseconds. Calibration mode timeout
unsigned long leftSigTime = 0;          //milliseconds. Used to determine if signal is blinking
unsigned long rightSigTime = 0;         //milliseconds. Used to determine if signal is blinking
unsigned long brakeTime = 0;            //milliseconds. Used to time how long the brakes have been on
unsigned int blinkDelay = 100;                   //milliseconds. Delay between segment blinks
unsigned int leniency = 25;					          	//milliseconds. Leniency in blinkPeriod length for logical checks



//INITIALIZE BOOLEANS
bool isFourWire = true;                 //Default 4 wire standard in US
bool brakeflag = false;                 //Tells if brake is on (REPLACE WITH BRAKE VAR)
bool caliWiringSuccess = false;         //Calibration Flag for Wiring
bool caliTimingSuccess = false;         //Calibration Flag for Signal Timing
bool leftSignalFlag = false;            //Flag that the left signal is on
bool rightSignalFlag = false;           //Flag that the right signal is on
bool BLINKING = false;                  //Flag that shows if the signal is blinking



//LIGHTING STATES
const int leftHorn = 126;
const int leftPer = 14590;
const int leftBrake = 1792;
const int leftPerBrake = 16382;
const int L1 = 64;
const int L2 = 96;
const int L3 = 112;
const int L4 = 120;
const int L5 = 124;
const int rightHorn = 504;
const int rightPer = 5116;
const int rightBrake = 3074;
const int rightPerBrake = 8190;
const int R1 = 8;
const int R2 = 24;
const int R3 = 56;
const int R4 = 120;
const int R5 = 248;



void setup() {
  //isFourWire = false; //testing 5 wire

  //INITIALIZE PINS
  pinMode(DATA, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(R_TURN_PIN, INPUT);
  pinMode(BRAKE_PIN, INPUT_PULLUP);
  pinMode(L_TURN_PIN, INPUT);

  if (digitalRead(BRAKE_PIN) == HIGH) {
    brakeflag = true;
  }

  //INTIALIZE BRAKE INTERRUPT
  attachPCINT(digitalPinToPCINT(BRAKE_PIN), brakeON, CHANGE);               //Attaching interrupt for BRAKE_PIN. Calls brakeON() function on CHANGE
  //  attachPCINT(digitalPinToPCINT(L_TURN_PIN),leftSignalOn, RISING);				//Attaching interrupt for L_TURN_PIN. Calls leftSignalOn on RISING EDGE
  //  attachPCINT(digitalPinToPCINT(R_TURN_PIN), rightSignalOn, RISING);			//Attaching interrupt for R_TURN_PIN. Calls rightSignalOn on RISING EDGE


  //READ EEPROM
  //blinkPeriod = 500 * 7;
  //EEPROM.put(1, blinkPeriod);
  isFourWire = EEPROM.read(0);           //Reading EEPROM for wire calibration data
  blinkPeriod = EEPROM.get(1, blinkPeriod);         //Reading EEPROM for timing calibration data
  blinkDelay = blinkPeriod / 7;


  //RESET BLINK
  //This is just acknowledgement that the controller has reset
  updateShift(0, 0);
  delay(250);
  updateShift(leftPerBrake, rightPerBrake);
  delay(250);
  updateShift(0, 0);
  delay(250);
  updateShift(leftPerBrake, rightPerBrake);
  delay(250);
  updateShift(0, 0);
  delay(500);

}

void updateShift(uint16_t left, uint16_t right) {
  //Writes to both LED drivers. Use decimal format to address specific bits

  digitalWrite(LATCH, LOW);

  for (int i = 0; i < 16; i++) {
    digitalWrite(DATA, !!(right & (1 << (16 - i))));
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);
  }

  for (int i = 0; i < 16; i++) {
    digitalWrite(DATA, !!(left & (1 << (16 - i))));
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);
  }

  digitalWrite(LATCH, HIGH);
}


void calibrateWiring() {
  //detect 4 or 5 wire configuration
  //do this by checking when the brake pedal is depressed
  //if both turn signals are on, it is 4 wire. otherwise it is 5

  calibrationStopWatch = millis();				  //Reset timer for calibration mode
  while (((millis() - calibrationStopWatch) < caliTimeout) && BRAKE == HIGH && caliWiringSuccess == false) {      //Timeout after 10 seconds
    //READ PINS


    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {    //4 Wire
      if (isFourWire != true) {       	//If the current calibration is NOT 4 wire, change to 4 wire
        EEPROM.write(0, true);          //Write to 0th memory block that isFourWire = True
        isFourWire = true;
        updateShift(leftHorn, rightHorn);
        caliWiringSuccess = true;
        break;
      }
      else {
        caliWiringSuccess = true;		    //Current calibration is already Four Wire, good check!
        updateShift(4, 128);
        break;
      }
    }
    if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {    //5 Wire
      if (isFourWire != false) {        //If the current calibration is NOT 5 wire, change to 5 wire
        EEPROM.write(0, false);         //Write to 0th memory block that isFourWire = False
        isFourWire = false;
        updateShift(leftHorn, rightHorn);
        caliWiringSuccess = true;
        break;
      }
      else {
        caliWiringSuccess = true;		    //Current calibration is already Five Wire, good check!
        updateShift(4, 128);
        break;

      }
    }
    if (caliWiringSuccess == true) {
      break;
    }
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
  }
}


void calibrateTiming() {
  //Calibrate turn signal timing
  //Timeout after 10 seconds (see pulseIn line)

  calibrationStopWatch = millis();
  int newTime = 0;

  while ((millis() - calibrationStopWatch) < caliTimeout && BRAKE == HIGH && caliTimingSuccess == false) {
    if (digitalRead(L_TURN_PIN) == HIGH && caliTimingSuccess == false) {
      newTime = pulseIn(L_TURN_PIN, LOW , 3000000) / 1000;
      if (newTime != 0 && newTime < 1500 && newTime > 25) {
        updateShift(leftBrake, rightBrake);
        caliTimingSuccess = true;
        delay(500);
      }
      else {
        //updateShift(64,64);
      }
    }

    if (digitalRead(R_TURN_PIN) == HIGH && caliTimingSuccess == false) {
      newTime = pulseIn(R_TURN_PIN,LOW , 3000000)/1000;
      if (newTime != 0 && newTime < 1500 && newTime > 25) {
        updateShift(leftBrake, rightBrake);
        caliTimingSuccess = true;
        delay(500);
      }
      else {
        //updateShift(64,64);
      }
    }

    //If stored value is more than 25 milliseconds difference
    if (((newTime - blinkPeriod) > 25 || (blinkPeriod - newTime) > 25) && caliTimingSuccess == true) {
      blinkPeriod = (unsigned int) newTime;
      EEPROM.put(1, blinkPeriod);                    //Store new timing in 1st address
      blinkDelay = blinkPeriod / 7;                //New delay for horn run sequence
      updateShift(leftHorn, rightHorn);
      delay(500);
    }

    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
  }
}


void runLeft() {
  //Blinks the left side of horns with a delay
  updateShift(L_LEDS - leftHorn, R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS - leftHorn + L1, R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS - leftHorn + L2, R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS - leftHorn + L3, R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS - leftHorn + L4, R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS - leftHorn + L5, R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS, R_LEDS);
  //delay(blinkDelay);
}

void runRight() {
  //Blinks the right side of horns with a delay
  updateShift(L_LEDS, R_LEDS - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS , R_LEDS - rightHorn + R1);
  delay(blinkDelay);
  updateShift(L_LEDS , R_LEDS - rightHorn + R2);
  delay(blinkDelay);
  updateShift(L_LEDS , R_LEDS - rightHorn + R3);
  delay(blinkDelay);
  updateShift(L_LEDS , R_LEDS - rightHorn + R4);
  delay(blinkDelay);
  updateShift(L_LEDS , R_LEDS - rightHorn + R5);
  delay(blinkDelay);
  updateShift(L_LEDS , R_LEDS);
  //delay(blinkDelay);

}

void emergencyFlashers() {

  while (L_TURN == HIGH && R_TURN == HIGH) {
    if (L_TURN == HIGH && R_TURN == HIGH) {
      updateShift(leftPerBrake, rightPerBrake);
      delay(blinkPeriod);
      updateShift(0, 0);
      delay(blinkPeriod);
    }

    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
  }
}

void brakeON() {
  //INTERRUPT
  //Uses interrupt on BRAKE_PIN to switch brake signal

  if (brakeflag == false) {

    L_LEDS = leftPerBrake;
    R_LEDS = rightPerBrake;
    brakeflag = true;
  }

  else {

    L_LEDS = leftPer;
    R_LEDS = rightPer;
    brakeflag = false;
  }
}



void leftSignalOn() {
  //INTERRUPT
  //Uses interrupt on L_TURN_PIN and R_TURN_PIN to time signal on
  //Only relevant for four wire
  leftSigTime = millis();
}

void rightSignalOn() {
  //INTERRUPT
  //Uses interrupt on L_TURN_PIN and R_TURN_PIN to time signal on
  //Only relevant for four wire
  rightSigTime = millis();
}

bool isBlinking() {
  //Used to check if the lights are blinking or solid
  //Only used in 4 Wire
  //This is where the magic logic comes into play for all 4 wire complexities

  //I think this has a flaw in the logic...please look over the conditions
  unsigned long currentTime = millis();
  unsigned long brakeStopwatch = currentTime - brakeTime;
  unsigned long leftStopwatch = currentTime - leftSigTime;
  unsigned long rightStopwatch = currentTime - rightSigTime;
  unsigned long blinkLeniency = blinkPeriod + leniency;

  if (BRAKE == LOW && L_TURN == HIGH && R_TURN == LOW) {                     //BAISC LEFT TURN
    BLINKING == true;
  }

  else if (BRAKE == LOW && L_TURN == LOW && R_TURN == HIGH) {                //BASIC RIGHT TURN
    BLINKING == true;
  }

  else if (BRAKE == LOW && L_TURN == HIGH && R_TURN == HIGH) {  			       //BASIC HAZARD LIGHTS
    BLINKING = true;
  }

  else if (BRAKE == HIGH && brakeStopwatch > blinkLeniency) {
    BLINKING = true;														 //IF BRAKE HELD LONGER THAN BLINKPERIOD
  }																        			 //IT IS BLINKING!

  else if (BRAKE == HIGH && rightStopwatch > leftStopwatch && leftStopwatch < blinkLeniency) {
    BLINKING = true;														 //BRAKING AND LEFT TURN
  }

  else if (BRAKE == HIGH && leftStopwatch > rightStopwatch && rightStopwatch < blinkLeniency) {
    BLINKING = true;														 //BRAKING AND LEFT TURN
  }

  else if (BRAKE == HIGH && leftStopwatch - rightStopwatch < leniency) {
    BLINKING = true;														 //BRAKING AND HAZARDS
  }

  else {
    BLINKING = false;
  }

  return BLINKING;
}



//MAIN LOOP. RUNS INFINIETLY
void loop() {

  //READ PINS
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);
  currentTime = millis();



  //ENTER CALIBRATION
  if (millis() < 2000 && BRAKE == HIGH) {      //Brake must be held down when key on 500 ms leniency
    calibrationStopWatch = millis();                     //Get current time processor has been on
    updateShift(2, 256);
    delay(500);

    //CALIBRATION LOOP
    while ((millis() - calibrationStopWatch) < caliTimeout && BRAKE == HIGH) {    //While less than 10s AND brake is pressed

      //WIRING CALIRATION
      if (caliWiringSuccess == false) {
        calibrateWiring();
      }

      BRAKE = digitalRead(BRAKE_PIN);

      //TIMING CALIBRATION
      if (caliTimingSuccess == false) {
        calibrateTiming();
      }

      BRAKE = digitalRead(BRAKE_PIN);

      if (caliWiringSuccess == true && caliTimingSuccess == true) {
        updateShift(leftPerBrake, 0);
        delay(250);
        updateShift(0, rightPerBrake);
        delay(250);
        updateShift(leftPerBrake, rightPerBrake);
        break;			//All calibrated and ready to roll
      }
    }
  }

  //DEFAULT LIGHTING IS PERIPHERY

  if (BRAKE == HIGH) {               //This case should prevent blinking when brake is held down
    L_LEDS = leftPerBrake;           //L_LEDS will be a container that we can add to to light up different sections
    R_LEDS = rightPerBrake;          //R_LEDS will be a container that we can add to to light up different sections
  }

  else {
    L_LEDS = leftPer;                       //L_LEDS will be a container that we can add to to light up different sections
    R_LEDS = rightPer;                      //R_LEDS will be a container that we can add to to light up different sections
  }


  //FOUR WIRE SECTION
  if (isFourWire == true) {

    if (L_TURN == HIGH && R_TURN == HIGH && isBlinking() == true) {   //EMERGENCY FLASHERS
      emergencyFlashers();
    }

    if (L_TURN == HIGH && isBlinking() == true) {                     //LEFT SIGNAL
      runLeft();
    }

    if (R_TURN == HIGH && isBlinking() == true) {                     //RIGHT SIGNAL
      runRight();
    }
  }



  //FIVE WIRE SECTION
  else {

    if (L_TURN == HIGH && R_TURN == HIGH) {   //EMERGENCY FLASHERS
      emergencyFlashers();
    }

    if (L_TURN == HIGH) {                     //LEFT SIGNAL
      runLeft();
    }

    if (R_TURN == HIGH) {                     //RIGHT SIGNAL
      runRight();
    }
  }

  updateShift(L_LEDS, R_LEDS);              //OUTPUT PERIPHERY OR BRAKE LIGHTS
}











