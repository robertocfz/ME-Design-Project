
const int DATA = 10; //Physical Pin 2 
const int LATCH = 9; //Physical Pin 3
const int CLK = 8; //Physical Pin 5
const int R_Turn = 2; //Physical Pin 11
const int Brake = 1; //Physical Pin 12
const int L_Turn = 0; //Physical Pin 13

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
  pinMode(R_Turn,INPUT);
  pinMode(Brake,INPUT);
  pinMode(L_Turn,INPUT);

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
  delay(1000);

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

void runLeft() {
  int x = 100;
  updateShift(64,8);
  delay(x);
  updateShift(96,24);
  delay(x);
  updateShift(112,56);
  delay(x);
  updateShift(120,120);
  delay(x);
  updateShift(124,248);
  delay(x);
  updateShift(126,273);
  delay(x);
  updateShift(0,0);
}

void Calibrate() {
  //enter calibration if brake is switched 3 times


  //detect 4 or 5 wire configuration
  //do this by checking when the brake pedal is depressed 
  //if both turn signals are on, it is 4 wire. otherwise it is 5

  //calibrate turn signal timing
  //use pulseIn library
  
}

void loop() {
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






 

