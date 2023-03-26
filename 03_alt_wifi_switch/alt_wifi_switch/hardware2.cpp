#include <Arduino.h>
#include <esp32-hal-log.h>
#include "mdSimpleButton.h"
#include "domoticz.h"
#include "hardware2.h"


// DHT sensor library and sanity check
#if (DHT==DHT20)
  #include <DFRobot_DHT20.h>  // modified
#elif (DHT==DHT11) || (DHT==DHT22)
  #ifndef DHT_PIN
    #error "***I/O Pin to DHT11 or DHT22 not defined***"
  #endif
  #include <SimpleDHT.h>
#elif (DHT!=DHT_NONE)
  #error "***No valid DHT defined***"
#endif

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
  switch( button.update()) {
    case BUTTON_LONGPRESS:
      ESP_LOGI(TAG, "Push-button long press - restarting");
      delay(1000); // wait one second
      esp_restart();
      break;
    case BUTTON_RELEASED:
      ESP_LOGI(TAG, "Push-button released");
      toggleLed();
      break;
  }
}

// DHT Sensor

unsigned long temptime;

#if (DHT == FAKE_DHT)

// nothing to setup, just implement a pseuod radom walk around initial values

void initSensor() {
  ESP_LOGI(TAG, "Initializing emulated temperature sensor.");
  temptime = millis();
}

void readTemp(void) {
  if (millis() - temptime > SENSOR_DELAY) {
    ESP_LOGV(TAG, "Reading temperature and humidity sensor");
    float temp = Temperature.toFloat() + (float) (random(100)-50)/60;
    ESP_LOGI(TAG, "Temperature %s --> %.1f", Temperature.c_str(), temp);
    Temperature = String(temp, 1);

    float humid = Humidity.toFloat() + (float) (random(100)-50)/60;
    ESP_LOGI(TAG, "Humidity %s --> %.1f", Humidity.c_str(), humid);
    Humidity = String(humid, 1);
    temptime = millis();
    updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, temp, humid)
  }
}
#endif // DHT == FAKE_DHT


#if (DHT == DHT20)

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

#endif // DHT == DHT20

#if (DHT == DHT11)
  // Hardware DHT11 sensor
  SimpleDHT11 dht_wire(DHT_PIN);
#elif (DHT == DHT22)
  SimpleDHT22 dht_wire(DHT_PIN);
#endif

#if (DHT == DHT11) || (DHT == DHT22)

void initSensor() {
    ESP_LOGI(TAG, "Initializing %s temperature and humidity sensor.",
    #if (DHT==DHT11)
    "DHT11"
    #else
    "DHT22"
    #endif
  );
  dht_wire.setPinInputMode(INPUT_PULLUP);
  temptime = millis();
}

void readTemp(void) {
  if (millis() - temptime > SENSOR_DELAY) {
    ESP_LOGV(TAG, "Reading temperature and humidity sensor");
    // read without samples.
    float temperature = 0;
    float humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht_wire.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      ESP_LOGE(TAG, "Reading DHT (1-Wire) failed, error: %d", SimpleDHTErrCode(err));
    } else {
      ESP_LOGI(TAG, "Temperature %s --> %.1f", Temperature.c_str(), temperature);
      Temperature = String(temperature, 1);

      ESP_LOGI(TAG, "Humidity %s --> %.1f", Humidity.c_str(), humidity);
      Humidity = String(humidity, 1);
    }
    temptime = millis();
    updateDomoticzTemperatureHumiditySensor(TEMP_HUMI_IDX, temperature, humidity);
  }
}

#endif // DHT == DHT11 or DHT == DHT22

// Light Sensor

unsigned long lighttime = 0;

#if (LS==LS_NONE)

const int TrendRun = 20;   // seed for length of run in one direction
int currentTrendRun = 10;
int runCount = 0;
int runDir = 1;
int LightValue = 50;

void ReverseTrend(void) {
  runDir = -1*runDir;
  currentTrendRun = random(TrendRun, 3*TrendRun) / 2;
  runCount = 0;
}

void initLight(void) {
  ESP_LOGI(TAG, "Initializing emulated light sensor.");
  LightValue = Light.toInt();
  lighttime = millis();
}

void readLight(void) {
  if (millis() - lighttime >= LS_READ) {
    ESP_LOGV(TAG, "Reading and updating light sensor values.");
    LightValue += runDir;
    if (LightValue > 98)
      LightValue = 90;
    else if (LightValue < 5)
      LightValue = 10;
    runCount++;
    if (runCount > currentTrendRun)
      ReverseTrend();
    ESP_LOGI(TAG, "Light %s --> %d", Light.c_str(), LightValue);
    Light = String(LightValue);
    lighttime = millis();
    updateDomoticzLightSensor(LUX_IDX, LightValue);
  }
}

#else  // real light sensor

#if (LS_FIFO > 1)
unsigned long lightreadtime = 0;
#endif

void initLight(void) {
  pinMode(LS_PIN, INPUT);
  lighttime = millis();
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

void readLight() {
  if (millis() - lightreadtime >= LS_READ) {
    ESP_LOGI(TAG, "Reading light sensor.");
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    lsAvg = addlsValue(mvolt);
    ESP_LOGI(TAG, "Raw light value: %d, avg: %d", mvolt, lsAvg);
    lightreadtime = millis();
  }

  if (millis() - lighttime >= SENSOR_DELAY) {
    ESP_LOGI(TAG, "Updating average light sensor value.");
    #if (LS==LS_LDR)
    int value = map(lsAvg, 0, 3300, 100, 0);
    #else
    int value = map(lsAvg, 0, 3300, 0, 100);
    #endif
    ESP_LOGI(TAG, "Light %s --> %d", Light.c_str(), value);
    Light = String(value);
    lighttime = millis();
    updateDomoticzLightSensor(LUX_IDX, value);
  }
}

#else  // no rolling average

void readLight() {
  if (millis() - lighttime >= SENSOR_DELAY) {
    ESP_LOGI(TAG, "Reading and updating light sensor values.");
    uint32_t mvolt = analogReadMilliVolts(LS_PIN);
    #if (LS==LS_LDR)
    int value = map(mvolt, 0, 3300, 100, 0);
    #else
    int value = map(mvolt, 0, 3300, 0, 100);
    #endif
    ESP_LOGI(TAG, "Light %s --> %d", Light.c_str(), value);
    Light = String(value);
    lighttime = millis();
    updateDomoticzLightSensor(LUX_IDX, value);
  }
}

#endif  // no FIFO
#endif  // not LS_NONE


/*

  //https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#check-the-battery-voltage
  // About calibration

WiFi connected, IP address: 192.168.0.162
[  5135][I][main.cpp:103] setup(): [MAIN] setup completed.
[  5876][I][main.cpp:87] operator()(): [WEB] Index page requested.
[ 10986][I][main.cpp:87] operator()(): [WEB] Index page requested.
[ 12282][I][hardware.cpp:229] readLight(): [HDW] Reading light sensor.
[ 12282][D][esp32-hal-adc.c:190] __analogReadMilliVolts(): eFuse Two Point: Supported
[ 12285][I][esp32-hal-adc.c:232] __analogReadMilliVolts(): ADC1: Characterized using Two Point Value: 0

void readLight(void) {
  //Light:	476 (211 mV)
  int value = analogRead(LIGHT_PIN);
  uint32_t mvolt = analogReadMilliVolts(LIGHT_PIN);
  String light = String(value) + String(" (");
  light += String(mvolt) + String(" mV)");
  Serial.printf("Light %s --> %s\n", Light.c_str(), light.c_str());
  Light = light;
}
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
