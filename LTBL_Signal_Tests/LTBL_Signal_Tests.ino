const int buttonPin1 = 6; // Left signal
const int buttonPin2 = 7; // Right signal
const int buttonPin3 = 8; // Brake signal

const int pwmOutput1 = 3; // Left signal, PWM output
const int digitalOutput1 = 4; // Brake signal, digital output
const int pwmOutput2 = 5; // Right signal, PWM output

int buttonStatus1;
int buttonStatus2;
int buttonStatus3;

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
  Serial.begin(9600);

  pinMode(buttonPin1, INPUT); // digital pin 6
  pinMode(buttonPin2, INPUT); // digital pin 7
  pinMode(buttonPin3, INPUT); // digital pin 8
  //pinMode(buttonPin4, INPUT); // digital pin 9
  
  pinMode(pwmOutput1, OUTPUT); // digital pin 3
  pinMode(digitalOutput1, OUTPUT); // digital pin 4
  pinMode(pwmOutput2, OUTPUT); // digital pin 5
  }

void loop() {
  buttonStatus1 = digitalRead(buttonPin1);
  buttonStatus2 = digitalRead(buttonPin2);
  buttonStatus3 = digitalRead(buttonPin3);
  
  setPWMFrequency(pwmOutput1, 31250); // Pin 3, 1 Hz/1 second period
  setPWMFrequency(pwmOutput2, 62500); // Pin 5, 1 Hz/1 second period
 
  if (buttonStatus1 == HIGH) // Left signal PWM output (50% duty cycle) on pin 3
  {
    analogWrite(pwmOutput1, 127);
    //delay(1000);
    //analogWrite(pwmOutput1,0);
  }
  else if (buttonStatus2 == HIGH) // Right signal PWM output (50% duty cycle) on pin 5
  {
    analogWrite(pwmOutput2, 127);
    //delay(1000);
    //analogWrite(pwmOutput2,0);
  }
  else if (buttonStatus3 == HIGH) // Brake signal output on pin 4
  {
      while (buttonStatus3 == HIGH)
      {
        analogWrite(digitalOutput1, HIGH);
        buttonStatus1 = digitalRead(buttonPin1);
      }
      digitalWrite(digitalOutput1, LOW);
  }
}
