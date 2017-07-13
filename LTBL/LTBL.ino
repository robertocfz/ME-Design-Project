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
int L_TURN = 0;
int R_TURN = 0;
int BRAKE = 0;
int tempL_TURN = 0;
int tempR_TURN = 0;
int tempBRAKE = 0;
volatile int L_LEDS = 0;
volatile int R_LEDS = 0;


//INITIALIZE TIMING VARIABLES
unsigned long calibrationStopwatch = 0; //milliseconds. Used in Calibration
unsigned long readingStopwatch = 0;     //milliseconds. Used for software debounce
unsigned long blinkPeriod = 525;        //milliseconds. Blinking Period
unsigned long caliTimeout = 10000;      //milliseconds. Calibration mode timeout
unsigned long leftSigTime = 0;          //milliseconds. Used to determine if signal is blinking
unsigned long rightSigTime = 0;         //milliseconds. Used to determine if signal is blinking
unsigned long brakeTime = 0;            //milliseconds. Used to time how long the brakes have been on
int blinkDelay = 100;                   //milliseconds. Delay between segment blinks
int leniency = 25;					          	//milliseconds. Leniency in blinkPeriod length for logical checks



//INITIALIZE BOOLEANS
bool brakeflag = false;
bool isFourWire = true;                 //Default 4 wire standard in US     
bool caliWiringSuccess = false;         //Calibration Flag for Wiring
bool caliTimingSuccess = false;         //Calibration Flag for Signal Timing
bool leftSignalFlag = false;            //Flag that the left signal is on
bool rightSignalFlag = false;           //Flag that the right signal is on    
bool BLINKING = false;                  //Flag that shows if the signal is blinking


//INITIALIZE COUNTERS
int brakeDBCounter = 0;                 //Brake Debounce Counter
int leftDBCounter = 0;                  //Left Debounce Counter
int rightDBCounter = 0;                 //Right Debounce Counter
int debounceCounter = 5;               //Sample 10 samples over 10 milliseconds

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
isFourWire = false; //testing 5 wire

  //INITIALIZE PINS
  pinMode(DATA,OUTPUT);
  pinMode(LATCH,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(R_TURN_PIN,INPUT);
  pinMode(BRAKE_PIN,INPUT);
  pinMode(L_TURN_PIN,INPUT);

  // if (digitalRead(BRAKE_PIN) == HIGH) {                                   
  //   brakeflag = true;
  // }

  //INTIALIZE BRAKE INTERRUPT
  //attachPCINT(digitalPinToPCINT(BRAKE_PIN), brakeON, CHANGE);				    	//Attaching interrupt for BRAKE_PIN. Calls brakeON() function on CHANGE
  //attachPCINT(digitalPinToPCINT(L_TURN_PIN),leftSignalOn, RISING);				//Attaching interrupt for L_TURN_PIN. Calls leftSignalOn on RISING EDGE   
  //attachPCINT(digitalPinToPCINT(R_TURN_PIN), rightSignalOn, RISING);			//Attaching interrupt for R_TURN_PIN. Calls rightSignalOn on RISING EDGE


  //READ EEPROM
  //isFourWire = EEPROM.read(0);           //Reading EEPROM for wire calibration data
  //blinkPeriod = EEPROM.read(1);          //Reading EEPROM for timing calibration data
  //blinkDelay = blinkPeriod/7;
  

  //RESET BLINK 
  //This is just acknowledgement that the controller has reset
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

void updateShift(uint16_t left, uint16_t right) {
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

  calibrationStopwatch = millis();				  //Reset timer for calibration mode
  while ((millis() - calibrationStopwatch) < caliTimeout && BRAKE == HIGH && caliWiringSuccess == false) {      //Timeout after 10 seconds
    //READ PINS
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {    //4 Wire 
      if (isFourWire != true) {       	//If the current calibration is NOT 4 wire, change to 4 wire
        EEPROM.write(0,true);           //Write to 0th memory block that isFourWire = True
        isFourWire = true;
        updateShift(4,128);
        caliWiringSuccess = true;
        break;
      }
      else {
      	caliWiringSuccess = true;		    //Current calibration is already Four Wire, good check!
      	break;
      }
    }
    if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {    //5 Wire 
      if (isFourWire != false) {        //If the current calibration is NOT 5 wire, change to 5 wire
        EEPROM.write(0,false);          //Write to 0th memory block that isFourWire = False
        isFourWire = false;
        updateShift(4,128);
        caliWiringSuccess = true;
        break;
      }
      else {
      	caliWiringSuccess = true;		    //Current calibration is already Five Wire, good check!
      	break;
      }
    }
  }
}


void calibrateTiming() {
  //Calibrate turn signal timing 
  //Timeout after 10 seconds (see pulseIn line)

  calibrationStopwatch = millis();
  int newTime = 0;

  while ((millis() - calibrationStopwatch) < caliTimeout && BRAKE == HIGH && caliTimingSuccess == false) {
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);

    if (isFourWire == true) {
      if (isBlinking() == true) {   //susecptable to issues...check this later
        //Times signal pulse in microseconds, divide to get milliseconds. 
        //Does it twice because we can't really tell which one is blinking
        //Doesn't matter if it overwrites the first one, they should be the same
        newTime =  pulseIn(L_TURN_PIN, HIGH,1000000000)/1000; //Timeout of 10 seconds
        newTime =  pulseIn(R_TURN_PIN, HIGH,1000000000)/1000; //Timeout of 10 seconds
      }
    }

    if (isFourWire == false) {
      if (L_TURN == HIGH) {
        //Times signal pulse in microseconds, divide to get milliseconds. 
        newTime = pulseIn(L_TURN_PIN, HIGH,1000000000)/1000; //Timeout of 10 seconds
      }
      if (R_TURN == HIGH) {
        //Times signal pulse in microseconds, divide to get milliseconds. 
        newTime = pulseIn(R_TURN_PIN, HIGH,1000000000)/1000; //Timeout of 10 seconds
      }
    }

    if (newTime != 0 && newTime < 3000) {     //If period is non-zero (error in pulseIn) and does not exceed 3s
      caliTimingSuccess = true;						            //Just a quick sanity check
    }

  //If stored value is more than 25 milliseconds difference
  if (abs(newTime - blinkPeriod) > 25 && caliTimingSuccess == true) {   
    EEPROM.write(1,newTime);                     //Store new timing in 1st address
    blinkPeriod = newTime;
    blinkDelay = blinkPeriod/7;                  //New delay for horn run sequence
    updateShift(8,64);
  }
  }
}


