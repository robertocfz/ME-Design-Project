// SAE GROUP SUMMER 2017
// Team Members: Rex Baxter, Roberto Salinas, Darren Tan, Zachary Zhou

//LIBRARIES
#include "Arduino.h"
#include "PinChangeInterrupt.h"

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
unsigned long currentMillis = 0;    //milliseconds
unsigned long blinkPeriod = 525;    //milliseconds. Initialize default period
unsigned long caliTime = 10000;     //milliseconds
int blinkDelay = 75;                //milliseconds. Initialize default delay



//INITIALIZE OTHER VARIABLES
int brakeflag = 0;
int brakeCounter = 0;
int wireConfig = 0;     
int hazardflag = 0;


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


int calibrateWiring() {
  //enter calibration if brake is switched 3 times
  //detect 4 or 5 wire configuration
  //do this by checking when the brake pedal is depressed 
  //if both turn signals are on, it is 4 wire. otherwise it is 5
  int calibrateWireSuccess = 0;
  currentMillis = millis();

  while (millis() - currentMillis < caliTime) {                  //Timeout after 10 seconds
    //read pins
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
    currentMillis = millis();           //for timeout exits

    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {
      wireConfig = 4;
      calibrateWireSuccess = 1;
      updateShift(4,128);
      break;
    }
    else if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {
      wireConfig = 5;
      calibrateWireSuccess = 1;
      updateShift(4,128);
      break;
    }
    
  }
  if (calibrateWireSuccess == 0) {
    updateShift(leftBrake,rightBrake);
  }
  return calibrateWireSuccess;
}


void calibrateTiming(int PIN) {
  //Calibrate turn signal timing 
  //Timeout after 10 seconds
  blinkPeriod = (pulseIn(PIN, HIGH,1000000000))/1000; //large number for timeout length in milliseconds
  blinkDelay = blinkPeriod/7;
  updateShift(8,64);
}



void runLeft() {
  //Blinks the left side of horns with a delay 
  //126 is value of all left horn LEDs
  updateShift(L_LEDS - 126,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 64 -126,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 96 - 126,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 112 - 126,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 120 - 126,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS + 124 - 126,R_LEDS);
  delay(blinkDelay);
  updateShift(L_LEDS,R_LEDS);
  delay(blinkDelay);

}

void runRight() {
  //Blinks the right side of horns with a delay 
  //504 is value of all right horn LEDs
  updateShift(L_LEDS, R_LEDS - 504); 
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 8 - 504);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 24 - 504);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 56 - 504);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 120 - 504);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS + 248 - 504);
  delay(blinkDelay);
  updateShift(L_LEDS ,R_LEDS);
  delay(blinkDelay);

}

void hazardLights() { 

  while (L_TURN == R_TURN) {
    BRAKE = digitalRead(BRAKE_PIN);
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
    brakeCounter++;
  }

  else{
    
    L_LEDS = L_LEDS - leftBrake;
    R_LEDS = R_LEDS - rightBrake;   
    brakeflag = 0;
  }

}

void loop() {

  //READ PINS
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);
  

  //CALIBRATION SECTION
  if (millis() < caliTime && BRAKE == HIGH && brakeCounter > 5) {        //Timeout after 10 seconds
    int calibrationFlag = 0;
    currentMillis = millis();
    updateShift(2,256);

    while (millis() - currentMillis < caliTime) {
      BRAKE = digitalRead(BRAKE_PIN);
      L_TURN = digitalRead(L_TURN_PIN);
      R_TURN = digitalRead(R_TURN_PIN);
      
      
      if (BRAKE == HIGH && (L_TURN == HIGH || R_TURN == HIGH)) {
        if (calibrateWiring() == 1) {
          if (L_TURN == HIGH) {
            calibrateTiming(L_TURN_PIN);
            //updateShift(4,64);
          }
          else if (R_TURN == HIGH) {
            calibrateTiming(R_TURN_PIN);
            //updateShift(8,64);
          }
        }
      }
    }
  }


  //Periphery lights on as default
  if (brakeflag == 1) {
    L_LEDS = leftPer + leftBrake;               
    R_LEDS = rightPer + rightBrake;

  }
  else {
    L_LEDS = leftPer;
    R_LEDS = rightPer;
  }

  //HAZARD LIGHTS
  if(L_TURN == HIGH && R_TURN == HIGH) {
    hazardLights();
    }

  if(L_TURN == HIGH) {
    runLeft();
  }
  
  if(R_TURN == HIGH) {
    runRight();
  }

  updateShift(L_LEDS,R_LEDS);
  }
  








 

