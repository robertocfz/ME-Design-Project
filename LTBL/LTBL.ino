// SAE GROUP SUMMER 2017
// TEAM MEMBERS: REX BAXTER, ROBERTO SALINAS, DARREN TAN, ZACHARY ZHOU
// COMPILED WITH ARDUINO IDE 1.8.2
// EEPROM Library ver 2.0
// PinChangeInterrupt Library ver 1.2.4: https://github.com/NicoHood/PinChangeInterrupt 
// HARDWARE: ATtiny84a, STP16CPC26 16-bit LED Driver, Arduino Uno (as ISP)


//LIBRARIES
#include "PinChangeInterrupt.h"
#include <EEPROM.h>


//PINOUTS
const int DATA = 10;        //PHYSICAL PIN 2
const int LATCH = 9;        //PHYSICAL PIN 3
const int CLK = 8;          //PHYSICAL PIN 5
const int R_TURN_PIN = 2;   //PHYSICAL PIN 11
const int BRAKE_PIN = 1;    //PHYSICAL PIN 12
const int L_TURN_PIN = 0;   //PHYSICAL PIN 13
const int TEST_PIN = 7;     //PHYSICAL PIN 6  USED FOR LOOP TIMING



//PIN STATES
int L_TURN = 0;
int R_TURN = 0;
int BRAKE = 0;
volatile int L_LEDS = 0;
volatile int R_LEDS = 0;



//INITIALIZE TIMING VARIABLES
unsigned int blinkPeriod = 525;         //MILLISECONDS. ACTIVE BLINKING TIME (HIGH)
unsigned int blinkDelay = 75;           //MILLISECONDS. DELAY BETWEEN SEGMENT BLINKS
unsigned long calibrationStopWatch = 0; //MILLISECONDS. USED FOR CALIBRATION TIMING
unsigned long caliTimeout = 10000;      //MILLISECONDS. CALIBRATION MODE TIMEOUT
unsigned long currentMillis = 0;        //MILLISECONDS. FOR EMERGENCY FLASHERS
unsigned long previousMillis = 0;       //MILLISECONDS. FOR EMERGENCY FLASHERS
unsigned long newTime = 0;              //MILLISECONDS. PULSEIN TIMING



//INITIALIZE BOOLEANS
volatile bool brakeflag = false;        //TELLS IF BRAKE IS ON. VOLATILE B/C STATE CHANGES IN BRAKE INTERRUPT
bool isFourWire = true;                 //DEFAULT 4 WIRE STANDARD IN US
bool caliWiringSuccess = false;         //CALIBRATION FLAG FOR WIRING
bool caliTimingSuccess = false;         //CALIBRATION FLAG FOR SIGNAL TIMING
bool hazardFlag = false;                //EMERGENCY FLASHERS STATE



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

  //INITIALIZE PINS
  pinMode(DATA, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(R_TURN_PIN, INPUT_PULLUP);
  pinMode(BRAKE_PIN, INPUT_PULLUP);
  pinMode(L_TURN_PIN, INPUT_PULLUP);
  pinMode(TEST_PIN, OUTPUT);

  if (digitalRead(BRAKE_PIN) == HIGH) {
    brakeflag = true;
  }

  //INTIALIZE BRAKE INTERRUPT
  attachPCINT(digitalPinToPCINT(BRAKE_PIN), brakeON, CHANGE);   

  //READ EEPROM
  //EEPROM.write(0,isFourWire);                     //WRITE TO EEPROM FOR FIRST TIME SETUP
  //EEPROM.put(1, blinkPeriod);
  isFourWire = EEPROM.read(0);                      //READING EEPROM FOR WIRE CALIBRATION DATA
  blinkPeriod = EEPROM.get(1, blinkPeriod);         //READING EEPROM FOR TIMING CALIBRATION DATA. GET INSTEAD OF READ FOR MULTIPLE BYTES
  blinkDelay = blinkPeriod / 7;                     //USED FOR TURN SIGNAL SEQUENCING

/*
  //RESET BLINK
  //THIS IS JUST ACKNOWLEDGEMENT THAT THE CONTROLLER HAS RESET
  if (isFourWire = true) {
    blinkLEDS(leftPerBrake, rightPerBrake, 4, 200);   //BLINK ALL LEDS 3 TIMES WITH 200MS DELAY
    delay(500);
  }
  else {
    blinkLEDS(leftPerBrake, rightPerBrake, 5, 200);   //BLINK ALL LEDS 3 TIMES WITH 200MS DELAY
    delay(500);
  }
  */
  
}

