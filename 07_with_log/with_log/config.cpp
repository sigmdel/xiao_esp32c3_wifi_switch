#include <Arduino.h>
#include "secrets.h"
#include "config.h"
#include "user_config.h"
#include "logging.h"

config_t config;

uint32_t getConfigHash() {
  uint32_t hash = 0;
  uint8_t *bytes = (uint8_t*)&config;

  for (uint16_t i = 0; i < sizeof(config_t) - sizeof(config.checksum); i++) hash += bytes[i]*(i+1);
  return hash;
}

void useDefaultConfig(void) {
  memset(&config, 0x00, sizeof(config_t));
  config.magic = CONFIG_MAGIC;

  strlcpy(config.hostname, HOSTNAME, HOSTNAME_SZ);

  strlcpy(config.syslogHost, SYSLOG_HOST, URL_SZ);
  config.syslogPort = SYSLOG_PORT;

  config.logLevelUart   = LOG_LEVEL_UART;
  config.logLevelSyslog = LOG_LEVEL_SYSLOG;
  config.logLevelWeb = LOG_LEVEL_WEB;

  config.checksum = getConfigHash();
  addToLogP(LOG_INFO, TAG_CONFIG, PSTR("Using default configuration"));
}

void loadConfig(void) {
  useDefaultConfig();
}
