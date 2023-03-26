// Main module of the basic_wifi_switch PlatformIO/Arduino sketch
// Copyright: see basic_wifi_switch.ino
// Libraries: see basic_wifi_switch.ino

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
  if (var == "INFO") return String("Using AsyncWebServer and a content refresh meta tag");
  return String(); // empty string
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
    delay(100);
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
    request->send_P(200, "text/html", html_index, processor); // updates the client making the request only
  });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "Domoticz command on");
    setLed(1);
    request->send(200, "text/plain", "Domoticz device on");
  });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP_LOGI(TAG2, "Domoticz command off");
    setLed(0);
    request->send(202, "text/plain", "done");
  });
  server.onNotFound([] (AsyncWebServerRequest *request) {
    ESP_LOGI(TAG2, "URI not found.");
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
