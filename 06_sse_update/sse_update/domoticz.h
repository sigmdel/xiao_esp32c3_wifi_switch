#pragma once

#include "domoticz_data.h"

// Update the state of the virtual switch with given idx, value=0 for Off, value=1 for On.
bool updateDomoticzSwitch(int idx, int value);

// Update the "Lux" level to value in the light sensor with the given idx.
bool updateDomoticzLightSensor(int idx, int value);

// Update the Temperature (value1) and Humidity (value2) values in the Temp+Humidity
// sensor with the given idx. Must set the humidity state to normal (0), comfortabe (1),
// dry (2) or wet (3) explicitly, Domoticz does not calculate a value even though
// it does calculate the dew point.
//
// Note value1 must be in °C even if °F are chosen as units in Domoticz settings
//      value2 must be a percent (from 0 to 100) such as 48.9 and will be displayed as 48.9%
bool updateDomoticzTemperatureHumiditySensor(int idx, float value1, float value2, int state=0);
