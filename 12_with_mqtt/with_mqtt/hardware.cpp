//--- Hardware Configuration when Using Seeed Studio XIAO Starter Kit ---

// Relay (light)
#define RELAY_PIN   10 // (D10) Grove LED Socket kit in lieu relay, connected to XIAO female headers on XIAO Expansion base

// Push button
#define BUTTON_PIN  3  // (D1) User button on XIAO Expansion base

// Brightness sensor
#define LS_PIN      2  // (D0) Grove Light Sensor v1.2 connected to XIA0 Expansion base A0-D0 connector

// Grove DHT20 Temperature and Humidity sensor connected to I²C Grove connector on XIAO Expansion base
// Uses the default I²C pins: SDA = D4 = 6 and SCL = D5 = 7

//--------------------------------

#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>
#include <Ticker.h>
#include "ESPAsyncWebServer.h"  // for AsyncEventSource
#include "mdSimpleButton.h"
#include "DFRobot_DHT20.h"      // modified
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
  if (value) value = 1; // defensive
  if (digitalRead(RELAY_PIN) != value) {
    // avoid loops when Domoticz MQTT Hardware Prevent Loop is set to False
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Set relay to %d"), value);
    digitalWrite(RELAY_PIN, value);
    RelayState = (value ? "ON" : "OFF");
    // tell everyone
    events.send(RelayState.c_str(),"relaystate");        // updates all Web clients
    updateDomoticzSwitch(config.dmtzSwitchIdx, value);   // and Domoticz
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Relay state updated"));
  }
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

extern void espRestart(int level = 0);

mdSimpleButton button = mdSimpleButton(BUTTON_PIN);

void checkButton(void) {
  switch( button.update()) {
    case BUTTON_LONGPRESS:
      if (button.presstime > 30000) { // more than 30 seconds
        addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button long press - restart 7"));
        espRestart(7);
      } else if (button.presstime > 10000) { // more than 10 seconds
        addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button long press - restart 3"));
        espRestart(3);
      } else {
        addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button long press - restart 0"));
        espRestart(0);
      }
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

#ifdef TEST_THS_FAIL
DFRobot_DHT20 dht20(&Wire, 0x43);  // invalid I2C address
#else
DFRobot_DHT20 dht20;
#endif

#define DHT_ATTEMPTS 5

bool hasTempSensor = false;

void initSensor() {
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Initializing DHT20 temperature and humidity sensor"));

  for (int i=0; i<DHT_ATTEMPTS; i++) {
    hasTempSensor = !dht20.begin();
    if (hasTempSensor) break;
    delay(1000);
  }

  if (!hasTempSensor) {
    addToLogP(LOG_ERR, TAG_HARDWARE, PSTR("Initializing DHT20 sensor failed"));
    Temperature = "(no sensor)";
    Humidity = "(no sensor)";
  }
}

void readTemp(void) {
  if (!hasTempSensor) return;
  if (millis() - temptime > config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR( "Reading temperature and humidity data"));
    TempAndHumidity_t tah = dht20.getTempAndHumidity();
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature %s --> %.1f"), Temperature.c_str(), tah.temperature);
    Temperature = String(tah.temperature, 1);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Humidity %s --> %.1f"), Humidity.c_str(), 100*tah.humidity);
    Humidity = String(100*tah.humidity, 1);
    temptime = millis();
    events.send(Temperature.c_str(),"tempvalue");        // updates all Web clients
    events.send(Humidity.c_str(),"humdvalue");           // and Domoticz
    updateDomoticzTemperatureHumiditySensor(config.dmtzTHSIdx, tah.temperature, 100*tah.humidity);
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Temperature and humidity data updated"));
  }
}


// Brightness Sensor

unsigned long brightnesstime = 0;

void initBrightness(void) {
  pinMode(LS_PIN, INPUT);
}

void readBrightness() {
  if (millis() - brightnesstime >= config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading brightness sensor value"));
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    int value = map(mvolt, 0, 3300, 0, 100);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness %s --> %d"), Brightness.c_str(), value);
    Brightness = String(value);
    brightnesstime = millis();
    events.send(Brightness.c_str(),"brightvalue");            // updates all Web clients
    updateDomoticzBrightnessSensor(config.dmtzLSIdx, value);  // and Domoticz
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
