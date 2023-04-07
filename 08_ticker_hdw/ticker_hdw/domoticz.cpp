
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "logging.h"
#include "domoticz.h"
#include "domoticz_data.h"

#define RING_SIZE 8

String urlRing[RING_SIZE];
uint8_t urHead = 0;
uint8_t urTail = 0;
uint8_t urCount = 0;

// Adds the given url to the URL circular buffer urlRing[]
// This always succeeds, the oldest url in the queue will be
// deleted if need be.
void addToRing(String url) {
  urlRing[urHead] = url;
  urHead = (urHead + 1) % RING_SIZE; // Advance urHead to the next slot in the ring buffer
  urCount++;                         // Increment the urCount of requests yet to be sent out
  if (urCount > RING_SIZE) {         // If more requests to be sent out than the size of the buffer
    urTail = urHead;                 // then move the urTail to the oldest message in the buffer
    urCount = RING_SIZE;             // Can't send out more messanges than the total number in the buffer
    addToLogP(LOG_WARNING, TAG_DOMOTICZ, PSTR("Oldest queued request removed"));
  }
  //addToLogPf(LOG_DEBUG, TAG_DOMOTICZ, PSTR("urlRing count = %d"), urCount);  // let's see if ring buffer is used at all!
}
// see sendRequest() for the other end of this

bool sendHttpRequest(String url) {
  if (WiFi.status() != WL_CONNECTED) {
    addToLogP(LOG_ERR, TAG_DOMOTICZ, PSTR("Domoticz update failed: Wi-Fi not connected"));
    return false;
  }
  addToLogPf(LOG_DEBUG, TAG_DOMOTICZ, PSTR("HTTP request URL: %s"), url.c_str());
  HTTPClient http;
  http.setConnectTimeout(CONNECT_TIMEOUT);
  bool ok =http.begin(url.c_str()); // returns false when HTTPClient::beginInternal finds syntax error in url
  if (!ok) {
    http.end();
    addToLogP(LOG_ERR, TAG_DOMOTICZ, PSTR("Domoticz update failed: url invalid"));
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
    addToLogP(LOG_INFO, TAG_DOMOTICZ, PSTR("Domoticz updated"));
    return true;
  } else if (httpResponseCode == HTTP_CODE_OK) {
    addToLogPf(LOG_ERR, TAG_DOMOTICZ, PSTR("Domoticz update failed with response: %s"), payload.c_str());
    return false;
  }
  addToLogPf(LOG_ERR, TAG_DOMOTICZ, PSTR("Domoticz update failed with HTTP code: %d"), httpResponseCode);
  return false;
}

int sendRequest(void) {
  if (urCount < 1)
    return 0;
  // Send the oldest request in the ring buffer
  // Ignore the result, errors or success is logged
  sendHttpRequest(urlRing[urTail]);
  // remove the log message from the ring buffer
  urTail = (urTail + 1) % RING_SIZE;   // move the urTail to the next message to send
  urCount--;                        // reduce the urCount of message to be sent
  return 1;
}

String startUrl(int idx) {
  /*
  IPAddress ip;
  if (!WiFi.hostByName(config.domoticzHost, ip)) {   // could try "domus.local" etc
    addToLogP(LOG_ERR, TAG_DOMOTICZ, PSTR("Domoticz not found"));
    return false;
  }
  */
  String url = "http://";
  url += DOMOTICZ_URL;
  url += "/json.htm?type=command&param=udevice&idx=";   // only update the status, do not ask Domoticz to perform action
  url += idx;
  url += "&nvalue=";
  return url;
}

void updateDomoticzSwitch(int idx, int value) {
  String url = startUrl(idx);
  url += value;
  addToRing(url);
}

void updateDomoticzLightSensor(int idx, int value) {
  String url = startUrl(idx);
  url += "0&svalue=";
  url += value;
  addToRing(url);
}

void updateDomoticzTemperatureHumiditySensor(int idx, float value1, float value2, int state) {
  String url = startUrl(idx);
  url += "0&svalue=";
  url += String(value1, 1);
  url += ";";
  url += String(value2, 0);
  url += ";";
  url += state;
  addToRing(url);
}
