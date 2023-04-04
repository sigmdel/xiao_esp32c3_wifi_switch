#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>
#include "ESPAsyncWebServer.h"  // for AsyncEventSource
#include "mdSimpleButton.h"
#include "DFRobot_DHT20.h"      // modified
#include "logging.h"
#include "hardware.h"
#include "domoticz.h"

#define TEST_DHT_FAIL

#ifdef NO_TESTS
  // remove all test modules
  #undef TEST_DHT_FAIL
#endif


extern AsyncEventSource events;

// LED (i.e. relay)

void setLed(int value) {
  digitalWrite(LED_PIN, value);
  ledStatus = (value ? "ON" : "OFF");
  // tell everyone
  addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("LED now %s"), ledStatus);
  events.send(ledStatus.c_str(),"ledstate");  // updates all Web clients
  updateDomoticzSwitch(SWITCH_IDX, value);    // will slow reaction if Domoticz not on line
}

void toggleLed(void) {
  setLed(1-digitalRead(LED_PIN));
}

void initLed(void) {
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Initializing LED I/O pin."));
  pinMode(LED_PIN, OUTPUT);
  setLed(0);
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
      toggleLed();
      break;
    default:   // avoid unhandled BUTTON_UNCHANGED warning/error
      break;
  }
}

// DHT Sensor

unsigned long temptime;

#ifdef TEST_DHT_FAIL
DFRobot_DHT20 dht20(&Wire, 0x43);
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
  if (millis() - temptime > SENSOR_DELAY) {
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR( "Reading temperature and humidity sensor"));
    TempAndHumidity_t tah = dht20.getTempAndHumidity();
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Temperature %s --> %.1f"), Temperature.c_str(), tah.temperature);
    Temperature = String(tah.temperature, 1);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Humidity %s --> %.1f"), Humidity.c_str(), 100*tah.humidity);
    Humidity = String(100*tah.humidity, 1);
    temptime = millis();
    events.send(Temperature.c_str(),"tempvalue");                                               // updates all Web clients
    events.send(Humidity.c_str(),"humdvalue");                                                  // updates all Web clients
    updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, tah.temperature, 100*tah.humidity);  // will slow reaction if Domoticz not on line
  }
}

// Light Sensor

unsigned long lighttime = 0;

void initLight(void) {
  pinMode(LS_PIN, INPUT);
}

void readLight() {
  if (millis() - lighttime >= SENSOR_DELAY) {
    addToLogPf(LOG_DEBUG, TAG_HARDWARE, PSTR("Reading and updating light sensor values"));
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    int value = map(mvolt, 0, 3300, 0, 100);
    addToLogPf(LOG_INFO, TAG_HARDWARE, PSTR("Light %s --> %d"), Light.c_str(), value);
    Light = String(value);
    lighttime = millis();
    events.send(Light.c_str(),"lightvalue");       // updates all Web clients
    updateDomoticzLightSensor(LUX_IDX, value);     // will slow reaction if Domoticz not on line
  }
}

void initHardware(void) {
  initLed();
  initSensor();
  initLight();
  // Initialize sensor timers so that a sensor is polled every SENSOR_DELAY/2 ms
  // and wait 2 seconds before starting
  int HALF_DELAY = SENSOR_DELAY/2;
  lighttime = millis() - HALF_DELAY + 2000;
  temptime = lighttime - HALF_DELAY;         // will start with temperature sensor
}

void checkHardware(void) {
  checkButton();
  readTemp();
  readLight();
}
