// SAE GROUP SUMMER 2017
// Team Members: Rex Baxter, Roberto Salinas, Darren Tan, Zachary Zhou

//LIBRARIES
#include "PinChangeInterrupt.h"
#include <EEPROM.h>


//PINOUTS
const int DATA = 10;        //Physical Pin 2
const int LATCH = 9;        //Physical Pin 3
const int CLK = 8;          //Physical Pin 5
const int R_TURN_PIN = 2;   //Physical Pin 11
const int BRAKE_PIN = 1;    //Physical Pin 12
const int L_TURN_PIN = 0;   //Physical Pin 13
const int TEST_PIN = 7;     //Physical Pin 6  used for loop timing

//PIN STATES
int L_TURN = 0;
int R_TURN = 0;
int BRAKE = 0;
int temp_L_TURN = 0;
int temp_R_TURN = 0;
int temp_BRAKE = 0;
int prev_L_TURN = 0;
int prev_R_TURN = 0;
int prev_BRAKE = 0;

volatile int L_LEDS = 0;
volatile int R_LEDS = 0;



//INITIALIZE TIMING VARIABLES
unsigned int blinkPeriod = 525;         //milliseconds. Active blinking time (HIGH)
unsigned int blinkDelay = 75;           //milliseconds. Delay between segment blinks
unsigned long calibrationStopWatch = 0; //milliseconds. Used for calibration timing
unsigned long caliTimeout = 10000;      //milliseconds. Calibration mode timeout
unsigned long currentMillis = 0;        //milliseconds. For emergency flashers
unsigned long previousMillis = 0;       //milliseconds. For emergency flashers
unsigned long readingMillis = 0;        //milliseconds. Used for reading buffer timing
unsigned long newTime = 0;              //milliseconds. pulseIn timing


//INITIALIZE BOOLEANS
volatile bool brakeflag = false;        //Tells if brake is on. Volatile b/c state changes in Brake Interrupt
bool isFourWire = true;                 //Default 4 wire standard in US
bool caliWiringSuccess = false;         //Calibration Flag for Wiring
bool caliTimingSuccess = false;         //Calibration Flag for Signal Timing
bool hazardFlag = false;                //Emergency Flashers state


