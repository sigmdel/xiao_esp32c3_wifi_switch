// hardware.h

#pragma once

#include <Arduino.h>  // pin definitions

// The sensor data in main.cpp that the hardware will update - all strings
extern String ledStatus;
extern String Temperature;
extern String Humidity;
extern String Light;

// LED (or relay)
#define LED_PIN    D10

// Push button
#define BUTTON_PIN D1

// Light sensor
#define LS_PIN     D0    // has to be one of D0 - D3 (A0 - A3)

// Temperature and humidity sensor
#define DHT_PIN    D8     // not used if DHT = DHT20 or DHT_NONE


#define DHT_NONE   0      // emulate a temperature/humidity sensor
#define DHT11      1      // 1-wire DHT11 sensor
#define DHT22      2      // 1-wire DHT22 sensor
#define DHT20      3      // I²C DHT20 sensor Grove temperature/humidity sensor
#define DHT        DHT11  // Installed temperature/humidiy sensor type

// light sensor
#define LS_NONE    0       // emulate light sensor
#define LS_LDR     1       // light dependant resistor
#define LS_DIODE   2       // photodiode sensor Grove Light Sensor 1.2
#define LS         LS_LDR  // installed light sensor type

#define LS_FIFO      10      // size of FIFO queue when calculating rolling average if < 2, then no averaging done
#define LS_READ      10000   // minimum time (in ms) between readings of the sensor data if averaging

#define SENSOR_DELAY 60000   // minimum time (in ms) between updates of the sensor data

// Hardware abstraction
void initHardware(void);   // Initialize the hardware (LED, button, temperature and light sensors)
void checkHardware(void);  // Read button and sensors and update readings strings in main.cpp
void toggleLed(void);      // Toggle the LED state and update ledStatus in main.cpp
void setLed(int value);    // Set the LED on (value = 1) or off (value = 0)
