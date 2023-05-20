#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "logging.h"
#include "html.h"
#include "hardware.h"
#include "webserver.h"
#include "commands.hpp"

// Values to be displayed in Web page with initial values
String RelayState = "OFF";
String Temperature = "21.8";
String Humidity = "38.9";
String Brightness = "51";


// Webserver instance using default HTTP port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Web server template substitution function
String processor(const String& var){
  addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("Processing %s"), var.c_str());
  if (var == "TITLE") return String("XIAO ESP32C3 WEB SERVER");
  if (var == "DEVICENAME") return String("Kitchen Light");
  if (var == "TEMPERATURE") return Temperature;
  if (var == "HUMIDITY") return Humidity;
  if (var == "BRIGHTNESS") return Brightness;
  if (var == "RELAYSTATE") return RelayState;
  if (var == "LOG") return logHistory();
  if (var == "INFO") return String("Using AsyncWebServer, AJAX and Server-Sent Events (SSE)");
  // [ ] TODO - returning var instead may be the solution to % in headers - see note in ?
  //   That does not work because starting % and ending %  get stripped
  //   Trying to "%" + var + "%" causes wdt timeout (recursive substitution ?)
  //   How about return nullptr which would be interpreted as leave unchanged
  //   We need empty string return in case we want to remove the placeholder
  return String(); // empty string
}

void webserversetup(void) {
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Adding HTTP request handlers"));

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /"));
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

  server.onNotFound([] (AsyncWebServerRequest *request) {
    addToLogPf(LOG_INFO, TAG_WEBSERVER, PSTR( "URI \"%s\" from \"%s:%d\" not found."), request->url().c_str(),
      request->client()->remoteIP().toString().c_str(), request->client()->remotePort());
    request->send_P(404, "text/html", html_404, processor);
  });

  // add SSE handler
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Adding SSE handler"));
  server.addHandler(&events);

  // Start async web browser
  server.begin();
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Web server started"));
}
