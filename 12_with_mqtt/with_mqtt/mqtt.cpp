#include <Arduino.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include "ArduinoJson.h"
#include "hardware.h"
#include "logging.h"
#include "config.h"
#include "commands.hpp"
#include "mqtt.hpp"

#define MSG_SZ  441

extern bool wifiConnected;

WiFiClient mqttClient;
PubSubClient mqtt_client(mqttClient);

void mqttLogStatus(void) {
  if (!strlen(config.mqttHost))
    addToLogP(LOG_INFO, TAG_MQTT, PSTR("No MQTT broker defined"));
  else {
    String connected;
    connected = (mqtt_client.connected()) ? "Connected" : "Not connected";
    addToLogPf(LOG_INFO, TAG_MQTT, PSTR("%s to MQTT broker %s:%d"), connected.c_str(), config.mqttHost, config.mqttPort);
  }
}

void mqttSubscribe(void) {
  mqtt_client.subscribe(config.topicDmtzSub);
  String topic(config.topicCmd);
  topic.replace("%h%", config.hostname);
  mqtt_client.subscribe(topic.c_str());
}


void receivingDomoticzMQTT(String const payload) {
  DynamicJsonDocument doc(config.mqttBufferSize);
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    addToLogPf(LOG_ERR, TAG_MQTT, PSTR("deserializeJson() failed : %s with message %s"), err.c_str(), payload.substring(0, 80).c_str());
    return;
  }

  int idx = doc["idx"];  // default 0 - not valid
  if (!idx) {
    addToLogP(LOG_ERR, TAG_MQTT, PSTR("idx not found in MQTT message"));
    return;
  }

  if (idx != config.dmtzSwitchIdx)
    return;

  int status = doc["nvalue"];
  setRelay(status);
  addToLogPf(LOG_INFO, TAG_MQTT, PSTR("Relay set to %s in Domoticz"), (status) ? "ON" : "OFF");
}

// Callback function, when we receive an MQTT value on the topics
// subscribed this function is called
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  //byte* payload_copy = reinterpret_cast<byte*>(malloc(length + 1));
  char* payload_copy = reinterpret_cast<char*>(malloc(length + 1));
  if (payload_copy == NULL) {
    addToLogP(LOG_ERR, TAG_MQTT, PSTR("Can't allocate memory for MQTT payload. Message ignored"));
    return;
  }

  // Copy the payload to the new buffer
  memcpy(payload_copy, payload, length);
  // Conversion to a printable string
  payload_copy[length] = '\0';

  addToLogPf(LOG_DEBUG, TAG_MQTT, PSTR("MQTT rx [%s] %s"), topic, payload_copy);

  String sTopic(topic);
  if (sTopic.indexOf(config.topicDmtzSub) > -1)
    //receivingDomoticzMQTT((char *)payload_copy); // launch the function to treat received data
    receivingDomoticzMQTT(payload_copy); // launch the function to treat received data
  else
    doCommand(FROM_MQTT, payload_copy);

  // Free the memory
  free(payload_copy);
}



void mqttClientSetup(void) {
  addToLogPf(LOG_DEBUG, TAG_MQTT, PSTR("Setting up MQTT server: %s:%d"), config.mqttHost, config.mqttPort);
  if (!mqtt_client.setBufferSize(config.mqttBufferSize))
    addToLogPf(LOG_ERR, TAG_MQTT, PSTR("Could not allocated %d byte MQTT buffer"), config.mqttBufferSize);
  mqtt_client.setServer(config.mqttHost, config.mqttPort);
  mqtt_client.setCallback(mqttCallback);
  //mqttReconnect();
}

void mqttDisconnect(void) {
  if (mqtt_client.connected())
    mqtt_client.disconnect();
}

unsigned long lastMqttConnectAttempt = 0;

void mqttReconnect(void) {
  if ((mqtt_client.connected()) || (!wifiConnected) || (millis() - lastMqttConnectAttempt < 5000) )
    return;
  if (!strlen(config.mqttUser) || !strlen(config.mqttPswd))
    mqtt_client.connect(config.hostname);
  else
    mqtt_client.connect(config.hostname, config.mqttUser, config.mqttPswd);
  lastMqttConnectAttempt = millis();
}


bool mqttConnected = false;

void mqttLoop(void) {
  if (mqttConnected != mqtt_client.connected()) {
    // MQTT status has changed
    //int seconds = (int) ((millis() - connectiontime)/1000);
    //mqttConnectiontime = millis();
    mqttConnected = !mqttConnected;
    if (mqttConnected) {
       addToLogPf(LOG_INFO, TAG_MQTT, PSTR("Reconnected to MQTT broker %s as %s"), config.mqttHost, config.hostname);
       mqttSubscribe();
    } else
      addToLogP(LOG_INFO, TAG_MQTT, PSTR("Disconnected from MQTT broker"));
  }
  if (mqtt_client.connected()) {
    mqtt_client.loop();
    lastMqttConnectAttempt = millis();
  } else
    mqttReconnect();
}


bool mqttPublish(String payload, char* topic = NULL) {
  if (!mqtt_client.connected()) {
    mqttReconnect();
    delay(10);
  }
  if (!mqtt_client.connected()) {
    return false;
  }

  char* theTopic;
  if (topic == NULL)
    theTopic = config.topicDmtzPub;
  else
    theTopic = topic;
  addToLogPf(LOG_DEBUG, TAG_MQTT, PSTR("MQTT update message: %s"), payload.c_str());
  return mqtt_client.publish(theTopic, payload.c_str());
}

bool mqttLog(String message) {
  String topic(config.topicLog);
  topic.replace("%h%", config.hostname);
  return mqttPublish(message, (char*) topic.c_str());
}

#define MQTT_JSON "{\"idx\":%idx%, \"nvalue\":%nval%, \"svalue\":\"%sval%\", \"parse\":false}"

String startPayload(int idx, int value) {
  String payload(MQTT_JSON);
  payload.replace("%idx%", String(idx).c_str());
  payload.replace("%nval%", String(value).c_str());
  return payload;
}

bool mqttUpdateDmtzSwitch(int idx, int value) {
  String payload(startPayload(idx, value));
  payload.replace("%sval%", String("").c_str());
  return mqttPublish(payload);
}

bool mqttUpdateDomoticzBrightnessSensor(int idx, int value) {
  String payload(startPayload(idx, value));
  payload.replace("%sval%", String(value).c_str());
  return mqttPublish(payload);
}

bool mqttUpdateDomoticzTemperatureHumiditySensor(int idx, float value1, float value2, int state) {
  String payload(startPayload(idx, 0));
  String svalue(value1, 1);
  svalue += ";";
  svalue += String(value2, 0);
  svalue += ";";
  svalue += state;
  payload.replace("%sval%", svalue.c_str());
  return mqttPublish(payload);
}
