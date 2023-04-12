#pragma once

#include <Arduino.h>  // pin definitions

// The sensor data in main.cpp that the hardware will update - all strings
extern String RelayState;
extern String Temperature;
extern String Humidity;
extern String Brightness;

// Hardware abstraction
void initHardware(void);     // Initialize the hardware (LED, button, temperature and light sensors)
void toggleRelay(void);      // Toggle the relay state and update String relayState
void setRelay(int value);    // Set the LED on (value = 1) or off (value = 0)
