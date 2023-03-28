// Main module of the websocket_update PlatformIO/Arduino sketch
// Copyright: see websocket_update.ino
// Libraries: see websocket_update.ino

/*
References:

ESP32 WebSocket Server: Control Outputs (Arduino IDE)
  by Random Nerd Tutorial
  https://randomnerdtutorials.com/esp32-websocket-server-arduino/

ESP32 Async HTTP web server: websockets introduction
  by techtutorialsx
  https://techtutorialsx.com/2018/08/14/esp32-async-http-web-server-websockets-introduction/
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
  if (var == "INFO") return String("Using AsyncWebServer and websocket");
  return String(); // empty string
}

// Webserver instance using default HTTP port 80
AsyncWebServer server(80);

// Create an websocket
AsyncWebSocket ws("/ws");

// Function used by hardware3.cpp to update all client web pages
void updateAllClients(void) {
  ws.printfAll("%s\n%s\n%s\n%s", ledStatus.c_str(), Temperature.c_str(), Humidity.c_str(), Light.c_str());
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      toggleLed();
    }
  }
}

// Websocket event handler
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      ESP_LOGI(TAG2, "websocket client connected");
      break;
    case WS_EVT_DISCONNECT:
      ESP_LOGI(TAG2, "websocket client #%u disconnected", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

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

  //
  ws.onEvent(onWsEvent);

  // register the websocket
  server.addHandler(&ws);

  // Start async web browser
  server.begin();

  initHardware();

  ESP_LOGI(TAG, "setup completed.");
}

void loop() {
  checkHardware();
  ws.cleanupClients();
}
