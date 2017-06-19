
const int DATA = 10; //Physical Pin 2 
const int LATCH = 9; //Physical Pin 3
const int CLK = 8; //Physical Pin 5
const int R_Turn = 2; //Physical Pin 11
const int Brake = 1; //Physical Pin 12
const int L_Turn = 0; //Physical Pin 13
unsigned long sigTime = 0;


int x = 0;
int foo = 0;

void setup() {


  pinMode(DATA,OUTPUT);
  pinMode(LATCH,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(R_Turn,INPUT);
  pinMode(Brake,INPUT);
  pinMode(L_Turn,INPUT);
  updateShift(DATA,CLK,MSBFIRST,0);
  delay(2000);

}



void updateShift(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint16_t val){
  uint8_t i;
  //digitalWrite(LATCH,LOW);
  for(i=0;i<16;i++){
    if (bitOrder == LSBFIRST){
      digitalWrite(dataPin, !!(val & (1 << i)));
    }
     else{
       digitalWrite(dataPin, !!(val & (1 << (16 - i))));
     }
  
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);       
    }
  //digitalWrite(LATCH,HIGH);
}



void loop() {
/*
  for(int i=0;i<x;i++){
  updateShift(DATA,CLK,MSBFIRST,i);
  delay(500);
  }
  */
  digitalWrite(LATCH,LOW);
  updateShift(DATA,CLK,MSBFIRST,8191); //Right side
  updateShift(DATA,CLK,MSBFIRST,16383);  //Left side 
  digitalWrite(LATCH,HIGH);
}






 

