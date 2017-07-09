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
unsigned long savedMillis = 0;          //milliseconds
unsigned long blinkPeriod = 525;        //milliseconds. Blinking Period
unsigned long caliTimeout = 10000;      //milliseconds. Calibration mode timeout
unsigned long leftSigTime = 0;          //milliseconds. Used to determine if signal is blinking
unsigned long rightSigTime = 0;         //milliseconds. Used to determine if signal is blinking
unsigned long brakeTime = 0;            //milliseconds. Used to time how long the brakes have been on
int blinkDelay = 75;                    //milliseconds. Delay between segment blinks



//INITIALIZE BOOLEANS
bool brakeflag = false;
bool isFourWire = true;                 //Default 4 wire standard in US     
bool caliWiringSuccess = false;         //Calibration Flag for Wiring
bool caliTimingSuccess = false;         //Calibration Flag for Signal Timing
bool leftSignalFlag = false;            //Flag that the left signal is on
bool rightSignalFlag = false;           //Flag that the right signal is on    
bool BLINKING = false;                  //Flag that shows if the signal is blinking


//LIGHTING STATES
const int leftHorn = 126;
const int leftPer = 14590;
const int leftBrake = 1792;
const int L1 = 64;
const int L2 = 96;
const int L3 = 112;
const int L4 = 120;
const int L5 = 124;
const int rightHorn = 504;
const int rightPer = 5116;
const int rightBrake = 3074;
const int R1 = 8;
const int R2 = 24;
const int R3 = 56;
const int R4 = 120;
const int R5 = 248;



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
  attachPCINT(digitalPinToPCINT(L_TURN_PIN),leftSignalOn, RISING);   //change to rising for more effiency
  attachPCINT(digitalPinToPCINT(R_TURN_PIN), rightSignalOn, RISING);

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
  while ((millis() - savedMillis) < caliTimeout && BRAKE == HIGH) {      //Timeout after 10 seconds
    //READ PINS
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {    //4 Wire 
      if (isFourWire != true) {      //If the current calibration is NOT 4 wire, change to 4 wire
        EEPROM.write(0,true);           //Write to 0th memory block that isFourWire = True
        updateShift(4,128);
        caliWiringSuccess = true;
        break;
      }
      
    }
    if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {    //5 Wire 
      if (isFourWire != false) {      //If the current calibration is NOT 5 wire, change to 5 wire
        EEPROM.write(0,false);            //Write to 0th memory block that isFourWire = False
        updateShift(4,128);
        caliWiringSuccess = true;
        break;
      }
    }
  }
}


void calibrateTiming() {
  //Calibrate turn signal timing 
  //Timeout after 10 seconds (see pulseIn line)
  int storedTiming = EEPROM.read(1);    //1st block of memory is for blinker timing in milliseconds

  savedMillis = millis();
  while ((millis() - savedMillis) < caliTimeout && BRAKE == HIGH && caliTimingSuccess == false) {
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (isFourWire == true) {
      if (L_TURN == HIGH && isBlinking() == true) {   //susecptable to issues...check this later
        //Times signal pulse in microseconds, divide to get milliseconds. 
        blinkPeriod = (int) (pulseIn(L_TURN_PIN, HIGH,1000000000))/1000; //Timeout of 10 seconds **might not need cast int
      }

      if (R_TURN == HIGH && isBlinking() == true) {   //susecptable to issues...check this later
        //Times signal pulse in microseconds, divide to get milliseconds. 
        blinkPeriod = (int) (pulseIn(R_TURN_PIN, HIGH,1000000000))/1000; //Timeout of 10 seconds
      }


    }

    if (isFourWire == false) {
      if (L_TURN == HIGH) {
        //Times signal pulse in microseconds, divide to get milliseconds. 
        blinkPeriod = (int) (pulseIn(L_TURN_PIN, HIGH,1000000000))/1000; //Timeout of 10 seconds
      }
      if (R_TURN == HIGH) {
        //Times signal pulse in microseconds, divide to get milliseconds. 
        blinkPeriod = (int) (pulseIn(R_TURN_PIN, HIGH,1000000000))/1000; //Timeout of 10 seconds
      }
    }

    if (blinkPeriod != 0 && blinkPeriod < 3000) {     //If period is non-zero (error in pulseIn) and does not exceed 3s
      caliTimingSuccess = true;
    }

  //If stored value is more than 25 milliseconds difference
  if (abs(blinkPeriod - storedTiming) > 25 && caliTimingSuccess == true) {   
    EEPROM.write(1,blinkPeriod);                //Store new timing in 1st address
    blinkDelay = blinkPeriod/7;                 //Delay for horn run sequence
    updateShift(8,64);
  }
  }
}


