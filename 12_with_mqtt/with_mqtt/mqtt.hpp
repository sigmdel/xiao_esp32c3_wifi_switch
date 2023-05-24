// mqtt.hpp

#pragma once

// Reports the Wi-FI network status to the log
void mqttLogStatus(void);

// Sets MQTT broker and callback function
void mqttClientSetup(void);

// Disconnectes form the MQTT broker
void mqttDisconnect(void);

// Calls the MQTT client loop function, so mqttLoop() must be in loop() function.
// Checks if state of the connection to the MQTT broker has changed.
//   Attempts to reconnect if 5 second interval from last attempt has expired and Wi-Fi is connected
void mqttLoop(void);

// Sends a message to the  MQTT broker to update the light switch sensor state to
//   "On" (value = 1) or "Off" (value = 0)
// Returns false if not connected to the MQTT broker
bool mqttUpdateDmtzSwitch(int idx, int value);
bool mqttUpdateDomoticzBrightnessSensor(int idx, int value);
bool mqttUpdateDomoticzTemperatureHumiditySensor(int idx, float value1, float value2, int state);

bool mqttLog(String message);
