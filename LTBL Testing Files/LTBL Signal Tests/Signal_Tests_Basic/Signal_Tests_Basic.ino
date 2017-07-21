const int leftIn = 2;
const int rightIn = 4;
const int brakeIn = 6;

const int leftOut = 8;
const int rightOut = 10;
const int brakeOut = 12;

int LEFT;
int RIGHT;
int BRAKE;

int leftState;
int rightState;

unsigned long previousMillisLeft = 0;
unsigned long previousMillisRight = 0;

const long interval = 150;

void setup() 
{
  Serial.begin(9600);
  
  pinMode(leftIn, INPUT);
  pinMode(rightIn, INPUT);
  pinMode(brakeIn, INPUT);
  
  pinMode(leftOut, OUTPUT);
  pinMode(rightOut, OUTPUT);
  pinMode(brakeOut, OUTPUT);
}

void loop() 
{
  unsigned long currentMillis = millis();

  LEFT = digitalRead(leftIn);
  RIGHT = digitalRead(rightIn);
  BRAKE = digitalRead(brakeIn);

  // BRAKE SIGNAL
  if (BRAKE == HIGH) 
  {
    digitalWrite(brakeOut, HIGH);
  }
  else
  {
    digitalWrite(brakeOut, LOW);
  }
    
  // LEFT SIGNAL
  if ( (currentMillis - previousMillisLeft >= interval) && (LEFT == HIGH) ) 
  {
    previousMillisLeft = currentMillis;
  
    if (leftState == LOW) 
    {
      leftState = HIGH;
    } 
    else 
    {
      leftState = LOW;
    }
  digitalWrite(leftOut, leftState);
  }
  else if ( (currentMillis - previousMillisLeft >= interval) && (LEFT == LOW) ) 
  {
    digitalWrite(leftOut, LOW);
  }

  // RIGHT SIGNAL
  if ( (currentMillis - previousMillisRight >= interval) && (RIGHT == HIGH) ) 
  {
    previousMillisRight = currentMillis;
  
    if (rightState == LOW) 
    {
      rightState = HIGH;
    } 
    else 
    {
      rightState = LOW;
    }
    digitalWrite(rightOut, rightState);
  }
  else if ( (currentMillis - previousMillisRight >= interval) && (RIGHT == LOW) ) 
  {
    digitalWrite(rightOut, LOW);
  }
}