void runLeft() {
  //Blinks the left side of horns with a delay 
  updateShift(L_LEDS - leftHorn,R_LEDS);
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
  //Blinks the right side of horns with a delay 
  updateShift(L_LEDS, R_LEDS - rightHorn); 
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS - rightHorn + R1);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS - rightHorn + R2);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS - rightHorn + R3);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS - rightHorn + R4);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS - rightHorn + R5);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS);
  delay(blinkDelay);
}

void emergencyFlashers() { 

  while (L_TURN == HIGH && R_TURN == HIGH) {
    if (L_TURN == HIGH && R_TURN == HIGH) {
      updateShift(leftPer + leftBrake, rightPer + rightBrake);
      delay(blinkPeriod);
      updateShift(0,0);
      delay(blinkPeriod);
    }

    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
  }
}

void brakeON() {
  //INTERRUPT
  //Uses interrupt on BRAKE_PIN to switch brake signal
 
 /*
  L_LEDS = leftPer + leftBrake;
  R_LEDS = rightPer + rightBrake;
  brakeTime = millis();
  */

  if (brakeflag == false) {
    
    L_LEDS = leftPer + leftBrake;
    R_LEDS = rightPer + rightBrake;
    brakeTime = millis();
    brakeflag = true;
  }

  else{
    
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
  if (millis() != readingStopwatch) {                   //If not the same millisecond
    tempBRAKE = digitalRead(BRAKE_PIN);
    tempL_TURN = digitalRead(L_TURN_PIN);
    tempR_TURN = digitalRead(R_TURN_PIN);

    //BRAKES
    if (tempBRAKE == BRAKE && brakeDBCounter > 0) {
      brakeDBCounter--;
    }
    else if (tempBRAKE != BRAKE) {
      brakeDBCounter++;
    }

    if (brakeDBCounter >= debounceCounter) {
      brakeDBCounter = 0;
      BRAKE = tempBRAKE;
    }

    //LEFT TURN
    if (tempL_TURN == L_TURN && leftDBCounter > 0) {
      leftDBCounter--;
    }
    else if (tempL_TURN != L_TURN) {
      leftDBCounter++;
    }

    if (leftDBCounter >= debounceCounter) {
      leftDBCounter = 0;
      L_TURN = tempL_TURN;
    }

    //RIGHT TURN
    if (tempR_TURN == R_TURN && rightDBCounter > 0) {
      rightDBCounter--;
    }
    else if (tempR_TURN != R_TURN) {
      rightDBCounter++;
    }

    if (rightDBCounter >= debounceCounter) {
      rightDBCounter = 0;
      R_TURN = tempR_TURN;
    }
  readingStopwatch = millis();
  }


  
  
  



/*
  //ENTER CALIBRATION 
  if (millis() < 100 && BRAKE == HIGH) {      //Brake must be held down when key on 500 ms leniency 
    calibrationStopwatch = millis();          //Get current time processor has been on
    updateShift(2,256);
    delay(3000);

    //CALIBRATION LOOP
    while ((millis() - calibrationStopwatch) < caliTimeout && BRAKE == HIGH) {    //While less than 10s AND brake is pressed
      
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
        break;			//All calibrated and ready to roll
      }
    }
  }

*/

/*
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
*/

    if (L_TURN == HIGH && R_TURN == HIGH) {   //EMERGENCY FLASHERS
      emergencyFlashers();
    }

    if (L_TURN == HIGH) {                     //LEFT SIGNAL
      runLeft();
    }
    
    if (R_TURN == HIGH) {                     //RIGHT SIGNAL
      runRight();
    }
  

  //DEFAULT LIGHTING IS PERIPHERY
  if (BRAKE == HIGH) {                      //This case should prevent blinking when brake is held down
    L_LEDS = leftPer + leftBrake;           //L_LEDS will be a container that we can add to to light up different sections
    R_LEDS = rightPer + rightBrake;         //R_LEDS will be a container that we can add to to light up different sections
  }

  else {
    L_LEDS = leftPer;                       //L_LEDS will be a container that we can add to to light up different sections
    R_LEDS = rightPer;                      //R_LEDS will be a container that we can add to to light up different sections
  }

  updateShift(L_LEDS,R_LEDS);   //OUTPUT PERIPHERY OR BRAKE LIGHTS
   
  }
  








 

