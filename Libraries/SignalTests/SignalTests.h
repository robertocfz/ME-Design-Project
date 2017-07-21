// Library to call functions that output digital signals that mimic those present in a car.
#ifndef SignalTests_h
#define SignalTests_h

#include "Arduino.h"

class SignalTests
{
	// Class constructor, public variables and public functions used in the SignalTests class (all can take 0+ input parameters)
	// Variables and functions here can be called publically, i.e. within the main program
	public:
	
	// Class constructor, initializes needed variables
	SignalTests(int leftIn, int rightIn, int brakeIn4wire, int brakeIn5wire, int hazardsIn, int leftOut, int rightOut, int brakeOut4wire, int brakeOut5wire, int hazardsOut, long interval);
	
	// Essential class functions and their input parameters (if any)
	void leftBlinkers(int LEFT);
	void rightBlinkers(int RIGHT);
	void fourWire(int BRAKE4wire);
	void fiveWire(int BRAKE5wire);
	void hazardLights(int HAZARDS);
	void readSignals();
	void callFunctions();
	void runSimulation();
	
	// Private variables and functions used in the SignalTests class, similar to the public ones
	// These variables and functions are not accessible by the main program
	private:
	
	// Note: don't use "const" if the variable is user provided or will change later
	// User provided variables:
	int _leftIn;
	int _rightIn;
	int _brakeIn4wire;
	int _brakeIn5wire;
	int _hazardsIn;
	
	int _leftOut;
	int _rightOut;
	int _brakeOut4wire;
	int _brakeOut5wire;
	int _hazardsOut;
	
	long _interval;
	
	// Volatile (constantly changing) variables
	int _LEFT;
	int _RIGHT;
	int _BRAKE4wire;
	int _BRAKE5wire;
	int _HAZARDS;
	
	unsigned long _currentMillis;
	
	unsigned long _previousMillisLeft;
	unsigned long _previousMillisRight;
	unsigned long _previousMillisHazards;
	
	int _leftState;
	int _rightState;
	int _hazardsState;
};

#endif