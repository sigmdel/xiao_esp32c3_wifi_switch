
#include <Arduino.h>
#include <esp32-hal-log.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "domoticz.h"
#include "domoticz_data.h"

#define TAG "DMZ"  // for log messages

// based on BasicHttpClient.ino
// https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/examples/BasicHttpClient/BasicHttpClient.ino

bool sendHttpRequest(String url) {
  if (WiFi.status() != WL_CONNECTED) {
    ESP_LOGE(TAG, "Domoticz update failed: Wi-Fi not connected.");
    return false;
  }
  ESP_LOGV(TAG, "HTTP request URL: %s", url.c_str());
  HTTPClient http;
  http.setConnectTimeout(CONNECT_TIMEOUT);
  bool ok =http.begin(url.c_str()); // returns false when HTTPClient::beginInternal finds syntax error in url
  if (!ok) {
    http.end();  
    ESP_LOGE(TAG, "Domoticz update failed: url invalid.");
    return false;
  }
  int httpResponseCode = http.GET();
  String payload;
  if (httpResponseCode > 0)
    payload = http.getString();
  // Free resources
  http.end();
  // Return true if OK with information log message, else return false with an error log message
  if ( (httpResponseCode == HTTP_CODE_OK) && (payload.indexOf("\"status\" : \"OK\"") > 0) ) {
    ESP_LOGI(TAG, "Domoticz updated.");
    return true;
  } else if (httpResponseCode == HTTP_CODE_OK) {
    ESP_LOGE(TAG, "Domoticz update failed with response: %s", payload.c_str());
    return false;
  }
  ESP_LOGE(TAG,  "Domoticz update failed with HTTP code: %d", httpResponseCode);
  return false;
}

String startUrl(int idx) {
  String url = "http://";
  url += DOMOTICZ_URL;
  url += "/json.htm?type=command&param=udevice&idx=";   // only update the status, do not ask Domoticz to perform action
  url += idx;
  url += "&nvalue=";
  return url;
}

bool updateDomoticzSwitch(int idx, int value) {
  String url = startUrl(idx);
  url += value;
  return sendHttpRequest(url);
}

bool updateDomoticzLightSensor(int idx, int value) {
  String url = startUrl(idx);
  url += "0&svalue=";
  url += value;
  return sendHttpRequest(url);
}

bool updateDomoticzTemperatureHumiditySensor(int idx, float value1, float value2, int state) {
  String url = startUrl(idx);
  url += "0&svalue=";
  url += String(value1, 1);
  url += ";";
  url += String(value2, 0);
  url += ";";
  url += state;
  return sendHttpRequest(url);
}

