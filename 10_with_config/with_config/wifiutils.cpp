#include <Arduino.h>
#include <WiFi.h>
#include "logging.h"
#include "config.h"
#include "wifiutils.hpp"


void wifiLogStatus(void) {
  if (WiFi.isConnected())
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Connected to WiFi access point %s as %s"),
        WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  else
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Not connected a WiFi access point"));
  if (config.staStaticIP)
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Static station IP %s, gateway %s and netmask %s"),
      IPAddress(config.staStaticIP).toString().c_str(),
      IPAddress(config.staGateway).toString().c_str(),
      IPAddress(config.staNetmask).toString().c_str());
  else
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Using DHCP obtained IP address"));
}

//  [ ] BUG: missing static ip in wifiConnect
//           refactor wifiConnect and wifiReconnect

void wifiConnect(String ssid, String pswd) {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
    while (WiFi.isConnected()) delay(10);
  }
  WiFi.setHostname(config.hostname);
  WiFi.setAutoReconnect(true);

  //  Note: Check if WiFi.begin("SSID", "") is same as WiFi.begin("SSID")

  /*
  if (ssid.isEmpty())
    WiFi.begin();
  else if (pswd.isEmpty())
    WiFi.begin(ssid.c_str(), nullptr);
  else */
    WiFi.begin(ssid.c_str(), pswd.c_str());
  addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Attempting to connect to %s: (password: %s)"), ssid.c_str(), pswd.c_str());
  //addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Attempting to connect to %s"), ssid.c_str());
}

// [ ] TODO: should not set a static IP address without first checking if that address is on the Wi-Fi subnet
//  otherwise this may result in a "lost" device if staip is done from mqtt or webc

void wifiReconnect(void) {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config.hostname);
  if (config.staStaticIP) {
    if (WiFi.config(IPAddress(config.staStaticIP), IPAddress(config.staGateway), IPAddress(config.staNetmask)))
     addToLogP(LOG_INFO, TAG_WIFI, PSTR("Static IP address set"));
    else
     addToLogP(LOG_ERR, TAG_WIFI, PSTR("Failed to set static IP address"));
  } else {
     // See How to clear static IP configuration and start DHCP
     // @ https://stackoverflow.com/questions/40069654/how-to-clear-static-ip-configuration-and-start-dhcp
     WiFi.config(IPAddress((uint32_t) 0), IPAddress((uint32_t) 0), IPAddress((uint32_t) 0));
     addToLogP(LOG_INFO, TAG_WIFI, PSTR("Using dynamic IP address"));
  }
  WiFi.setAutoReconnect(true);
  WiFi.reconnect();
  addToLogP(LOG_INFO, TAG_SYSTEM, PSTR("Attempting to connect to last used Wi-Fi network"));
}

void wifiDisconnect(void) {
  if (WiFi.isConnected()) {
    WiFi.disconnect(); //true, true);
    while (WiFi.isConnected())
      delay(10);
  }
  addToLogP(LOG_DEBUG, TAG_WIFI, PSTR("WiFi disconnected"));
}
