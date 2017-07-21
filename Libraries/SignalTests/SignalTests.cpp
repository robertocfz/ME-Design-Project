#include "Arduino.h"
#include "SignalTests.h"

// Class constructor function, establish and initialize inputs, outputs, and library parameters
SignalTests::SignalTests(int leftIn, int rightIn, int brakeIn4wire, int brakeIn5wire, int hazardsIn, int leftOut, int rightOut, int brakeOut4wire, int brakeOut5wire, int hazardsOut, long interval)
{
	_leftIn = leftIn;
	_rightIn = rightIn;
	_brakeIn4wire = brakeIn4wire;
	_brakeIn5wire = brakeIn5wire;
	_hazardsIn = hazardsIn;
	
	_leftOut = leftOut;
	_rightOut = rightOut;
	_brakeOut4wire = brakeOut4wire;
	_brakeOut5wire = brakeOut5wire;
	_hazardsOut = hazardsOut;
	
	pinMode(_leftIn, INPUT);
	pinMode(_rightIn, INPUT);
	pinMode(_brakeIn4wire, INPUT);
	pinMode(_brakeIn5wire, INPUT);
	pinMode(_hazardsIn, INPUT);
	
	pinMode(_leftOut, OUTPUT);
	pinMode(_rightOut, OUTPUT);
	pinMode(_brakeOut4wire, OUTPUT);
	pinMode(_brakeOut5wire, OUTPUT);
	pinMode(_hazardsOut, OUTPUT);
	
	// The interval should be in millseconds
	_interval = interval;
	
	_previousMillisLeft = 0;
	_previousMillisRight = 0;
	_previousMillisHazards = 0;
	
	_leftState = LOW;
	_rightState = LOW;
	_hazardsState = LOW;
}

// Left signal function, toggle between ON and OFF outputs within a given interval and 50% duty cycle as long as input is HIGH
void SignalTests::leftBlinkers(int LEFT)
{
	if ( (_currentMillis - _previousMillisLeft >= _interval) && (LEFT == HIGH) ) 
	{
		_previousMillisLeft = _currentMillis;
		
		if (_leftState == LOW) 
		{
			_leftState = HIGH;
		}
		else
		{
			_leftState = LOW;
		}
		digitalWrite(_leftOut, _leftState);
	}
	else if ( (_currentMillis - _previousMillisLeft >= _interval) && (LEFT == LOW) ) 
	{
		digitalWrite(_leftOut, LOW);
	}
}

// Right signal function, toggle between ON and OFF outputs within a given interval and 50% duty cycle as long as input is HIGH
void SignalTests::rightBlinkers(int RIGHT)
{
	if ( (_currentMillis - _previousMillisRight >= _interval) && (RIGHT == HIGH) ) 
	{
		_previousMillisRight = _currentMillis;
		
		if (_rightState == LOW) 
		{
			_rightState = HIGH;
		} 
		else
		{
			_rightState = LOW;
		}
		digitalWrite(_rightOut, _rightState);
	}
	else if ( (_currentMillis - _previousMillisRight >= _interval) && (RIGHT == LOW) ) 
	{
		digitalWrite(_rightOut, LOW);
	}
}

// 4 wire brake signal function, solid ON or OFF signal outputs dependent on pin input
void SignalTests::fourWire(int BRAKE4wire)
{
	if (BRAKE4wire == HIGH)
	{
		digitalWrite(_brakeOut4wire, HIGH);
	}
	else
	{
		digitalWrite(_brakeOut4wire, LOW);
	}
}

// 5 wire brake signal function, solid ON or OFF signal outputs dependent on pin input
void SignalTests::fiveWire(int BRAKE5wire)
{
	if (BRAKE5wire == HIGH)
	{
		digitalWrite(_brakeOut5wire, HIGH);
	}
	else
	{
		digitalWrite(_brakeOut5wire, LOW);
	}
}

// Hazard signal function, toggle between ON and OFF outputs within a given interval and 50% duty cycle as long as input is HIGH
void SignalTests::hazardLights(int HAZARDS)
{
	if ( (_currentMillis - _previousMillisHazards >= _interval) && (HAZARDS == HIGH) )
	{
		_previousMillisHazards = _currentMillis;
		
		if (_hazardsState == LOW) 
		{
			_hazardsState = HIGH;
		}
		else
		{
			_hazardsState = LOW;
		}
		digitalWrite(_hazardsOut, _hazardsState);
	}
	else if ( (_currentMillis - _previousMillisHazards >= _interval) && (HAZARDS == LOW) )
	{
		digitalWrite(_hazardsOut, LOW);
	}
}

// Read inputs
void SignalTests::readSignals()
{
	_LEFT = digitalRead(_leftIn);
	_RIGHT = digitalRead(_rightIn);
	_BRAKE4wire = digitalRead(_brakeIn4wire);
	_BRAKE5wire = digitalRead(_brakeIn5wire);
	_HAZARDS = digitalRead(_hazardsIn);
}

// Call functions
void SignalTests::callFunctions()
{
	_currentMillis = millis();
	
	leftBlinkers(_LEFT);
	rightBlinkers(_RIGHT);
	fourWire(_BRAKE4wire);
	fiveWire(_BRAKE5wire);
	hazardLights(_HAZARDS);
}

// Run all functions together
void SignalTests::runSimulation()
{
	readSignals();
	callFunctions();
}