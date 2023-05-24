// wifiutils.cpp

#include <Arduino.h>
#include <WiFi.h>
#include "logging.h"
#include "config.h"
#include "wifiutils.hpp"

void startAp(void) {
  WiFi.enableAP(true);
  String apName(config.hostname);
  if (strlen(config.apSuffix)) {
    apName += "-";
    apName += config.apSuffix;
  }
  apName.toUpperCase();
  WiFi.softAPsetHostname(apName.c_str());
  if (!WiFi.softAP(apName.c_str(), config.apPswd)) {
    addToLogP(LOG_ERR, TAG_WIFI, PSTR("Could not create Wi-Fi network access point"));
    return;
  }
  delay(100); // time for AP to come up
  if (config.apIP) {
    IPAddress ap(config.apIP);
    //addToLogPf(LOG_DEBUG, TAG_WIFI, PSTR("Setting AP IP at %s"), ap.toString().c_str());
    WiFi.softAPConfig(ap, ap, IPAddress(config.apMask));
  }
  //addToLogPf(LOG_INFO, TAG_WIFI, PSTR("Wi-Fi network access point %s created at %s"), apName.c_str(), WiFi.softAPIP().toString().c_str());
  addToLogPf(LOG_INFO, TAG_WIFI, PSTR("Wi-Fi network access point %s created at %s"), WiFi.softAPgetHostname(), WiFi.softAPIP().toString().c_str());
}

void stopAp(void) {
  WiFi.enableAP(false);
  addToLogP(LOG_INFO, TAG_WIFI, PSTR("Connected to the Wi-Fi network, closing the access point."));
}

bool accessPointUp = false;
bool wifiConnected = false;
unsigned long connectiontime = 0;

// Logs changes if Wi-Fi connected or disconnected
void wifiLoop(void) {
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


void wifiDisconnect(void) {
  if (WiFi.isConnected()) {
    addToLogP(LOG_INFO, TAG_WIFI, PSTR("Disconnecting from Wi-Fi network"));
    flushLog();
    WiFi.disconnect();
    while (WiFi.isConnected()) delay(10);
  }
}


void wifiLogStatus(void) {
  if (WiFi.isConnected())
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Connected to WiFi network %s as %s"),
        WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  else if (accessPointUp)
    addToLogPf(LOG_INFO, TAG_WIFI, PSTR("Not connected, Wi-Fi access point %s created at %s"), WiFi.softAPgetHostname(), WiFi.softAPIP().toString().c_str());
  else
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Not connected to a Wi-Fi network"));
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config.hostname);
  WiFi.setAutoReconnect(true);
  // Above must be done no matter if there is a valid ssid or not.
  // Starting with the followin test and exiting before setting mode etc
  //  causes "invalid mbox" panick reset when ssid == "".
  if (!strlen(config.wifiSsid)) {
    addToLogP(LOG_ERR, TAG_WIFI, PSTR("Wi-Fi network name must be specified"));
    return;
  }

  bool clearConfig = true;
  if (config.staStaticIP) {
    // No checking that the static IP address and gateway are on the same subnet
    // This is done in commands.cpp
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
