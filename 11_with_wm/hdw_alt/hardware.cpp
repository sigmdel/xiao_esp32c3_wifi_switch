//--- Alternate Hardware Drivers ---

// The brightness sensor is a light dependant resistor (LDR)
// The temperature & humidity sensor can be a DHT11 or a DHT22

// Implements rolling average measure of the brightness if LS_FIFO > 1
// Implements constant verification of the 1-Wire connection to the DHTxx

// Relay (light)
#define RELAY_PIN  10 // D10  

// Push button
#define BUTTON_PIN 3  // D1

// Brightness sensor
#define LS_PIN     2  // D0

// Temperature and humidity sensor
#define THS_PIN    8  // D8

#define DHT11      1      // 1-wire DHT11 sensor
#define DHT22      2      // 1-wire DHT22 sensor
#define THS        DHT11  // Installed temperature/humidiy sensor type

#define LS_FIFO    10     // size of FIFO queue when calculating rolling average if < 2, then no averaging done
#define LS_READ    10000  // minimum time (in ms) between readings of the sensor data if averaging

//--------------------------------

#include "arduino_config.h"     // done with build_flags in platformIO
#include <Arduino.h>
#include <Ticker.h>
#include "ESPAsyncWebServer.h"  // for AsyncEventSource
#include <SimpleDHT.h>
#include "mdSimpleButton.h"
#include "config.h"
#include "logging.h"
#include "hardware.h"
#include "domoticz.h"

#ifndef NO_TESTS
  #define TEST_THS_FAIL  // test temperature & humidity sensor
  #define DEBUG_LS_FIFO  // generates a lot of debug messages
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

mdSimpleButton button = mdSimpleButton(BUTTON_PIN);

void checkButton(void) {
  switch( button.update()) {
    case BUTTON_LONGPRESS:
      addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button long press"));
      esp_restart();
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
bool hasTempSensor = false;


#ifdef TEST_THS_FAIL
DFRobot_DHT20 dht20(&Wire, 0x43);  // invalid I2C address
#else
DFRobot_DHT20 dht20;
#endif

#ifdef TEST_THS_FAIL
#undef THS_PIN
#define THS_PIN -1
#endif

#if (THS == DHT11)
  // Hardware DHT11 sensor
  SimpleDHT11 dht_wire(THS_PIN);
#elif (THS == DHT22)
  SimpleDHT22 dht_wire(THS_PIN);
#else
  #error "***No valid DHT defined***"
#endif

void initSensor() {
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Initializing %s type temperature and humidity sensor"),
    #if (THS==DHT11)
    "DHT11"
    #else
    "DHT22"
    #endif
  );
  // use dht_wire.setPin() instead of above
  hasTempSensor = (dht_wire.setPinInputMode(INPUT_PULLUP) == SimpleDHTErrSuccess);
  if (!hasTempSensor) {
    addToLogP(LOG_ERR, TAG_HARDWARE, PSTR("Initializing temperature and humidity sensor failed"));
    Temperature = "(no sensor)";
    Humidity = "(no sensor)";
  }
  temptime = millis();
}

void readTemp(void) {
  if (millis() - temptime > config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading temperature and humidity data"));
    // read without samples.
    float temperature = 0;
    float humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht_wire.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      addToLogPf(LOG_ERR, TAG_HARDWARE, PSTR("Reading DHT (1-Wire) failed, error: %d"),
        SimpleDHTErrCode(err));
      // append '?' after old numeric measurement to show it is out of date
      if ((Temperature.indexOf("?") < 0) and (Temperature.indexOf("(") < 0))
        Temperature += "?";
      if ((Humidity.indexOf("?") < 0) and (Humidity.indexOf("(") < 0))
        Humidity += "?";
      // Domoticz shows time of last good value
    } else {
      addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature %s --> %.1f"), Temperature.c_str(), temperature);
      Temperature = String(temperature, 1);
      addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Humidity %s --> %.1f"), Humidity.c_str(), humidity);
      Humidity = String(humidity, 1);
      events.send(Temperature.c_str(), "tempvalue");        // updates all Web clients
      events.send(Humidity.c_str(), "humdvalue");           // and Domoticz
      updateDomoticzTemperatureHumiditySensor(config.dmtzTHSIdx, temperature, humidity);
      addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Temperature and humidity data updated"));
    }
    temptime = millis();
  }
}

// Brightness Sensor

unsigned long brightnesstime = 0;

#if (LS_FIFO > 1)
unsigned long lightreadtime = 0;
#endif

void initBrightness(void) {
  pinMode(LS_PIN, INPUT);
  brightnesstime = millis();
  #if (LS_FIFO > 1)
  lightreadtime = millis();
  #endif
}

#if (LS_FIFO > 1)

// variables used for rolling average LDR value
uint32_t lsValues[LS_FIFO] {0};
uint32_t lsSum = 0;
uint32_t lsAvg = 0;
int lsIndex = 0;
int lsCount = 0;

// adds the a new LS value and returns the new average
uint32_t addlsValue(uint32_t newValue) {
     lsSum = lsSum - lsValues[lsIndex] + newValue;
     lsValues[lsIndex] = newValue;
     if (lsCount < LS_FIFO) lsCount++;
     lsIndex = (lsIndex + 1) % LS_FIFO;
     return (uint32_t) lsSum / lsCount;
}

void readBrightness() {
  if (millis() - lightreadtime >= LS_READ) {
    #ifdef DEBUG_LS_FIFO
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading brightness sensor data"));
    #endif
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    lsAvg = addlsValue(mvolt);
    #ifdef DEBUG_LS_FIFO
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Raw light value: %d, avg: %d"), mvolt, lsAvg);
    #endif
    lightreadtime = millis();
  }

  if (millis() - brightnesstime >= config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Updating average brightness value."));
    int value = map(lsAvg, 0, 3300, 100, 0);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness %s --> %d"), Brightness.c_str(), value);
    Brightness = String(value);
    brightnesstime = millis();
    events.send(Brightness.c_str(),"brightvalue");             // updates all Web clients
    updateDomoticzBrightnessSensor(config.dmtzLSIdx, value);  // and Domoticz
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Average brightness data updated"));
  }
}

#else  // no rolling average

void readBrightness() {
  if (millis() - brightnesstime >= config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading brightness sensor data"));
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    int value = map(mvolt, 0, 3300, 100, 0);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness %s --> %d"), Brightness.c_str(), value);
    Brightness = String(value);
    brightnesstime = millis();
    events.send(Brightness.c_str(),"brightvalue");            // updates all Web clients
    updateDomoticzBrightnessSensor(config.dmtzLSIdx, value);  // and Domoticz
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Brightness data updated"));
  }
}

#endif  // no FIFO


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
