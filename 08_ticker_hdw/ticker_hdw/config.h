#pragma once

#include <Arduino.h>

// Maximum number of characters in string including terminating 0
//
#define HOSTNAME_SZ      65
#define URL_SZ           81
#define IP_SZ            16  // IPv4 only

#define CONFIG_MAGIC    0x4D45    // 'M'+'D'

struct config_t {
  uint16_t magic;                 // check for valid config

  char hostname[HOSTNAME_SZ];     // up to 54 character host name
  char syslogHost[IP_SZ];         // Static IP of Syslog server (must be IPv4)
  uint16_t syslogPort;            // Syslog port

  uint16_t hdwPollTime;           // Interval between hardware polling (ms)
  uint32_t sensorUpdtTime;        // Interval between updates of hardware values (ms)

  uint8_t logLevelUart;
  uint8_t logLevelSyslog;
  uint8_t logLevelWeb;

  uint32_t checksum;
};

void loadConfig(void);

extern config_t config;
