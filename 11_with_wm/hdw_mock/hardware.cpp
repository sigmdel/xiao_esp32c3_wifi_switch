//--- Mock Hardware Drivers ---

// Emulates the brightness and temperature and humidity sensors.

// Relay (light)
#ifndef RELAY_PIN
#define RELAY_PIN  10    // D10 on XIAO ESP32-C3
#endif

// Push button
#ifndef BUTTON_PIN
#define BUTTON_PIN  3     // D1 on XIAO ESP32-C3
#endif


//--------------------------------

#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>
#include <Ticker.h>
#include "ESPAsyncWebServer.h"  // for AsyncEventSource
#include "mdSimpleButton.h"
#include "config.h"
#include "logging.h"
#include "hardware.h"
#include "domoticz.h"


#ifndef NO_TESTS
  #define TEST_THS_FAIL  // test temperature & humidity sensor
#endif

extern AsyncEventSource events;

// Relay

void setRelay(int value) {
  addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Set relay to %d"), value);
  digitalWrite(RELAY_PIN, value);
  RelayState = (value ? "ON" : "OFF");
  // tell everyone
  events.send(RelayState.c_str(),"relaystate");        // updates all Web clients
  updateDomoticzSwitch(config.dmtzSwitchIdx, value);   // and Domoticz
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Relay state updated"));
}

void toggleRelay(void) {
  setRelay(1-digitalRead(RELAY_PIN));
}

void initRelay(void) {
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Initializing relay I/O pin."));
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(0);
}

// Button

extern void espRestart(void);

mdSimpleButton button = mdSimpleButton(BUTTON_PIN);

void checkButton(void) {
  switch( button.update()) {
    case BUTTON_LONGPRESS:
      addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button long press - restart"));
      espRestart();
      break;
    case BUTTON_RELEASED:
      addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button released"));
      toggleRelay();
      break;
    default:   // avoid unhandled BUTTON_UNCHANGED warning/error
      break;
  }
}

// Temperature and Humidity Sensor (THS)

unsigned long temptime;

// nothing to setup, just implement a pseudo radom walk around initial values

bool hasTempSensor = false;

void initSensor() {
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Initializing emulated temperature and humidity sensor"));
  temptime = millis();
  #ifdef TEST_THS_FAIL
  hasTempSensor = false;
  #else
  hasTempSensor = true;
  #endif

  if (!hasTempSensor) {
    addToLogP(LOG_ERR, TAG_HARDWARE, PSTR("Initializing temperature and humidity sensor failed"));
    Temperature = "(no sensor)";
    Humidity = "(no sensor)";
  }
}

void readTemp(void) {
  if (!hasTempSensor) return;
  if (millis() - temptime > config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR( "Reading temperature and humidity data"));
    float temp = Temperature.toFloat() + (float) (random(100)-50)/60;
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature %s --> %.1f"), Temperature.c_str(), temp);
    Temperature = String(temp, 1);
    float humid = Humidity.toFloat() + (float) (random(100)-50)/60;
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Humidity %s --> %.1f"), Humidity.c_str(), humid);
    Humidity = String(humid, 1);
    temptime = millis();
    events.send(Temperature.c_str(),"tempvalue");        // updates all Web clients
    events.send(Humidity.c_str(),"humdvalue");           // and Domoticz
    updateDomoticzTemperatureHumiditySensor(config.dmtzTHSIdx, temp, humid);
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Temperature and humidity data updated"));
  }
}


// Brightness Sensor

unsigned long brightnesstime = 0;

const int TrendRun = 20;   // seed for length of run in one direction
int currentTrendRun = 10;
int runCount = 0;
int runDir = 1;
int BrightnessValue = 50;

void ReverseTrend(void) {
  runDir = -1*runDir;
  currentTrendRun = random(TrendRun, 3*TrendRun) / 2;
  runCount = 0;
}

void initBrightness(void) {
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Initializing emulated light sensor."));
  BrightnessValue = Brightness.toInt();
  brightnesstime = millis();
}

void readBrightness(void) {
  if (millis() - brightnesstime >= config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading brightness sensor data"));
    BrightnessValue += runDir;
    if (BrightnessValue > 98)
      BrightnessValue = 90;
    else if (BrightnessValue < 5)
      BrightnessValue = 10;
    runCount++;
    if (runCount > currentTrendRun)
      ReverseTrend();
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness %s --> %d"), Brightness.c_str(), BrightnessValue);
    Brightness = String(BrightnessValue);
    brightnesstime = millis();
    events.send(Brightness.c_str(),"brightvalue");                 // updates all Web clients
    updateDomoticzBrightnessSensor(config.dmtzLSIdx, BrightnessValue); // and Domoticz
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Brightness data updated"));
  }
}

void checkHardware(void) {
  checkButton();
  readTemp();
  readBrightness();
}

Ticker ticker;

void initHardware(void) {
  initRelay();
  initSensor();
  initBrightness();
  // Initialize sensor timers so that a sensor is polled every config.sensorUpdtTime/2 ms
  // and wait 2 seconds before starting
  int HALF_DELAY = config.sensorUpdtTime/2;
  brightnesstime = millis() - HALF_DELAY + 2000;
  temptime = brightnesstime - HALF_DELAY;         // will start with temperature sensor
  ticker.attach_ms(config.hdwPollTime, checkHardware);
}
