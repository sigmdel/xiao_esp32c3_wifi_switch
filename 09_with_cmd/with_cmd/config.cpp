#include <Arduino.h>
#include <IPAddress.h>
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

void defaultStaticStaIp(void) {
  IPAddress ip;
  if (ip.fromString(STA_STATIC_IP))
    config.staStaticIP = ip;
  else
    config.staStaticIP = 0;
  if (ip.fromString(STA_NETMASK))
    config.staNetmask = ip;
  else
    config.staNetmask = 0;
  if (ip.fromString(STA_GATEWAY))
    config.staGateway = ip;
  else
    config.staGateway = 0;
}

void defaultDmtz(void) {
  strlcpy(config.dmtzHost, DMTZ_HOST, HOST_SZ);
  config.dmtzPort = DMTZ_PORT;
  strlcpy(config.dmtzUser, DMTZ_USER, USER_SZ);
  strlcpy(config.dmtzPswd, DMTZ_PSWD, PSWD_SZ);
}

void defaultIdx(void) {
  config.dmtzSwitchIdx = DMTZ_SWITCH_IDX;
  config.dmtzTHSIdx = DMTZ_THS_IDX;
  config.dmtzLSIdx = DMTZ_LS_IDX;
}

void defaultLogLevels(void) {
  config.logLevelUart   = LOG_LEVEL_UART;
  config.logLevelSyslog = LOG_LEVEL_SYSLOG;
  config.logLevelWeb = LOG_LEVEL_WEB;
}

void defaultMqtt(void) {
  strlcpy(config.mqttHost, MQTT_HOST, HOST_SZ);
  config.mqttPort = MQTT_PORT;
  strlcpy(config.mqttUser, MQTT_USER, USER_SZ);
  strlcpy(config.mqttPswd, MQTT_PSWD, PSWD_SZ);
  config.mqttBufferSize = MQTT_BUFFER_SIZE;
}

void defaultSyslog(void) {
  //strlcpy(config.syslogHost, SYSLOG_HOST, IP_SZ);
  IPAddress ipa;
  if (ipa.fromString(SYSLOG_HOST))
    config.syslogIP = ipa;
  else
    config.syslogIP = 0;
  config.syslogPort = SYSLOG_PORT;
}

void defaultTimes(void) {
  config.dmtzReqTimeout = DMTZ_TIMEOUT;
  config.hdwPollTime = HDW_POLL_TIME;
  config.sensorUpdtTime = SENSOR_UPDT_TIME;
}

void useDefaultConfig(void) {
  memset(&config, 0x00, sizeof(config_t));
  config.magic = CONFIG_MAGIC;
  config.version = CONFIG_VERSION;

// -- start of user settings
  strlcpy(config.hostname, HOSTNAME, HOSTNAME_SZ);
  defaultStaticStaIp();
  defaultSyslog();
  defaultDmtz();
  defaultIdx();
  defaultMqtt();
  defaultTimes();
  defaultLogLevels();
// end of user settings --

  config.checksum = getConfigHash();
  addToLogP(LOG_INFO, TAG_CONFIG, PSTR("Using default configuration"));
}

void loadConfig(void) {
  useDefaultConfig();
  addToLogPf(LOG_INFO, TAG_CONFIG, PSTR("Config version %d, size %d"), config.version, sizeof(config_t));
}
