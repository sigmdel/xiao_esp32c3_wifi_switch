//--- Hardware configuration ---

// Relay (light)
#define RELAY_PIN  D10    // Grove RELAY Socket kit connected to XIAO female headers on XIAO Expansion base

// Push button
#define BUTTON_PIN D1     // User button on XIAO Expansion base

// Brightness sensor
#define LS_PIN     D0     //  connected to XIA0 Expansion base A0-D0 Grove connector

// Temperature and humidity sensor
#define THS_PIN    D8     // not used if THS = DHT20 or THS_NONE

#define MOCK       0      // emulated sensor

#define DHT11      1      // 1-wire DHT11 sensor
#define DHT22      2      // 1-wire DHT22 sensor
#define DHT20      3      // I²C DHT20 sensor Grove temperature/humidity sensor
#define THS        DHT11  // Installed temperature/humidiy sensor type

// DHT20 uses the default I²C pins: SDA = D4 = 6 and SCL = D5 = 7

// light sensor
#define LS_LDR     1        // light dependant resistor
#define LS_DIODE   2        // photodiode sensor Grove Light Sensor 1.2
#define LS         LS_LDR   // installed light sensor type

#define LS_FIFO    10       // size of FIFO queue when calculating rolling average if < 2, then no averaging done
#define LS_READ    10000    // minimum time (in ms) between readings of the sensor data if averaging

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

// DHT sensor library and sanity check
#if (THS==DHT20)
  #include <DFRobot_DHT20.h>  // modified
#elif (THS==DHT11) || (THS==DHT22)
  #ifndef THS_PIN
    #error "***I/O Pin to DHT11 or DHT22 not defined***"
  #endif
  #include <SimpleDHT.h>
#elif (THS!=MOCK)
  #error "***No valid DHT defined***"
#endif

#define TEST_THS_FAIL  // assume THS failure is being tested

#ifdef NO_TESTS
  // remove all test modules
  #undef TEST_THS_FAIL
#endif

extern AsyncEventSource events;

// Relay

void setRelay(int value) {
  addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Set relay to %d"), value);
  digitalWrite(RELAY_PIN, value);
  RelayState = (value ? "ON" : "OFF");
  // tell everyone
  events.send(RelayState.c_str(),"relaystate");    // updates all Web clients
  updateDomoticzSwitch(SWITCH_IDX, value);         // and Domoticz
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
      addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Push-button long press - restarting"));
      delay(1000); // wait one second
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

#if (THS == MOCK)

// nothing to setup, just implement a pseuod radom walk around initial values

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
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR( "Reading temperature and humidity data"));
    float temp = Temperature.toFloat() + (float) (random(100)-50)/60;
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature %s --> %.1f"), Temperature.c_str(), temp);
    Temperature = String(temp, 1);
    float humid = Humidity.toFloat() + (float) (random(100)-50)/60;
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Humidity %s --> %.1f"), Humidity.c_str(), humid);
    Humidity = String(humid, 1);
    temptime = millis();
    events.send(Temperature.c_str(),"tempvalue");        // updates all Web clients
    events.send(Humidity.c_str(),"humdvalue");           // and Domoticz
    updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, temp, humid);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature and humidity data updated"));
  }
}
#endif // THS == MOCK

#ifdef TEST_THS_FAIL
DFRobot_DHT20 dht20(&Wire, 0x43);  // invalid I2C address
#else
DFRobot_DHT20 dht20;
#endif

#if (THS == DHT20)

#define DHT_ATTEMPTS 5

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
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR( "Reading temperature and humidity data"));
    TempAndHumidity_t tah = dht20.getTempAndHumidity();
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature %s --> %.1f"), Temperature.c_str(), tah.temperature);
    Temperature = String(tah.temperature, 1);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Humidity %s --> %.1f"), Humidity.c_str(), 100*tah.humidity);
    Humidity = String(100*tah.humidity, 1);
    temptime = millis();
    events.send(Temperature.c_str(),"tempvalue");        // updates all Web clients
    events.send(Humidity.c_str(),"humdvalue");           // and Domoticz
    updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, tah.temperature, 100*tah.humidity);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature and humidity data updated"));
  }
}

#endif // THS == DHT20



#if (THS == DHT11) || (THS == DHT22)

#ifdef TEST_THS_FAIL
#undef THS_PIN
#define THS_PIN -1
#endif

#if (THS == DHT11)
  // Hardware DHT11 sensor
  SimpleDHT11 dht_wire(THS_PIN);
#elif (THS == DHT22)
  SimpleDHT22 dht_wire(THS_PIN);
#endif

void initSensor() {
    ESP_LOGI(TAG, "Initializing %s temperature and humidity sensor.",
    #if (THS==DHT11)
    "DHT11"
    #else
    "DHT22"
    #endif
  );
  // use dht_wire.setPin() instead of above
  hasTempSensor = dht_wire.setPinInputMode(INPUT_PULLUP);
  if (!hasTempSensor) {
    addToLogP(LOG_ERR, TAG_HARDWARE, PSTR("Initializing temperature and humidity sensor failed"));
    Temperature = "(no sensor)";
    Humidity = "(no sensor)";
  }
  temptime = millis();
}

void readTemp(void) {
  if (millis() - temptime > config.sensorUpdtTime) {
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading temperature and humidity data"));
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
      updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, temperature, humidity);
      addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature and humidity data updated"));
  }
    temptime = millis();
  }
}

#endif // THS == DHT11 or THS == DHT22

// Light Sensor

unsigned long brightnesstime = 0;

#if (LS==MOCK)

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
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading light sensor value"));
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
    events.send(Brightness.c_str(),"brightvalue");            // updates all Web clients
    updateDomoticzBrightnessSensor(LUX_IDX, BrightnessValue); // and Domoticz
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness data updated"));
  }
}

#else  // real light sensor

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
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading light sensor."));
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    lsAvg = addlsValue(mvolt);
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Raw light value: %d, avg: %d"), mvolt, lsAvg);
    lightreadtime = millis();
  }

  if (millis() - brightnesstime >= config.sensorUpdtTime) {
    addToLogP(LOG_DEBUG, TAG_HARDWARE, PSTR("Updating average brightness value."));
    #if (LS==LS_LDR)
    int value = map(lsAvg, 0, 3300, 100, 0);
    #else
    int value = map(lsAvg, 0, 3300, 0, 100);
    #endif
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness %s --> %d"), Brightness.c_str(), value);
    Brightness = String(value);
    brightnesstime = millis();
    events.send(Brightness.c_str(),"brightvalue");   // updates all Web clients
    updateDomoticzBrightnessSensor(LUX_IDX, value);       // and Domoticz
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness data updated"));
  }
}

#else  // no rolling average

void readBrightness() {
  if (millis() - brightnesstime >= config.sensorUpdtTime) {
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading light sensor value"));
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    #if (LS==LS_LDR)
    int value = map(mvolt, 0, 3300, 100, 0);
    #else
    int value = map(mvolt, 0, 3300, 0, 100);
    #endif
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Brightness %s --> %d"), Brightness.c_str(), value);
    Brightness = String(value);
    brightnesstime = millis();
    events.send(Brightness.c_str(),"brightvalue"); // updates all Web clients
    updateDomoticzBrightnessSensor(LUX_IDX, value);     // and Domoticz
    addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Brightness data updated"));
  }
}

#endif  // no FIFO
#endif  // not LS_NONE

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
