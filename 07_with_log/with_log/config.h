#pragma once

#include <Arduino.h>

// Maximum number of characters in string including terminating 0
//
#define HOSTNAME_SZ      65
#define URL_SZ           81

#define CONFIG_MAGIC    0x4D45    // 'M'+'D'

struct config_t {
  uint16_t magic;                 // check for valid config

  char hostname[HOSTNAME_SZ];     // up to 54 character host name
  char syslogHost[URL_SZ];        // URL of Syslog server
  uint16_t syslogPort;            // Syslog port

  uint8_t logLevelUart;
  uint8_t logLevelSyslog;
  uint8_t logLevelWeb;

  uint32_t checksum;
};

void loadConfig(void);

extern config_t config;
