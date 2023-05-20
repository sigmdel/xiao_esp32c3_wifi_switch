#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "AsyncElegantOTA.h"
#include "logging.h"
#include "config.h"
#include "html.h"
#include "hardware.h"
#include "webserver.h"
#include "commands.hpp"

// Values to be displayed in Web page with initial values
String RelayState = "OFF";
String Temperature = "21.8";
String Humidity = "38.9";
String Brightness = "51";

extern void espRestart(void);

// Webserver instance using default HTTP port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Web server template substitution function
String processor(const String& var){
  addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("Processing %s"), var.c_str());
  if (var == "TITLE") return String("XIAO ESP32C3 WEB SERVER");
  if (var == "DEVICENAME") return String(config.devname);
  if (var == "TEMPERATURE") return Temperature;
  if (var == "HUMIDITY") return Humidity;
  if (var == "BRIGHTNESS") return Brightness;
  if (var == "RELAYSTATE") return RelayState;
  if (var == "LOG") return logHistory();
  /*  // NOTE:   for debugging size of log {
    String test = logHistory();
    Serial.printf("logHistory.length: %d, starts with: %s\n", test.length(), test.substring(0, 64).c_str());
    return test;
  } */
  if (var == "INFO") return String("Using AsyncWebServer, AJAX and Server-Sent Events (SSE)");
  return String(); // empty string
}

// Web server template substitution function for access point index page
String processor2(const String& var){
  addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("Processing %s"), var.c_str());
  if (var == "TITLE") return String("XIAO ESP32C3 WEB SERVER");
  if (var == "DEVICENAME") {
    String devstring(config.devname);
    devstring += "<br/>";
    devstring += "Access Point";
    return devstring;
  }
  if (var == "SSID") return String(config.wifiSsid);
  if ((var == "PASS") && strlen(config.wifiPswd)) return String("***********");
  if (var == "STAIP") return IPAddress(config.staStaticIP).toString();
  if (var == "GATE") return IPAddress(config.staGateway).toString();
  if (var == "MASK") return IPAddress(config.staNetmask).toString();
  return String(); // empty string
}


void webserversetup(void) {
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Adding HTTP request handlers"));

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /"));
    if (accessPointUp)
      request->send_P(200, "text/html", html_wm, processor2);
    else
      request->send_P(200, "text/html", html_index, processor);
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    // log in to index page even if using access point - can control light with web interface
    // but .......
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /index.html"));
    request->send_P(200, "text/html", html_index, processor);
  });

