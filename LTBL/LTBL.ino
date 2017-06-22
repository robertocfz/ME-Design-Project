
//PINOUTS
const int DATA = 10; //Physical Pin 2 
const int LATCH = 9; //Physical Pin 3
const int CLK = 8; //Physical Pin 5
const int R_TURN_PIN = 2; //Physical Pin 11
const int BRAKE_PIN = 1; //Physical Pin 12
const int L_TURN_PIN = 0; //Physical Pin 13

//PIN STATES
int R_TURN = 0;
int L_TURN = 0;
volatile int BRAKE = 0;
volatile int L_LEDS = 0;
volatile int R_LEDS = 0;

//INITIALIZE VARIABLES
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long blinkLength = 75; //milliseconds
int brakeflag = 0;




//LIGHTING STATES
const int leftHorn = 126;
const int rightHorn = 504;
const int leftPer = 14590;
const int rightPer = 5116;
const int leftBrake = 1792;
const int rightBrake = 3074;


void setup() {

  //initialize pins
  pinMode(DATA,OUTPUT);
  pinMode(LATCH,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(R_TURN_PIN,INPUT);
  pinMode(BRAKE_PIN,INPUT);
  pinMode(L_TURN_PIN,INPUT);

  //Interrupt for Brakes
  attachInterrupt(digitalPinToInterrupt(BRAKE_PIN), brakeON, CHANGE);

  //turn off all leds to signify reset
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
  
}


void calibrateTiming() {

   //calibrate turn signal timing
  //use pulseIn library

  blinkLength = pulseIn(L_TURN_PIN, HIGH,1000000000); //large number for timeout length in microseconds
}

void runProgram() {
  int timeDelay = 1000;
  updateShift(14590,5116); //Periph
  delay(timeDelay);
  //updateShift(126,504); //Left and Right horn
  //delay(timeDelay);
  runLeft();
  runLeft();
  runLeft();
  updateShift(1792,3074); //brake lights
  delay(timeDelay);
  updateShift(leftPer+leftBrake,rightPer+rightBrake); //All on
  delay(timeDelay);
  updateShift(0,0);
  delay(500);
}


void runLeft() {
  //126 is value of all horns
  updateShift(L_LEDS - 126,R_LEDS);
  delay(blinkLength);
  updateShift(L_LEDS + 64 -126,R_LEDS);
  delay(blinkLength);
  updateShift(L_LEDS + 96 - 126,R_LEDS);
  delay(blinkLength);
  updateShift(L_LEDS + 112 - 126,R_LEDS);
  delay(blinkLength);
  updateShift(L_LEDS + 120 - 126,R_LEDS);
  delay(blinkLength);
  updateShift(L_LEDS + 124 - 126,R_LEDS);
  delay(blinkLength);
  updateShift(L_LEDS,R_LEDS);
  delay(blinkLength);
  updateShift(0,0);
}

void runRight() {
  //504 is value of all horns
  updateShift(L_LEDS, R_LEDS - 504); 
  delay(blinkLength);
  updateShift(L_LEDS ,R_LEDS + 8 - 504);
  delay(blinkLength);
  updateShift(L_LEDS ,R_LEDS + 24 - 504);
  delay(blinkLength);
  updateShift(L_LEDS ,R_LEDS + 56 - 504);
  delay(blinkLength);
  updateShift(L_LEDS ,R_LEDS + 120 - 504);
  delay(blinkLength);
  updateShift(L_LEDS ,R_LEDS + 248 - 504);
  delay(blinkLength);
  updateShift(L_LEDS ,R_LEDS);
  delay(blinkLength);
  updateShift(0,0);
}

void brakeON() {
  if (brakeflag == 0) {
    L_LEDS = L_LEDS + leftBrake;
    R_LEDS = R_LEDS + rightBrake;
    updateShift(L_LEDS, R_LEDS);
    brakeflag = 1;
  }

  else{
    L_LEDS = L_LEDS - leftBrake;
    R_LEDS = R_LEDS - rightBrake;
    updateShift(L_LEDS, R_LEDS);
    brakeflag = 0;
  }

}

void loop() {
  //READ PINS
  //BRAKE = digitalRead(BRAKE_PIN);
  L_TURN = digitalRead(L_TURN_PIN);
  R_TURN = digitalRead(R_TURN_PIN);
  
  //default to periphery lights
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
  








 