//INITIALIZE COUNTING VARIABLES
int brakeCounter = 0;                   //Counts number of times the same reading has been measured
int L_Counter = 0;
int R_Counter = 0;


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
  pinMode(R_TURN_PIN, INPUT_PULLUP);
  pinMode(BRAKE_PIN, INPUT_PULLUP);
  pinMode(L_TURN_PIN, INPUT_PULLUP);

  pinMode(TEST_PIN, OUTPUT);

  if (digitalRead(BRAKE_PIN) == HIGH) {
    brakeflag = true;
  }

  //INTIALIZE BRAKE INTERRUPT
  attachPCINT(digitalPinToPCINT(BRAKE_PIN), brakeON, CHANGE);                 //Attaching interrupt for BRAKE_PIN. Calls brakeON() function on CHANGE

  //READ EEPROM
  //EEPROM.write(0,isFourWire);
  //EEPROM.put(1, blinkPeriod);
  isFourWire = EEPROM.read(0);                      //Reading EEPROM for wire calibration data
  blinkPeriod = EEPROM.get(1, blinkPeriod);         //Reading EEPROM for timing calibration data. get instead of read for multiple bytes
  blinkDelay = blinkPeriod / 7;                     //Used for turn signal sequencing


  //RESET BLINK
  //This is just acknowledgement that the controller has reset
  if (isFourWire = true) {
    blinkLEDS(leftPerBrake, rightPerBrake, 4, 200);   //Blink all LEDs 3 times with 250ms delay
    delay(250);
  }
  else {
    blinkLEDS(leftPerBrake, rightPerBrake, 5, 200);   //Blink all LEDs 3 times with 250ms delay
    delay(250);
  }

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
  //detect 4 or 5 wire configuration
  //do this by checking when the brake pedal is depressed
  //if both turn signals are on, it is 4 wire. otherwise it is 5

  calibrationStopWatch = millis();		  //Reset timer for calibration mode
  while (((millis() - calibrationStopWatch) < caliTimeout) && BRAKE == HIGH && caliWiringSuccess == false) {      //Timeout after 10 seconds
    //READ PINS


    if (BRAKE == HIGH && L_TURN == HIGH && R_TURN == HIGH) {    //4 Wire
      if (isFourWire != true) {       	//If the current calibration is NOT 4 wire, change to 4 wire
        EEPROM.write(0, true);          //Write to 0th memory block that isFourWire = True
        isFourWire = true;
        caliWiringSuccess = true;       //Blink center LEDS 4 times with 100ms delay
        blinkLEDS(leftBrake, rightBrake, 4, 100);
        delay(1000);
        break;
      }
      else {
        caliWiringSuccess = true;		    //Current calibration is already Four Wire, good check!
        blinkLEDS(leftBrake, rightBrake, 4, 100);
        delay(1000);                    //Blink center LEDS 4 times with 100ms delay
        break;
      }
    }
    if (BRAKE == HIGH && L_TURN == LOW && R_TURN == LOW) {    //5 Wire
      if (isFourWire != false) {        //If the current calibration is NOT 5 wire, change to 5 wire
        EEPROM.write(0, false);         //Write to 0th memory block that isFourWire = False
        isFourWire = false;
        caliWiringSuccess = true;       //Blink center LEDS 5 times with 100ms delay
        blinkLEDS(leftBrake, rightBrake, 5, 100);
        delay(1000);
        break;
      }
      else {
        caliWiringSuccess = true;		    //Current calibration is already Five Wire, good check!
        blinkLEDS(leftBrake, rightBrake, 5, 100);
        delay(1000);                    //Blink center LEDS 5 times with 100ms delay
        break;

      }
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
  blinkLEDS(leftHorn, rightHorn, 2, 250);       //Blink horns 2 times with a 250ms delay
  updateShift(leftHorn, rightHorn);

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


    //If stored value is more than 25 milliseconds difference
    if (((newTime - blinkPeriod) > 25 || (blinkPeriod - newTime) > 25) && caliTimingSuccess == true) {
      blinkPeriod = (unsigned int) newTime;
      EEPROM.put(1, blinkPeriod);                    //Store new timing in 1st address
      blinkDelay = blinkPeriod / 7;                  //New delay for horn run sequence
      delay(250);
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
  delay(blinkDelay);
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
      updateShift(L_LEDS - leftHorn, R_LEDS - rightHorn);
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
  //Uses interrupt on BRAKE_PIN to switch brake signal

  if (brakeflag == false) {

    L_LEDS = leftPerBrake;
    R_LEDS = rightPerBrake;
    brakeflag = true;

    /*
    for(int i=0; i<1000;i++){
      digitalWrite(TEST_PIN,HIGH);
      digitalWrite(TEST_PIN,LOW); 
    }
    */
  }

  else {

    L_LEDS = leftPer;
    R_LEDS = rightPer;
    brakeflag = false;
    
    for(int i=0; i<300;i++){
      digitalWrite(TEST_PIN,HIGH);
      digitalWrite(TEST_PIN,LOW); 
    }
    
  }
}




//MAIN LOOP. RUNS INFINIETLY
void loop() {
  
  //used for loop timing
  //digitalWrite(TEST_PIN,HIGH);
  //delay(10);
  //digitalWrite(TEST_PIN,LOW);     

 /*
  //READ PINS AND BUFFER 3 OF THE SAME READINGS

  brakeCounter = 0;
  L_Counter = 0;
  R_Counter = 0;

  while (brakeCounter < 5 && L_Counter < 5 && R_Counter < 5) {
    if ((millis() - readingMillis) > 15) {               //Limits reads to at least 5ms intervals
      temp_BRAKE = digitalRead(BRAKE_PIN);              //Reading for at least 15ms
      temp_L_TURN = digitalRead(L_TURN_PIN);
      temp_R_TURN = digitalRead(R_TURN_PIN);

      if (prev_BRAKE == temp_BRAKE) {                   //If brake is same, add to brake counter
        brakeCounter++;
      }
      else {                                            //Else, subtract from brake counter
        brakeCounter--;
      }


      if (prev_L_TURN == temp_L_TURN) {
        L_Counter++;
      }
      else {
        L_Counter--;
      }


      if (prev_R_TURN == temp_R_TURN) {
        R_Counter++;
      }
      else {
        R_Counter--;
      }


      if (brakeCounter >= 5 && L_Counter >= 5 && R_Counter >= 5) {
        brakeCounter = 0;
        L_Counter = 0;
        R_Counter = 0;
        BRAKE = temp_BRAKE;
        L_TURN = temp_L_TURN;
        R_TURN = temp_R_TURN;
        //updateShift(2,256);
        //delay(1000);
        break;
      }

      prev_BRAKE = temp_BRAKE;
      prev_L_TURN = temp_L_TURN;
      prev_R_TURN = temp_R_TURN;
      
      readingMillis = millis();
    }

  }

*/
  
  BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);


  //DEFAULT LIGHTING IS PERIPHERY LIGHTS
  if (BRAKE == HIGH) {                        //This case should prevent blinking when brake is held down
    L_LEDS = leftPerBrake;                    //L_LEDS will be a container that we can add to to light up different sections
    R_LEDS = rightPerBrake;                   //R_LEDS will be a container that we can add to to light up different sections
  }

  else {
    L_LEDS = leftPer;                         //L_LEDS will be a container that we can add to to light up different sections
    R_LEDS = rightPer;                        //R_LEDS will be a container that we can add to to light up different sections
  }


  
  //ENTER CALIBRATION PROCEDURE
  if (millis() < 2500 && BRAKE == HIGH) {                //Brake must be held down when key on 500 ms leniency
    calibrationStopWatch = millis();                     //Get current time processor has been on
    updateShift(2, 256);
    delay(1000);

    //CALIBRATION LOOP
    while ((millis() - calibrationStopWatch) < caliTimeout && BRAKE == HIGH) {    //While less than 10s AND brake is pressed

      //WIRING CALIRATION
      if (caliWiringSuccess == false) {
        calibrateWiring();
      }

      //TIMING CALIBRATION
      if (caliTimingSuccess == false) {
        calibrateTiming();
      }

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
        break;			//All calibrated and ready to roll
      }
    }
  }



  //FOUR WIRE SECTION
  if (isFourWire == true) {

    if (BRAKE == HIGH) {
      
      if (L_TURN == LOW && R_TURN == LOW) {     //BRAKE + EMERGENCY FLASHERS
        hazardFlag = true;
        emergencyFlashers();
      }
      
      if (L_TURN == LOW && R_TURN == HIGH) {    //BRAKE + LEFT SIGNAL
        runLeft();
      }

      if (L_TURN == HIGH && R_TURN == LOW) {    //BRAKE + RIGHT SIGNAL
        runRight();
      }
    }

    else {

      if (L_TURN == HIGH && R_TURN == HIGH) {  //EMERGENCY FLASHERS
        hazardFlag = false;
        emergencyFlashers();
      }

      if (L_TURN == HIGH && R_TURN == LOW) {                     //LEFT SIGNAL
        runLeft();
      }

      if (L_TURN == LOW && R_TURN == HIGH) {                     //RIGHT SIGNAL
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
}











