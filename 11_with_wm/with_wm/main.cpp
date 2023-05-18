// Main module of the with_wm PlatformIO/Arduino sketch
// Copyright: see with_wm.ino
// Libraries: see with_wm.ino

#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>         // not needed in Arduino IDE
#include <WiFi.h>
#include "version.h"
#include "logging.h"
#include "config.h"
#include "hardware.h"
#include "wifiutils.hpp"
#include "webserver.h"
#include "domoticz.h"
#include "commands.hpp"

#ifdef NO_TESTS
  // remove all test modules
#endif


/* It seems that the radios are turned off before the shutdownHandler is called!

void shutdownHandler(void) {
  // use esp_register_shutdown_handler in setup()
  // see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv429esp_register_shutdown_handler18shutdown_handler_t
  Serial.println("ShutdownHandler");
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Restarting in 1 second"));   // shows up in serial log but not web console!!
  if (configAutoSave)
    saveConfig(); // save configuration if changed
  flushLog();
  // esp_restart() will be executed next;
}
*/

void espRestart(void) {
  if (configAutoSave)
    saveConfig(); // save configuration if changed
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Restarting in 1 second"));
  flushLog();
  delay(1000);
  esp_restart();
}

String inputString;
boolean stringComplete = false;

void inputModule() {
   if (stringComplete) {
    doCommand(FROM_UART, inputString);
    inputString = "";
    stringComplete = false;
  }
  while (Serial.available() && !stringComplete) {
    char inChar = (char)Serial.read();
    if ((inChar == '\b') && (inputString.length() > 0)) {
      inputString.remove(inputString.length()-1);
      // and overwrite it with space on serial monitor
      Serial.write(" \b");  // not needed in Web console
    } else if (inChar == '\n') {
      stringComplete = true;
    } else
      inputString += inChar;
  }
}

bool accessPointUp = false;
bool wifiConnected = false;
unsigned long connectiontime = 0;

// Logs changes if Wi-Fi connected or disconnected
void WiFiModule(void) {
  if (wifiConnected != WiFi.isConnected()) {
    // WiFi status has changed
    int seconds = (int) ((millis() - connectiontime)/1000);
    connectiontime = millis();
    wifiConnected = !wifiConnected;
    if (wifiConnected) {
      addToLogPf(LOG_INFO, TAG_WIFI, PSTR("WiFi connected to %s as %s after %d seconds disconnected."),
        WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), seconds);
      if (accessPointUp) {
        stopAp();
        accessPointUp = false;
        //addToLogP(LOG_INFO, TAG_WIFI, PSTR("Closing access point"));
      }
    } else
      addToLogPf(LOG_INFO, TAG_WIFI, PSTR("WiFi disconnected after %d seconds connected"), seconds);
  }
  if (!wifiConnected) {
    if (!accessPointUp && (millis() - connectiontime > config.apDelayTime)) {
      //addToLogP(LOG_INFO, TAG_WIFI, PSTR("WiFi not connected for more than config.apDelayTime, starting access point on xxxxx"));
      accessPointUp = true;
      startAp();
    }
  }
}

void setup() {
  addToLogPf(LOG_INFO, TAG_SYSTEM, PSTR("Firmware version %s"), FirmwareVersionToString().c_str());
  // need to add firmware version
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Starting setup()"));

  Serial.begin();   // ESP_LOGx use Serial, so a 2 second delay
  delay(2000);      // should be sufficient for USB serial to be up

  loadConfig();

  wifiConnect();
  webserversetup();
  initHardware();
}

void loop() {
  sendRequest();
  sendLog();
  WiFiModule();
  inputModule();
}
