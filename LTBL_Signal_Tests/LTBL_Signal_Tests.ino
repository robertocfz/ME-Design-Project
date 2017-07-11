const int leftIn = 6; // Left signal
const int rightIn = 7; // Right signal
const int brakeIn = 8; // Brake signal

const int leftOut = 3; // Left signal, PWM output
const int brakeOut = 4; // Brake signal, digital output
const int rightOut = 5; // Right signal, PWM output

int LEFT;
int RIGHT;
int BRAKE;

// Base frequencies are divided by a provided divisor.
// The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
// The base frequency for pins 5 and 6 is 62500 Hz.
// NOTE: PWM frequencies are tied together in pairs of pins. If one in a
// pair is changed, the other is also changed to match:
// Pins 5 and 6 are paired on timer0
// Pins 9 and 10 are paired on timer1
// Pins 3 and 11 are paired on timer2
void setPWMFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

void setup() {
  //Serial.begin(9600);

  pinMode(leftIn, INPUT); // digital pin 6
  pinMode(rightIn, INPUT); // digital pin 7
  pinMode(brakeIn, INPUT); // digital pin 8
  //pinMode(buttonPin4, INPUT); // digital pin 9
  
  pinMode(leftOut, OUTPUT); // digital pin 3
  pinMode(brakeOut, OUTPUT); // digital pin 4
  pinMode(rightOut, OUTPUT); // digital pin 5
  }

void loop() {
  LEFT = digitalRead(leftIn);
  RIGHT = digitalRead(rightIn);
  BRAKE = digitalRead(brakeIn);
  
  setPWMFrequency(leftOut, 31250); // Pin 3, 1 Hz/1 second period
  setPWMFrequency(rightOut, 62500); // Pin 5, 1 Hz/1 second period
 
  if (LEFT == HIGH) // Left signal PWM output (50% duty cycle) on pin 3
  {
    analogWrite(leftOut, 127);
    //delay(1000);
    //analogWrite(leftOut,0);
  }
  else if (RIGHT == HIGH) // Right signal PWM output (50% duty cycle) on pin 5
  {
    analogWrite(rightOut, 127);
    //delay(1000);
    //analogWrite(rightOut,0);
  }
  else if (BRAKE == HIGH) // Brake signal output on pin 4
  {
      while (BRAKE == HIGH)
      {
        digitalWrite(brakeOut, HIGH);
        LEFT = digitalRead(leftIn);
      }
      digitalWrite(brakeOut, LOW);
  }
}
