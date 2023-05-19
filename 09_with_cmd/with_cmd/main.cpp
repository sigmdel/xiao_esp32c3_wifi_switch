// Main module of the ticker_hdw PlatformIO/Arduino sketch
// Copyright: see ticker_hdw.ino
// Libraries: see ticker_hdw.ino

#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>         // not needed in Arduino IDE
#include <WiFi.h>
#include "version.h"
#include "secrets.h"
#include "logging.h"
#include "config.h"
#include "hardware.h"
#include "webserver.h"
#include "domoticz.h"
#include "commands.hpp"

#ifdef NO_TESTS
  // remove all test modules
#endif


String inputString;
boolean stringComplete = false;

void inputModule() {
  //lwdtStamp(INPUT_MODULE);

  if (stringComplete) {
    //addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Serial command: \"%s\""), inputString.c_str());
    //doCommand(FROM_UART, inputString.c_str());
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

void setup() {
  addToLogPf(LOG_INFO, TAG_SYSTEM, PSTR("Firmware version %s"), FirmwareVersionToString().c_str());
  addToLogP(LOG_DEBUG, TAG_SYSTEM, PSTR("Starting setup()"));
  Serial.begin();
  delay(2000); // should be sufficient for USB serial to be up if connected
  loadConfig();
  // Connect to the Wi-Fi network
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  wifiConnected = false;
  connectiontime = millis();
  #ifdef USE_SECRETS
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  addToLogPf(LOG_INFO, TAG_SYSTEM, PSTR("Attempting to connect to Wi-Fi network %s"), WIFI_SSID);
  #else
  WiFi.begin();
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Attempting to connect to last used Wi-Fi network"));
  #endif
  webserversetup();
  initHardware();
  addToLogP(LOG_DEBUG, TAG_SYSTEM, PSTR("Completed setup(), starting loop()"));
}

void loop() {
  sendRequest();
  sendLog();
  WiFiModule();
  inputModule();
}
