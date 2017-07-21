The basic premise behind signal testing is to mimic vehicle signals using an Arduino so that the LTBL can read these signals in and light up the correct patterns as if it were connected to a real car.
Essentially, the commands the LTBL needs to interpret are L and R blinkers (recieved as 50% duty cycle PWM waves), braking (4 wire and 5 wire), and hazards (L+R blinking at the same time). 
Special note on 4 vs 5 wire braking - vehicles in the US typically use 4 wires for third brake lights, while some non-US produced vehicles use 5 wires. The main difference is how braking is handled:
-4 wire: to simulate braking, L and R solid HIGH, or non-PWM/non-blinking, signals are used and interpreted as braking
-5 wire: a dedicated brake signal is used and interpreted as braking

Thus, the setup for testing these conditions for the LTBL is thus:
-Connect buttons to input pins to send whichever signal you want (blinkers, brakes, etc.) to the output pins
-Split the output pin for the 4 wire brake signal using a breadboard and jumper wires and connect it to the L and R pins on the LTBL
-Similarly for 4 wire brakes, the hazards light signal output should be split using a breadboard and connected to the L and R pins on the LTBL
-Connect the other output pins normally (including the 5 wire brake signal)