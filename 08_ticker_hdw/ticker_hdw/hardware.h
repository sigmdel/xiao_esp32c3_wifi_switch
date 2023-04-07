#pragma once

#include <Arduino.h>  // pin definitions

// The sensor data in main.cpp that the hardware will update - all strings
extern String ledStatus;
extern String Temperature;
extern String Humidity;
extern String Light;

// LED (or relay)
#define LED_PIN    D10 // Grove LED Socket kit connected to XIAO female headers on XIAO Expansion base

// Push button
#define BUTTON_PIN D1  // User button on XIAO Expansion base

// Light sensor
#define LS_PIN     D0  //  connected to XIA0 Expansion base A0-D0 Grove connector

// DHT20 uses the default I²C pins: SDA = D4 = 6 and SCL = D5 = 7

#define SENSOR_DELAY 120000  // minimum time (in ms) between updates of the sensor data

#define POLL_TIME    25      // time (in ms) between polling of hardware,
                             // 50 ms probably fast enough
// Hardware abstraction
void initHardware(void);    // Initialize the hardware (LED, button, temperature and light sensors)
void toggleLed(void);       // Toggle the LED state and update ledStatus in main.cpp
void setLed(int value);     // Set the LED on (value = 1) or off (value = 0)
