#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "logging.h"
#include "html.h"
#include "hardware.h"
#include "webserver.h"

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
  return String(); // empty string
}

void webserversetup(void) {
  addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("Adding HTTP request handlers"));
  // Setup async web browser

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    addToLogP(LOG_INFO, TAG_WEBSERVER, PSTR("GET /"));
    request->send_P(200, "text/html", html_index, processor);
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
