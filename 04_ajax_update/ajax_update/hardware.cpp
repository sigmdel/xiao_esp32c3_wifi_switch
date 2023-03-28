#include <Arduino.h>
#include <esp32-hal-log.h>
#include "mdSimpleButton.h"
#include "DFRobot_DHT20.h"  // modified
#include "hardware.h"
#include "domoticz.h"

#define TAG "HDW"  // debug tag

// LED (i.e. relay)

void setLed(int value) {
  digitalWrite(LED_PIN, value);
  ledStatus = (value ? "ON" : "OFF");
  ESP_LOGI(TAG, "LED now %s.", ledStatus);
  updateDomoticzSwitch(SWITCH_IDX, value);  
}

void toggleLed(void) {
  setLed(1-digitalRead(LED_PIN));
}

void initLed(void) {
  ESP_LOGI(TAG, "Initializing LED I/O pin.");
  pinMode(LED_PIN, OUTPUT);
  setLed(0);
}

// Button

mdSimpleButton button = mdSimpleButton(BUTTON_PIN);

void checkButton(void) {
  if (button.update() >= BUTTON_RELEASED) {   // don't distinguish between long and short button presses
    ESP_LOGI(TAG, "Push-button released");
    toggleLed();
  }
}

// DHT Sensor

unsigned long temptime;
DFRobot_DHT20 dht20;

void initSensor() {
  ESP_LOGI(TAG, "Initializing DHT20 temperature and humidity sensor.");
  while(dht20.begin()){
    ESP_LOGE(TAG, "Initializing DHT20 sensor failed");
    delay(1000);
  }
}

void readTemp(void) {
  if (millis() - temptime > SENSOR_DELAY) {
    ESP_LOGV(TAG, "Reading temperature and humidity sensor");
    TempAndHumidity_t tah = dht20.getTempAndHumidity();
    ESP_LOGI(TAG, "Temperature %s --> %.1f", Temperature.c_str(), tah.temperature);
    Temperature = String(tah.temperature, 1);
    ESP_LOGI(TAG, "Humidity %s --> %.1f", Humidity.c_str(), 100*tah.humidity);
    Humidity = String(100*tah.humidity, 1);
    temptime = millis();
    updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, tah.temperature, 100*tah.humidity);
  }
}

// Light Sensor

unsigned long lighttime = 0;

void initLight(void) {
  pinMode(LS_PIN, INPUT);
}

void readLight() {
  if (millis() - lighttime >= SENSOR_DELAY) {
    ESP_LOGV(TAG, "Reading and updating light sensor values.");
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    int value = map(mvolt, 0, 3300, 0, 100);
    ESP_LOGI(TAG, "Light %s --> %d", Light.c_str(), value);
    Light = String(value);
    lighttime = millis();
    updateDomoticzLightSensor(LUX_IDX, value);
  }
}
/*
  //https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#check-the-battery-voltage
  // About calibration
*/

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
