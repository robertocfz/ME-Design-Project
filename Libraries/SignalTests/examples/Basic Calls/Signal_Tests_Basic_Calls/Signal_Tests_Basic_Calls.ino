#include "SignalTests.h"

// Pin mapout
const int leftIn = 2;
const int rightIn = 3;
const int brakeIn4wire = 4;
const int brakeIn5wire = 5;
const int hazardsIn = 6;

const int leftOut = 7;
const int rightOut = 8;
const int brakeOut4wire = 9;
const int brakeOut5wire = 10;
const int hazardsOut = 11;

// Input pin states
int LEFT;
int RIGHT;
int BRAKE4wire;
int BRAKE5wire;
int HAZARDS;

// Interval for blinkers
const long interval = 150; // milliseconds

// Initialize pins and simulation parameters
SignalTests sim(leftIn, rightIn, brakeIn4wire, brakeIn5wire, hazardsIn, leftOut, rightOut, brakeOut4wire, brakeOut5wire, hazardsOut, interval);

void setup() {};

void loop() 
{
  // Read in inputs
  LEFT = digitalRead(leftIn);
  RIGHT = digitalRead(rightIn);
  BRAKE4wire = digitalRead(brakeIn4wire);
  BRAKE5wire = digitalRead(brakeIn5wire);
  HAZARDS = digitalRead(hazardsIn);

  // Alternatively, replace the above with:
  //sim.readSignals();

  // Call functions to send appropriate outputs
  sim.fiveWire(BRAKE5wire);
  sim.fourWire(BRAKE4wire);
  sim.rightBlinkers(RIGHT);
  sim.leftBlinkers(LEFT);
  sim.hazardLights(HAZARDS);

  // Alternatively, replace the above with:
  //sim.callFunctions();
}
