#pragma once

#include "logging.h" // for log levels

// default configuration

#define HOSTNAME         "KitchenLight"

//--- Syslog address
#define SYSLOG_HOST      "192.168.1.22"  // must be IPv4 address
#define SYSLOG_PORT      514

//--- MQTT broker        // not yet implemented
#define MQTT_BROKER      "192.168.1.22"
#define MQTT_PORT        1883
#define MQTT_USER        ""
#define MQTT_PWD         ""
#define MQTT_BUFFER_SIZE 768;

//--- Time intervals
#define SENSOR_DELAY 120000  // minimum time (in ms) between updates of the sensor data
#define POLL_TIME    25      // time (in ms) between polling of hardware,
                             // 50 ms probably fast enough

// *** Log levels ***
#define LOG_LEVEL_UART    LOG_DEBUG
#define LOG_LEVEL_SYSLOG  LOG_INFO
#define LOG_LEVEL_WEB     LOG_INFO
#define LOG_LEVEL_MQTT    LOG_ERR     // not yet implemented