void updateShift(uint16_t left, uint16_t right) {
  //WRITES TO BOTH LED DRIVERS. USE DECIMAL FORMAT TO ADDRESS SPECIFIC BITS
  //OUTPUTS TWO, 16 BIT STRINGS TO SHIFT REGISTERS
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



void blinkLEDS(int left, int right, int num, int interval) {
  for (int i = 0; i < num; i++) {
    updateShift(left, right);
    delay(interval);
    updateShift(0, 0);
    delay(interval);
  }
  updateShift(left,right);
}



void calibrateWiring() {
  //DETECT 4 OR 5 WIRE CONFIGURATION
  //DO THIS BY CHECKING WHEN THE BRAKE PEDAL IS DEPRESSED
  //IF BOTH TURN SIGNALS ARE ON, IT IS 4 WIRE. OTHERWISE IT IS 5

  calibrationStopWatch = millis();		  //RESET TIMER FOR CALIBRATION MODE
  while (((millis() - calibrationStopWatch) < caliTimeout) && BRAKE == HIGH && caliWiringSuccess == false) {      //TIMEOUT AFTER 10 SECONDS

    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {    //4 Wire
      if (isFourWire != true) {       	//IF THE CURRENT CALIBRATION IS NOT 4 WIRE, CHANGE TO 4 WIRE
        EEPROM.write(0, true);          //WRITE TO 0TH MEMORY BLOCK THAT ISFOURWIRE = TRUE
        isFourWire = true;
        caliWiringSuccess = true;       //BLINK CENTER LEDS 4 TIMES WITH 100MS DELAY
        blinkLEDS(leftBrake, rightBrake, 4, 100);
        delay(1000);
        break;
      }
      else {
        caliWiringSuccess = true;		    //CURRENT CALIBRATION IS ALREADY FOUR WIRE, GOOD CHECK!
        blinkLEDS(leftBrake, rightBrake, 4, 100);
        delay(1000);                    //BLINK CENTER LEDS 4 TIMES WITH 100MS DELAY
        break;
      }
    }
    if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {    //5 Wire
      if (isFourWire != false) {        //IF THE CURRENT CALIBRATION IS NOT 5 WIRE, CHANGE TO 5 WIRE
        EEPROM.write(0, false);         //WRITE TO 0TH MEMORY BLOCK THAT ISFOURWIRE = FALSE
        isFourWire = false;
        caliWiringSuccess = true;       //BLINK CENTER LEDS 5 TIMES WITH 100MS DELAY
        blinkLEDS(leftBrake, rightBrake, 5, 100);
        delay(1000);
        break;
      }
      else {
        caliWiringSuccess = true;		    //CURRENT CALIBRATION IS ALREADY FIVE WIRE, GOOD CHECK!
        blinkLEDS(leftBrake, rightBrake, 5, 100);
        delay(1000);                    //BLINK CENTER LEDS 5 TIMES WITH 100MS DELAY
        break;

      }
    }

    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
  }
}


void calibrateTiming() {
  //CALIBRATE TURN SIGNAL TIMING
  //TIMEOUT AFTER 10 SECONDS 

  calibrationStopWatch = millis();
  blinkLEDS(leftHorn, rightHorn, 1, 250);       //BLINK HORNS 2 TIMES WITH A 250MS DELAY

  while ((millis() - calibrationStopWatch) < caliTimeout && BRAKE == HIGH && caliTimingSuccess == false) {

    if (isFourWire == true) {

      if (L_TURN == LOW && caliTimingSuccess == false) {
        updateShift(leftHorn, 0);
        while (digitalRead(L_TURN_PIN) == LOW) {
        }
        newTime = pulseIn(L_TURN_PIN, LOW , 3000000) / 1000;
        if (newTime != 0 && newTime < 1500 && newTime > 25) {
          caliTimingSuccess = true;
          updateShift(leftHorn, rightHorn);
          delay(250);
        }

      }

      if (R_TURN == LOW && caliTimingSuccess == false) {
        updateShift(0, rightHorn);
        while (digitalRead(R_TURN_PIN) == LOW) {
        }
        newTime = pulseIn(R_TURN_PIN, LOW , 3000000) / 1000;
        if (newTime != 0 && newTime < 1500 && newTime > 25) {
          caliTimingSuccess = true;
          updateShift(leftHorn, rightHorn);
          delay(250);
        }
      }
    }

    else if (isFourWire == false) {

      if (L_TURN == HIGH && caliTimingSuccess == false) {
        updateShift(leftHorn, 0);
        newTime = pulseIn(L_TURN_PIN, LOW , 3000000) / 1000;
        if (newTime != 0 && newTime < 1500 && newTime > 25) {
          caliTimingSuccess = true;
          updateShift(leftHorn, rightHorn);
          delay(250);
        }

      }

      if (R_TURN == HIGH && caliTimingSuccess == false) {
        updateShift(0, rightHorn);
        newTime = pulseIn(R_TURN_PIN, LOW , 3000000) / 1000;
        if (newTime != 0 && newTime < 1500 && newTime > 25) {
          caliTimingSuccess = true;
          updateShift(leftHorn, rightHorn);
          delay(250);
        }
      }
    }


    //IF STORED VALUE IS MORE THAN 25 MILLISECONDS DIFFERENCE
    if (((newTime - blinkPeriod) > 25 || (blinkPeriod - newTime) > 25) && caliTimingSuccess == true) {
      blinkPeriod = (unsigned int) newTime;
      EEPROM.put(1, blinkPeriod);                    //STORE NEW TIMING IN 1ST ADDRESS
      blinkDelay = blinkPeriod / 7;                  //NEW DELAY FOR HORN RUN SEQUENCE
      delay(250);
    }

    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
  }
}


void runLeft() {
  //BLINKS THE LEFT SIDE OF HORNS WITH A DELAY
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
  delay(blinkDelay);
}

void runRight() {
  //BLINKS THE RIGHT SIDE OF HORNS WITH A DELAY
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
  delay(blinkDelay);
}



void emergencyFlashers() {
  while (L_TURN == R_TURN) {
    currentMillis = millis();

    if (hazardFlag == false && (currentMillis - previousMillis >= blinkPeriod)) {
      updateShift(L_LEDS, R_LEDS);
      hazardFlag = true;
      previousMillis = millis();
    }

    else if (hazardFlag == true && (currentMillis - previousMillis >= blinkPeriod)) {
      //updateShift(L_LEDS - leftHorn, R_LEDS - rightHorn);
      updateShift(L_LEDS - leftPer, R_LEDS - rightPer);

      hazardFlag = false;
      previousMillis = millis();
    }

    if (BRAKE == LOW && L_TURN == LOW && R_TURN == LOW) {
      break;
    }

    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

  }
}



void brakeON() {
  //INTERRUPT
  //USES INTERRUPT ON BRAKE_PIN TO SWITCH BRAKE SIGNAL
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




//MAIN LOOP. RUNS INFINIETLY
void loop() {
  
  //used for loop timing
  //digitalWrite(TEST_PIN,HIGH);
  //delay(10);
  //digitalWrite(TEST_PIN,LOW);     


  //BASE READING
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);


  //DEFAULT LIGHTING IS PERIPHERY LIGHTS
  if (BRAKE == HIGH) {                        //THIS CASE SHOULD PREVENT BLINKING WHEN BRAKE IS HELD DOWN
    L_LEDS = leftPerBrake;                    //L_LEDS WILL BE A CONTAINER THAT WE CAN ADD TO TO LIGHT UP DIFFERENT SECTIONS
    R_LEDS = rightPerBrake;                   //R_LEDS WILL BE A CONTAINER THAT WE CAN ADD TO TO LIGHT UP DIFFERENT SECTIONS
  }

  else {
    L_LEDS = leftPer;                         //L_LEDS WILL BE A CONTAINER THAT WE CAN ADD TO TO LIGHT UP DIFFERENT SECTIONS
    R_LEDS = rightPer;                        //R_LEDS WILL BE A CONTAINER THAT WE CAN ADD TO TO LIGHT UP DIFFERENT SECTIONS
  }


  
  //ENTER CALIBRATION PROCEDURE
  if (millis() < 2500 && BRAKE == HIGH) {     //BRAKE MUST BE HELD DOWN WHEN KEY ON 1 SEC LENIENCY
    calibrationStopWatch = millis();          //GET CURRENT TIME PROCESSOR HAS BEEN ON
    updateShift(2, 256);
    delay(1000);

    //CALIBRATION LOOP
    //EXIT CALIBRATION IF IT TAKES MORE THAN 10s OR IF BRAKE IS RELEASED
    while ((millis() - calibrationStopWatch) < caliTimeout && BRAKE == HIGH) {    

      //WIRING CALIRATION
      if (caliWiringSuccess == false) {
        calibrateWiring();
      }

      //TIMING CALIBRATION
      if (caliTimingSuccess == false) {
        calibrateTiming();
      }

      //ACKNOWLEDGEMENT THAT CALIBRATION IS FULLY SUCCESSFUL
      if (caliWiringSuccess == true && caliTimingSuccess == true) {
        updateShift(leftPerBrake, 0);
        delay(250);
        updateShift(0, rightPerBrake);
        delay(250);
        updateShift(leftPerBrake, rightPerBrake);
        delay(250);
        updateShift(0, 0);
        delay(250);
        updateShift(leftPer, rightPer);
        break;			                          //ALL CALIBRATED AND READY TO ROLL
      }
    }
  }



  //FOUR WIRE SECTION
  if (isFourWire == true) {

    if (BRAKE == HIGH) {
      
      if (L_TURN == LOW && R_TURN == LOW) {     //USED TO CATCH DELAYED FLASHERS FROM BRAKE SIG
        //UNCOMMENT FOR EMERGENCY FLASHERS
        //THIS IS COMMENTED OUT BECAUSE OF SIGNAL DELAY IN 4-WIRE CARS
        //SOMETIMES THE BRAKE SIGNALS ARRIVE BEFORE/AFTER TURN SIGNALS WHICH CAUSE LOGIC ISSUES
        //hazardFlag = true;
        //emergencyFlashers();
      }
      
      if (L_TURN == LOW && R_TURN == HIGH) {    //BRAKE + LEFT SIGNAL
        runLeft();
      }

      if (L_TURN == HIGH && R_TURN == LOW) {    //BRAKE + RIGHT SIGNAL
        runRight();
      }
    }

    else {

      if (L_TURN == HIGH && R_TURN == HIGH) {   //USED TO CATCH DELAYED BRAKE SIGNAL
        //UNCOMMENT FOR EMERGENCY FLASHERS
        //THIS IS COMMENTED OUT BECAUSE OF SIGNAL DELAY IN 4-WIRE CARS
        //SOMETIMES THE BRAKE SIGNALS ARRIVE BEFORE/AFTER TURN SIGNALS WHICH CAUSE LOGIC ISSUES
        //hazardFlag = false;
        //emergencyFlashers();

        L_LEDS = leftPerBrake;                  //DISPLAY BRAKING
        R_LEDS = rightPerBrake;
      }

      if (L_TURN == HIGH && R_TURN == LOW) {    //LEFT SIGNAL
        runLeft();
      }

      if (L_TURN == LOW && R_TURN == HIGH) {    //RIGHT SIGNAL
        runRight();
      }
    }

  }



  //FIVE WIRE SECTION
  else {

    if (L_TURN == HIGH && R_TURN == HIGH) {     //EMERGENCY FLASHERS
      emergencyFlashers();
    }

    if (L_TURN == HIGH) {                       //LEFT SIGNAL
      runLeft();
    }

    if (R_TURN == HIGH) {                       //RIGHT SIGNAL
      runRight();
    }
  }

  updateShift(L_LEDS, R_LEDS);                  //OUTPUT PERIPHERY OR BRAKE LIGHTS
  delay(20);
}











