// wifiutils.cpp

#include <Arduino.h>
#include <WiFi.h>
#include "logging.h"
#include "config.h"
#include "wifiutils.hpp"

void wifiLogStatus(void) {
  if (WiFi.isConnected())
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Connected to WiFi network %s as %s"),
        WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  else
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Not connected to a WiFi network"));
}

void wifiConnect() {
  if (!strlen(config.wifiSsid)) {
    addToLogP(LOG_ERR, TAG_WIFI, PSTR("Wi-Fi network name must be specified"));
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config.hostname);
  WiFi.setAutoReconnect(true);

  bool clearConfig = true;
  if (config.staStaticIP) {
    if (WiFi.config(IPAddress(config.staStaticIP), IPAddress(config.staGateway), IPAddress(config.staNetmask))) {
      addToLogP(LOG_INFO, TAG_WIFI, PSTR("Static IP address set"));
      clearConfig = false;
    }
    else
      addToLogP(LOG_ERR, TAG_WIFI, PSTR("Failed to set static IP address"));
  }
  if (clearConfig) {
     // See How to clear static IP configuration and start DHCP
     // @ https://stackoverflow.com/questions/40069654/how-to-clear-static-ip-configuration-and-start-dhcp
     WiFi.config(IPAddress((uint32_t) 0), IPAddress((uint32_t) 0), IPAddress((uint32_t) 0));
     addToLogP(LOG_INFO, TAG_WIFI, PSTR("Using dynamic IP address"));
  }
  WiFi.begin(config.wifiSsid, config.wifiPswd);
  //addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Attempting to connect to %s: (password: %s)"), config.wifiSsid, config.wifiPswd);
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Attempting to connect to %s"), config.wifiSsid);
}

