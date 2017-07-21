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

// Interval for blinkers
const long interval = 150; // milliseconds

// Initialize pins and simulation parameters
SignalTests sim(leftIn, rightIn, brakeIn4wire, brakeIn5wire, hazardsIn, leftOut, rightOut, brakeOut4wire, brakeOut5wire, hazardsOut, interval);

void setup() {};

void loop() 
{
    // Run entire simulation automatically (read inputs and call functions to send appropriate outputs)
    sim.runSimulation();
}
