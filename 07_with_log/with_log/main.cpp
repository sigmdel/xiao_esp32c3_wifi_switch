// Main module of the with_log PlatformIO/Arduino sketch
// Copyright: see with_log.ino
// Libraries: see with_log.ino


#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>         // not needed in Arduino IDE
#include <WiFi.h>
#include "version.h"
#include "secrets.h"
#include "logging.h"
#include "config.h"
#include "hardware.h"
#include "webserver.h"

#define TEST_LOG

#ifdef NO_TESTS
  // remove all test modules
  #undef TEST_LOG
#endif

bool wifiConnected = false;
unsigned long connectiontime = 0;

// Logs changes if Wi-Fi connected or disconnected
void WiFiModule(void) {
  if (wifiConnected != WiFi.isConnected()) {
    // WiFi status has changed
    int seconds = (int) ((millis() - connectiontime)/1000);
    connectiontime = millis();
    wifiConnected = !wifiConnected;
    if (wifiConnected)
      addToLogPf(LOG_INFO, TAG_WIFI, PSTR("WiFi connected to %s as %s after %d seconds disconnected."),
        WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), seconds);
    else
      addToLogPf(LOG_INFO, TAG_WIFI, PSTR("WiFi disconnected after %d seconds connected"), seconds);
  }
}

#ifdef TEST_LOG
// ---  testLogModule
// Adds messages to log every 1/2 second to test logging
unsigned long logtime = millis();
uint32_t logcount = 0;
void testLogModule(void) {
  if (millis() - logtime > 500) {
    addToLogf(LOG_INFO, TAG_SYSTEM, "loop messsage #%d", logcount);
    logcount++;
    logtime = millis();
  }
}
#endif

void setup() {
  addToLogPf(LOG_INFO, TAG_SYSTEM, PSTR("Firmware version %s"), FirmwareVersionToString().c_str());
  // need to add firmware version
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Starting setup()"));

  Serial.begin();   // ESP_LOGx use Serial, so a 2 second delay
  delay(2000);      // should be sufficient for USB serial to be up

  loadConfig();

  // Connect to the Wi-Fi network
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  wifiConnected = false;
  connectiontime = millis();
  #ifdef USE_SECRETS
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  addToLogPf(LOG_CRIT, TAG_SYSTEM, PSTR("Attempting to connect to Wi-Fi network %s"), WIFI_SSID);
  #else
  WiFi.begin();
  addToLogP(LOG_CRIT, TAG_SYSTEM, PSTR("Attempting to connect to last used Wi-Fi network"));
  #endif
  webserversetup();
  initHardware();

  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Completed setup(), starting loop()"));
}

void loop() {
  checkHardware();
  sendLog();
  WiFiModule();
  #ifdef TEST_LOG
  testLogModule();
  #endif
}
