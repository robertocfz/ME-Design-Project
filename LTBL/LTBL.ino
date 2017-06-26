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
unsigned long previousMillis = 0;   //milliseconds
unsigned long blinkLength = 75;     //MICROseconds


//INITIALIZE OTHER VARIABLES
int brakeflag = 0;
int brakeCounter = 0;
int wireConfig = 0;     
int calibrateWireSuccess = 0;


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
  //enter calibration if brake is switched 3 times
  //detect 4 or 5 wire configuration
  //do this by checking when the brake pedal is depressed 
  //if both turn signals are on, it is 4 wire. otherwise it is 5
  currentMillis = millis();

  while (millis() - currentMillis < 10000) {
    //read pins
    BRAKE = digitalRead(BRAKE_PIN);
    L_TURN = digitalRead(L_TURN_PIN);
    R_TURN = digitalRead(R_TURN_PIN);
    currentMillis = millis();           //for timeout exits

    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {
      wireConfig = 4;
      calibrateWireSuccess = 1;
      break;
    }
    else if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {
      wireConfig = 5;
      calibrateWireSuccess = 1;
      break;
    }
  }
}


void calibrateTiming(int PIN) {
  //Calibrate turn signal timing 
  blinkLength = pulseIn(PIN, HIGH,1000000000); //large number for timeout length in microseconds
}



void runLeft() {
  //Blinks the left side of horns with a delay 
  //126 is value of all horns
  updateShift(L_LEDS - 126,R_LEDS);
  delay(blinkLength/1000);
  updateShift(L_LEDS + 64 -126,R_LEDS);
  delay(blinkLength/1000);
  updateShift(L_LEDS + 96 - 126,R_LEDS);
  delay(blinkLength/1000);
  updateShift(L_LEDS + 112 - 126,R_LEDS);
  delay(blinkLength/1000);
  updateShift(L_LEDS + 120 - 126,R_LEDS);
  delay(blinkLength/1000);
  updateShift(L_LEDS + 124 - 126,R_LEDS);
  delay(blinkLength/1000);
  updateShift(L_LEDS,R_LEDS);
  delay(blinkLength/1000);
  updateShift(0,0);
}

void runRight() {
  //Blinks the right side of horns with a delay 
  //504 is value of all horns
  updateShift(L_LEDS, R_LEDS - 504); 
  delay(blinkLength/1000);
  updateShift(L_LEDS ,R_LEDS + 8 - 504);
  delay(blinkLength/1000);
  updateShift(L_LEDS ,R_LEDS + 24 - 504);
  delay(blinkLength/1000);
  updateShift(L_LEDS ,R_LEDS + 56 - 504);
  delay(blinkLength/1000);
  updateShift(L_LEDS ,R_LEDS + 120 - 504);
  delay(blinkLength/1000);
  updateShift(L_LEDS ,R_LEDS + 248 - 504);
  delay(blinkLength/1000);
  updateShift(L_LEDS ,R_LEDS);
  delay(blinkLength/1000);
  updateShift(0,0);
}

void brakeON() {
  //Uses interrupt on BRAKE_PIN to switch brake signal
  if (brakeflag == 0) {
    L_LEDS = L_LEDS + leftBrake;
    R_LEDS = R_LEDS + rightBrake;
    updateShift(L_LEDS, R_LEDS); //might not need this line?
    brakeflag = 1;
  }

  else{
    L_LEDS = L_LEDS - leftBrake;
    R_LEDS = R_LEDS - rightBrake;
    updateShift(L_LEDS, R_LEDS); //might not need this line?
    brakeflag = 0;
  }

}

void loop() {

  //READ PINS
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);
  currentMillis = millis();

  /////////////////////
  //CALIBRATION SECTION
  //if brake is pressed 3 times within 10 secs of turning on
  //unsure how to enter calibration
  //if (millis() < 10000 && //brakeCounter >3) something like that
  /////////////////////

  //Periphery lights on as default
  L_LEDS = leftPer;               
  R_LEDS = rightPer;

  if(L_TURN == HIGH) {
    runLeft();
  }
  if(R_TURN == HIGH) {
    runRight();
  }

  updateShift(L_LEDS,R_LEDS);
  }
  








 

