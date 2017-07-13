// Create and assign a constant/non-changing variable to indicate the pin a particular button is connected to
const int buttonPin1 = 7;

// Create output pin variables and assinging them for use with digitalWrite() or analogWrite()
// Note: pins 3, 5, 6, 9, 10, and 11 are special pins that can output PWM waves on the Uno board. These PWM pins use values between 0-255 (0 = 0% duty cycle, 255 = 100% duty cycle) for their PWM waves/square waves
// Note 2: Avoid using 5 or 6 for PWM output. The internal timer for PWM are conected to these, and have a higher frequency than the other PWM pins
// Note 3: Digital pins (0-13 on the Uno board) are not Analog pins. Analog inputs are values from 0-1023, not 0 or 1 for normal digital pins or 0-255 for PWM digital pins
const int digitalOutput1 = 2;
const int digitalOutput2 = 4;
const int pwmOutput1 = 3;
const int pwmOutput2 = 5;

// Create a variable that will store the status of a button(s)
int buttonStatus1;

// Create a counter that will count the number of times the button is pressed
int counter1 = 0;

// Setup code runs only once, everytime the Arduino is powered ON. Use to setup pins as input/outputs
void setup() {
  // Setup a monitor (usually, the computer you're using) to later print values for debugging
  Serial.begin(9600);

  // Initialize the button pin(s) so as to listen for input
  pinMode(buttonPin1, INPUT); // digital pin 7

  // Initialize output pins
  pinMode(digitalOutput1, OUTPUT); // digital pin 2
  pinMode(digitalOutput2, OUTPUT); // digital pin 4
  pinMode(pwmOutput1, OUTPUT); // digital PWM pin 3
  pinMode(pwmOutput2, OUTPUT); // digital PWM pin 5

  // Initialize port register variables. Port registers are an entirely different topic and requires studying
  DDRB = B00000011; // Sets pins 8 and 9 as outputs, 10-13 as outputs (6th and 7th digits are not usable/conencted to any pins)
}

// Continually runs the following code in a loop until the Arduino is powered off
// Note: each line of code is executed approximately within 10 microseconds of the previous line, i.e. not instantly or simultaneously unless you use Port Registers
void loop() {
  // Update the buttonStatus variable(s) to ON or OFF depending if button is pressed
  buttonStatus1 = digitalRead(buttonPin1);

  if (buttonStatus1 == HIGH)
  {
    // Keep track of the number of times the button is pressed
    counter1 = counter1 + 1;

    // Create case for when the button is pressed the first time
    if (counter1 == 1)
    {
      // Output a single, constant HIGH/ON 5V signal until switch off
      digitalWrite(digitalOutput1, HIGH); // pin 2

      // Outputs a PWM wave that is approximately 50% duty cycle
      analogWrite(pwmOutput1, 127); // pin 3
  
      // If simultaneous/instant output manipulate is wanted, use of the Port Register variables is needed
      PORTB = B00000011; // Sets pins 8 and 9 as high simultaneously
    }

    // Create case for when the button is pressed the second time
    if (counter1 == 2)
    {
      // Insert code desired here
    }

    // Create case for when the button is pressed the third time
    if (counter1 == 3)
    {
      // Insert code desired here
      
      // Reset the counter to indicate a new set of button presses
      counter1 = 0;
    }

    // Delay to allow for another button press to register, value is in miliseconds
    delay(1000); // 1 second wait
  }
}
