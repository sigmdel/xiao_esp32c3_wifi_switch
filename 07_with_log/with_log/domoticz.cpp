
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "logging.h"
#include "domoticz.h"
#include "domoticz_data.h"

// based on BasicHttpClient.ino
// https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/examples/BasicHttpClient/BasicHttpClient.ino

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

/*
michel@hp:~$ curl "192.168.1.22:8080/json.htm?type=command&param=switchlight&idx=195&switchcmd=Toggle" -w "\n"
{
	"status" : "OK",
	"title" : "SwitchLight"
}

Domoticz will execte

michel@hp:~$ curl "192.168.1.22:8080/json.htm?type=command&param=switchlight&idx=195&switchcmd=Off" -w "\n"
{
	"status" : "OK",
	"title" : "SwitchLight"
}

michel@hp:~$ curl "192.168.1.22:8080/json.htm?type=command&param=switchlight&idx=195&switchcmd=On" -w "\n"
{
	"status" : "OK",
	"title" : "SwitchLight"
}

michel@hp:~$ curl "192.168.1.22:8080/json.htm?type=command&param=switchlight&idx=295&switchcmd=On" -w "\n"
{
	"status" : "ERR"
}


michel@hp:~$ curl "192.168.1.22:8080/json.htm?type=devices&rid=195" -w "\n"
{
	"ActTime" : 1678747700,
	"AstrTwilightEnd" : "21:01",
	"AstrTwilightStart" : "05:55",
	"CivTwilightEnd" : "19:51",
	"CivTwilightStart" : "07:05",
	"DayLength" : "11:46",
	"NautTwilightEnd" : "20:26",
	"NautTwilightStart" : "06:30",
	"ServerTime" : "2023-03-13 19:48:20",
	"SunAtSouth" : "13:28",
	"Sunrise" : "07:35",
	"Sunset" : "19:21",
	"app_version" : "2022.1",
	"result" :
	[
		{
			"AddjMulti" : 1.0,
			"AddjMulti2" : 1.0,
			"AddjValue" : 0.0,
			"AddjValue2" : 0.0,
			"BatteryLevel" : 255,
			"CustomImage" : 0,
			"Data" : "Off",
			"Description" : "",
			"DimmerType" : "none",
			"Favorite" : 0,
			"HardwareDisabled" : false,
			"HardwareID" : 2,
			"HardwareName" : "Virtual",
			"HardwareType" : "Dummy (Does nothing, use for virtual switches only)",
			"HardwareTypeVal" : 15,
			"HaveDimmer" : true,
			"HaveGroupCmd" : true,
			"HaveTimeout" : false,
			"ID" : "00014113",
			"Image" : "Light",
			"IsSubDevice" : false,
			"LastUpdate" : "2023-03-13 19:41:38",
			"Level" : 0,
			"LevelInt" : 0,
			"MaxDimLevel" : 100,
			"Name" : "Tasmota ESP32C3",
			"Notifications" : "false",
			"PlanID" : "0",
			"PlanIDs" :
			[
				0
			],
			"Protected" : false,
			"ShowNotifications" : true,
			"SignalLevel" : "-",
			"Status" : "Off",
			"StrParam1" : "",
			"StrParam2" : "",
			"SubType" : "Switch",
			"SwitchType" : "On/Off",
			"SwitchTypeVal" : 0,
			"Timers" : "false",
			"Type" : "Light/Switch",
			"TypeImg" : "lightbulb",
			"Unit" : 1,
			"Used" : 1,
			"UsedByCamera" : false,
			"XOffset" : "0",
			"YOffset" : "0",
			"idx" : "195"
		}
	],
	"status" : "OK",
	"title" : "Devices"
}
*/
