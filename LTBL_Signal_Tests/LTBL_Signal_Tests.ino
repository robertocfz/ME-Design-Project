const int buttonPin1 = 6; // Left signal
const int buttonPin2 = 7; // Right signal
const int buttonPin3 = 8; // Brake signal

const int pwmOutput1 = 3; // Left signal, PWM output
const int digitalOutput1 = 4; // Brake signal, digital output
const int pwmOutput2 = 5; // Right signal, PWM output

int buttonStatus1;
int buttonStatus2;
int buttonStatus3;

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
