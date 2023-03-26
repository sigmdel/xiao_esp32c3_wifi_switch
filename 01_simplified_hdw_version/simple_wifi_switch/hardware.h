#pragma once

// The sensor data in main.cpp that the hardware will update - all strings
extern String ledStatus;
extern String Temperature;
extern String Humidity;
extern String Light;

// LED (or relay)
#define LED_PIN    D10 // Grove LED Socket kit connected to XIAO female headers on XIAO Expansion base

// push button
#define BUTTON_PIN D1  // User button on XIAO Expansion base

// Light sensor
#define LS_PIN     D0  //  connected to XIA0 Expansion base A0-D0 Grove connector

// DHT20 uses the default I²C pins: SDA = D4 = 6 and SCL = D5 = 7

#define SENSOR_DELAY 10000    // minimum time (in ms) between updates of the sensor data

// Hardware abstraction
void initHardware(void);  // Initialize the hardware (LED, button, temperature and light sensors)
void checkHardware(void); // Read button and sensors and update readings strings in main.cpp
void toggleLed(void);     // Toggle the LED state and update ledStatus in main.cpp