/*
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    //addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("POST / params=%d"), request->params());
    addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("POST / request with %d params"), request->params());
    if (request->params() == 1) {
      //addToLogP(LOG_DEBUG, TAG_WEBSERVER, PSTR("getting param"));
      AsyncWebParameter* aParam = request->getParam(0, true, false);
      addToLogP(LOG_DEBUG, TAG_WEBSERVER, PSTR("got param"));
      delay(5);
      if ((aParam) && (aParam->name().equals("cmd")) && (aParam->value().length() > 0)) {
        addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Web console command: \"%s\""), aParam->value().c_str());
        delay(5);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("POST / with %d params"), request->params());
    delay(5);
    if (request->params() > 0) {
      addToLogP(LOG_DEBUG, TAG_WEBSERVER, PSTR("Getting aParam"));
      delay(5);
      AsyncWebParameter* aParam = request->getParam(0, true, false);
      addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("aParam: %s:%s"), aParam->name().c_str(), aParam->value().c_str());
      delay(10);
      //if ((aParam) && (aParam->name().equals("cmd")) && (aParam->value().length() > 0)) {
      //  addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Web console command: \"%s\""), aParam->value().c_str());
       // }
    }
    request->send(200, "text/plain", "OK");
  });

// BUG is server.on("/", HTTP_POST... ) buggy with ESP32C3 ??, device crashes when getting param

   but seems ok if using
     if (request->hasParam("xxx", true))
        String xxxdata = request->getParam("xxx", true)->value();
   see server.on("/creds", HTTP_POST.... below

*/

  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("GET /cmd with %d params"), request->params());
    if (request->params() == 1) {
      AsyncWebParameter* aParam = request->getParam(0);
      if ((aParam) && (aParam->name().equals("cmd")) && (aParam->value().length() > 0)) {
        doCommand(FROM_WEBC, aParam->value());
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /toggle"));
    toggleRelay();
    request->send(200, "text/plain", "OK");
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /on"));
    setRelay(1);
    request->send(200, "text/plain", "OK");
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /off"));
    setRelay(0);
    request->send(200, "text/plain", "OK");
  });

  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /log"));
    request->send_P(200, "text/html", html_console, processor);
  });

  server.on("/rst", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /rst"));
    request->send(200, "text/plain", "Restart device");
    espRestart();
  });

  server.on("/creds", HTTP_POST, [](AsyncWebServerRequest *request) {
    #define PARAM_INPUT_1 "ssid"
    #define PARAM_INPUT_2 "pass"
    #define PARAM_INPUT_3 "staip"
    #define PARAM_INPUT_4 "gateway"
    #define PARAM_INPUT_5 "mask"
    String wifi_ssid = "";
    String wifi_pass = "";
    IPAddress ipa;
    IPAddress gateway;
    IPAddress mask;

    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("POST /creds"));
    if (!accessPointUp) {
      request->send_P(404, "text/html", html_404, processor);
      return;
    }
    bool bad = true;
    if (request->hasParam(PARAM_INPUT_1, true))
      wifi_ssid = request->getParam(PARAM_INPUT_1, true)->value();
    if (request->hasParam(PARAM_INPUT_2, true))
      wifi_pass = request->getParam(PARAM_INPUT_2, true)->value();
    if (request->hasParam(PARAM_INPUT_3, true))
      ipa.fromString(request->getParam(PARAM_INPUT_3, true)->value());
    if (request->hasParam(PARAM_INPUT_4, true))
      gateway.fromString(request->getParam(PARAM_INPUT_4, true)->value());
    if (request->hasParam(PARAM_INPUT_5, true))
      mask.fromString(request->getParam(PARAM_INPUT_5, true)->value());

    if (wifi_ssid.isEmpty())
      addToLogP(LOG_ERR, TAG_COMMAND, PSTR("Empty SSID"));
    else if ((wifi_pass.length() > 0) && (wifi_pass.length() < 8))
      addToLogP(LOG_ERR, TAG_COMMAND, PSTR("Password too short"));
    else if (ipa && ((ipa & mask) != (gateway & mask)))
      addToLogP(LOG_ERR, TAG_COMMAND, PSTR("The station IP and gateway are not on the same subnet"));
    else
      bad = false;
    if (bad) {
      request->send_P(200, "text/html", html_wm_bad_creds, processor);
      return;
    }

    request->send_P(200, "text/html", html_wm_connect, processor);

    strlcpy(config.wifiSsid, wifi_ssid.c_str(), HOST_SZ);
    strlcpy(config.wifiPswd, wifi_pass.c_str(), PSWD_SZ);

    if (ipa) {
      config.staStaticIP = ipa;
      config.staGateway = gateway;
      config.staNetmask = mask;
    }
    espRestart();

  });

  server.onNotFound([] (AsyncWebServerRequest *request) {
    addToLogPf(LOG_INFO, TAG_WEBSERVER, PSTR( "URI \"%s\" from \"%s:%d\" not found."), request->url().c_str(),
      request->client()->remoteIP().toString().c_str(), request->client()->remotePort());
    request->send_P(404, "text/html", html_404, processor);
  });

  // add SSE handler
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Adding SSE handler"));
  server.addHandler(&events);

  // Starting Async OTA web server AFTER all the server.on requests registered
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Add OTA server at /update"));
  AsyncElegantOTA.begin(&server);

  // Start async web browser
  server.begin();
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Web server started"));
}
