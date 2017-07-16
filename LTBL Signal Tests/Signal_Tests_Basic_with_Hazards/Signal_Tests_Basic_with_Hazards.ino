const int leftIn = 8;
const int rightIn = 10;
const int brakeIn = 12;
const int hazardsIn = 3;

const int leftOut = 2;
const int rightOut = 4;
const int brakeOut = 6;
const int hazardsOut = 7;

int LEFT;
int RIGHT;
int HAZARDS;
int BRAKE;

int leftState;
int rightState;
int hazardsState;

unsigned long previousMillisLeft = 0;
unsigned long previousMillisRight = 0;
unsigned long previousMillisHazards = 0;

const long interval = 150;

void setup() 
{
  Serial.begin(9600);
  
  pinMode(leftIn, INPUT);
  pinMode(rightIn, INPUT);
  pinMode(brakeIn, INPUT);
  pinMode(hazardsIn, INPUT);
  
  pinMode(leftOut, OUTPUT);
  pinMode(rightOut, OUTPUT);
  pinMode(brakeOut, OUTPUT);
  pinMode(hazardsOut, OUTPUT);
}

void loop() 
{
  unsigned long currentMillis = millis();

  LEFT = digitalRead(leftIn);
  RIGHT = digitalRead(rightIn);
  BRAKE = digitalRead(brakeIn);
  HAZARDS = digitalRead(hazardsIn);

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

  // HAZARDS SIGNAL
  if ( (currentMillis - previousMillisHazards >= interval) && (HAZARDS == HIGH) ) 
    {
      previousMillisHazards = currentMillis;
    
      if (hazardsState == LOW) 
      {
        hazardsState = HIGH;
      } 
      else 
      {
        hazardsState = LOW;
      }
      digitalWrite(hazardsOut, hazardsState);
    }
  else if ( (currentMillis - previousMillisHazards >= interval) && (HAZARDS == LOW) )
  {
    digitalWrite(hazardsOut, LOW);
  }
  
}

