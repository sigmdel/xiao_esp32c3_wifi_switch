#pragma once

#include <Arduino.h>

// Maximum number of characters in string including terminating 0

#define HOSTNAME_SZ      49
#define HOST_SZ          81
#define USER_SZ          49
#define PSWD_SZ          49
#define IP_SZ            16  // IPv4 only

// Careful, changing any one of the above sizes will change
// the configuration image size in non volatile memory

#define CONFIG_MAGIC    0x4D45    // 'M'+'D'
#define CONFIG_VERSION  1


struct config_t {
  uint16_t magic;                 // check for valid config
  uint16_t version;               // and version number

// -- start of user settings
  char hostname[HOSTNAME_SZ];     // this device host name
  char devname[HOST_SZ];          // this device name in Web interface

  char wifiSsid[HOST_SZ];         // WiFi network name
  char wifiPswd[PSWD_SZ];         // WiFi network password

  uint32_t staStaticIP;           // Static IP, set to 0 for dhcp
  uint32_t staGateway;            //
  uint32_t staNetmask;            //
  //uint32_t staDnsIP1;           // not going beyond LAN yet
  //uint32_t staDnsIP2;           // ditto

  //char syslogHost[IP_SZ];       // Static IP of Syslog server (must be IPv4)
  uint32_t syslogIP;              // Static IP of Syslog server
  uint16_t syslogPort;            // Syslog port

  char dmtzHost[HOST_SZ];         // IP or hostname of Domoticz server
  uint16_t dmtzPort;              // Domoticz server TCP port
  char dmtzUser[USER_SZ];         // Domoticz user name
  char dmtzPswd[PSWD_SZ];         // Domoticz password

  uint16_t dmtzSwitchIdx;         // ID of virtual Domoticz switch
  uint16_t dmtzTHSIdx;            // ID of virtual Domoticz temperature and humidity sensor
  uint16_t dmtzLSIdx;             // ID of virtual Domoticz lux sensor
  uint32_t dmtzReqTimeout;        // HTTP request timeout

  char mqttHost[HOST_SZ];         // IP/hostname of MQTT broker
  uint16_t mqttPort;              // MQTT broker TCP port
  char mqttUser[USER_SZ];         // MQTT user name
  char mqttPswd[PSWD_SZ];         // MQTT password
  uint16_t mqttBufferSize;        // Size of MQTT buffer

  uint16_t hdwPollTime;           // Interval between hardware polling (ms)
  uint32_t sensorUpdtTime;        // Interval between updates of hardware values (ms)

  uint8_t logLevelUart;
  uint8_t logLevelSyslog;
  uint8_t logLevelWebc;
  uint8_t logLevelMqtt;
// end of user settings --

  uint32_t checksum;
};

void defaultNames(void);
void defaultWifiNetwork(void);
void defaultStaticStaIp(void);
void defaultDmtz(void);
void defaultIdx(void);
void defaultMqtt(void);
void defaultLogLevels(void);
void defaultStatip(void);
void defaultSyslog(void);
void defaultTimes(void);

void useDefaultConfig(void);
void loadConfig(void);
void saveConfig(bool force = false);

extern config_t config;
extern bool configAutoSave;
