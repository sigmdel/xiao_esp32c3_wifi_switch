// Main module of the ajax_update PlatformIO/Arduino sketch
// Copyright: see ajax_update.ino
// Libraries: see ajax_update.ino

/*
References:

AJAX Introduction
  By W3 Schools
  https://www.w3schools.com/xml/ajax_intro.asp

ESP32/ESP8266: Control Outputs with Web Server and a Physical Button Simultaneously
  By Random Nerd Tutorial (Rui Santos)
  https://randomnerdtutorials.com/esp32-esp8266-web-server-physical-button/

AJAX with ESP8266: Dynamic Web Page Update Without Reloading
  by Debasis Parida
  https://circuitdigest.com/microcontroller-projects/ajax-with-esp8266-dynamic-web-page-update-without-reloading
*/

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "html.h"
#include "secrets.h"
#include "hardware.h"

// logging tags
#define TAG "MAIN"
#define TAG2 "WEB"

// Values to be displayed in Web page
String ledStatus = "OFF";
String Temperature = "21.8";
String Humidity = "38.9";
String Light = "51";

// Web server template substitution function
String processor(const String& var){
  if (var == "TITLE") return String("XIAO ESP32C3 WEB SERVER");
  if (var == "DEVICENAME") return String("Kitchen Light");
  if (var == "TEMPERATURE") return Temperature;
  if (var == "HUMIDITY") return Humidity;
  if (var == "LIGHT") return Light;
  if (var == "LEDSTATUS") return ledStatus;
  if (var == "INFO") return String("Using AsyncWebServer and AJAX based Web page updates");
  return String(); // empty string
}

String makeResponse(void) {
  String resp = ledStatus;
  resp += "\n";
  resp += Temperature;
  resp += "\n";
  resp += Humidity;
  resp += "\n";
  resp += Light;
  return resp;
}

// Webserver instance using default HTTP port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin();   // ESP_LOGx use Serial, so a 2 second delay
  delay(2000);      // should be sufficient for USB serial to be up

  // Connect to the Wi-Fi network, credentials in "secrets.h"
  ESP_LOGI(TAG, "Connecting to %s", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection");
  }

  // Print local IP address
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Setup async web browser
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "Index page requested.");
    request->send_P(200, "text/html", html_index, processor);
  });
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "Web button pressed.");
    toggleLed();
    String resp = makeResponse();
    request->send(200, "text/plain", resp.c_str());  // updates the client making the request only
  });
  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "/state requested");
    String resp = makeResponse();
    request->send(200, "text/plain", resp.c_str()); // updates the client making the request only
  });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "Domoticz command on");
    setLed(1);
    request->send(200, "text/plain", "OK");
  });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "Domoticz command off");
    setLed(0);
    request->send(200, "text/plain", "OK");
  });
  server.onNotFound([] (AsyncWebServerRequest *request) {
    ESP_LOGI(TAG2, "URI \"%s\" from \"%s:%d\" not found.", request->url().c_str(),
      request->client()->remoteIP().toString().c_str(), request->client()->remotePort());
    request->send_P(404, "text/html", html_404, processor);
  });

  // Start async web browser
  server.begin();

  initHardware();

  ESP_LOGI(TAG, "setup completed.");
}

void loop() {
  checkHardware();
}
