// Main module of the with_wm PlatformIO/Arduino sketch
// Copyright: see with_mqtt.ino
// Libraries: see with_mqtt.ino

#include "arduino_config.h"  // done with build_flags in platformIO
#include <Arduino.h>         // not needed in Arduino IDE
#include "version.h"
#include "logging.h"
#include "config.h"
#include "hardware.h"
#include "wifiutils.hpp"
#include "webserver.h"
#include "mqtt.hpp"
#include "domoticz.h"
#include "commands.hpp"

/*
0 = saveConfig() if changed
1 = saveConfig(true) save config no matter if changes or not
2 = do not save config
3 = set dynamic IP, save config
4 = set dynamic IP, clear wifi credentials, save config
5 = restore default config then save config
6 = restore default config, then set dynamic IP, save config
7 = restore default config, then set dynamic IP, clear wifi credentials, save config
*/
void espRestart(int level = 0) {
  switch (level) {
    case 0:
      saveConfig(false);
      break;

    case 5:
      useDefaultConfig();
    case 1:
      saveConfig(true);
      break;

    case 2:
      break;

    case 6:
      useDefaultConfig();
    case 3:
      config.staStaticIP = 0;
      config.staNetmask = 0;
      config.staGateway = 0;
      saveConfig(true);
      break;

    case 7:
      useDefaultConfig();
    case 4:
      config.staStaticIP = 0;
      config.staNetmask = 0;
      config.staGateway = 0;
      config.wifiSsid[0] = '\0';
      config.wifiPswd[0] = '\0';
      saveConfig(true);
      break;
  }
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Restarting in 1 second"));
  mqttDisconnect();
  wifiDisconnect();
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

void setup() {
  addToLogPf(LOG_INFO, TAG_SYSTEM, PSTR("Firmware version %s"), FirmwareVersion().c_str());
  addToLogP(LOG_DEBUG, TAG_SYSTEM, PSTR("Starting setup()"));

#ifdef SERIAL_BAUD
  Serial.begin(SERIAL_BAUD);
#else
  Serial.begin();   // ESP_LOGx use Serial, so a 2 second delay
#endif
  delay(2000); // should be sufficient for USB serial to be up if connected
  loadConfig();
  wifiConnect();
  webserversetup();
  mqttClientSetup();
  initHardware();
  addToLogP(LOG_DEBUG, TAG_SYSTEM, PSTR("Completed setup(), starting loop()"));
}

void loop() {
  sendRequest();
  sendLog();
  wifiLoop();
  inputModule();
  mqttLoop();
}