void runLeft() {
  //Blinks the left side of horns with a delay 
  //126 is value of all left horn LEDs
  updateShift(L_LEDS - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + L1 -leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + L2 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + L3 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + L4 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + L5 - leftHorn,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS,R_LEDS);
  delay(blinkDelay);
}

void runRight() {
  //Blinks the right side of horns with a delay 
  //504 is value of all right horn LEDs
  updateShift(L_LEDS, R_LEDS - rightHorn); 
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + R1 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + R2 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + R3 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + R4 - rightHorn);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + R5 - rightHorn);
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
  //INTERRUPT
  //Uses interrupt on BRAKE_PIN to switch brake signal
  brakeTime = millis();

  if (brakeflag == false) {
    
    L_LEDS = L_LEDS + leftBrake;
    R_LEDS = R_LEDS + rightBrake;
    brakeflag = true;
  }

  else{
    
    L_LEDS = L_LEDS - leftBrake;
    R_LEDS = R_LEDS - rightBrake;   
    brakeflag = false;
  }
}


void leftSignalOn() {
  //INTERRUPT
  //Uses interrupt on L_TURN_PIN and R_TURN_PIN to time signal on
  //Maybe runLeft()? Would this conflict with hazard sequence?
  leftSigTime = millis();
  /*
  if (leftSignalFlag == false) {       //If signal was off, it's about to turn on
    leftSigTime = millis();            //Time when it started
    leftSignalFlag = true;             //The signal is now on
  }

  if (leftSignalFlag == true) {        //If signal was on, it's about to turn off
    leftSignalFlag = false;            //The signal is now off
  }
  */
}

void rightSignalOn() {
  //INTERRUPT
  //Uses interrupt on L_TURN_PIN and R_TURN_PIN to time signal on
  
  rightSigTime = millis();
  /*
  if (rightSignalFlag == false) {       //If signal was off, it's about to turn on
    rightSigTime = millis();            //Time when it started
    rightSignalFlag = true;             //The signal is now on
  }

  if (rightSignalFlag == true) {        //If signal was on, it's about to turn off
    rightSignalFlag = false;            //The signal is now off
  }
  */
}

bool isBlinking() {
  //Used to check if the lights are blinking or solid
  //Only used in 4 Wire

  //I think this has a flaw in the logic...please look over the conditions

  if (L_TURN == HIGH && R_TURN == LOW && (millis() - leftSigTime) < (blinkPeriod + 25)) {                      //LEFT TURN
    BLINKING == true;
  }

  else if (L_TURN == LOW && R_TURN == HIGH && (millis() - rightSigTime) < (blinkPeriod + 25)) {                //RIGHT TURN
    BLINKING == true;
  }

  else if ((millis() - leftSigTime) < (blinkPeriod + 25) && (millis() - rightSigTime) < (blinkPeriod + 25)) {  //HAZARD LIGHTS
    //might cause weird interaction with brakes
    BLINKING = true;
  }

  else {
    BLINKING = false;
  }

  if ((millis() - brakeTime) > blinkPeriod) {
    BLINKING = true;        //I THINK?!
  }
  return BLINKING;
}


//MAIN LOOP. RUNS INFINIETLY
void loop() {

  //READ PINS
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);


  //ENTER CALIBRATION 
  if (millis() < 500 && BRAKE == HIGH) {      //Brake must be held down when key on 500 ms leniency 
    savedMillis = millis();                   //Get current time processor has been on
    updateShift(2,256);

    //CALIBRATION LOOP
    while ((millis() - savedMillis) < caliTimeout && BRAKE == HIGH) {    //While less than 10s AND brake is pressed
      
      //WIRING CALIRATION
      if (caliWiringSuccess == false){
        calibrateWiring();
      }

      //TIMING CALIBRATION
      if (caliTimingSuccess == false) {
        calibrateTiming();
      }

      BRAKE = digitalRead(BRAKE_PIN);

      if (caliWiringSuccess == true && caliTimingSuccess == true) {
        break;
      }
    }
  }


  //DEFAULT LIGHTING IS PERIPHERY
  if (brakeflag == true) {
    L_LEDS = leftPer + leftBrake;               
    R_LEDS = rightPer + rightBrake;

  }
  
  else {
    L_LEDS = leftPer;
    R_LEDS = rightPer;
  }



  //FOUR WIRE SECTION
  if (isFourWire == true) {

    if (L_TURN == HIGH && R_TURN == HIGH && isBlinking() == true) {   //HAZARD LIGHTS
      hazardLights();
    }

    if (L_TURN == HIGH && isBlinking() == true) {   //THIS IS FLAWED! What if you brake first then signal?! 
      runLeft();
    }

    if (R_TURN == HIGH && isBlinking() == true) {
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
  








 

