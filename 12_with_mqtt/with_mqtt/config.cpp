#include <Arduino.h>
#include <IPAddress.h>
#include <Preferences.h>
#include "config.h"
#include "user_config.h"
#include "logging.h"

// BUG - it is possible that strlcpy could truncate !!!
//   see: https://en.wikibooks.org/wiki/C_Programming/C_Reference/nonstandard/strlcpy#Criticism

config_t config;
Preferences preferences;

void defaultNames(void) {
  strlcpy(config.hostname, HOSTNAME, HOSTNAME_SZ);
  strlcpy(config.devname, DEVICENAME, HOST_SZ);
}

void defaultWifiNetwork(void) {
  strlcpy(config.wifiSsid, WIFI_SSID, HOST_SZ);
  strlcpy(config.wifiPswd, WIFI_PSWD, PSWD_SZ);
}

void defaultStaticStaIp(void) {
  IPAddress ip;
  if (ip.fromString(STA_STATIC_IP))
    config.staStaticIP = ip;
  else
    config.staStaticIP = 0;
  if (ip.fromString(STA_GATEWAY))
    config.staGateway = ip;
  else
    config.staGateway = 0;
  if (ip.fromString(STA_NETMASK))
    config.staNetmask = ip;
  else
    config.staNetmask = 0;
}

void defaultAp(void) {
  strlcpy(config.apSuffix, AP_SUFFIX, AP_SUFFIX_SZ);
  strlcpy(config.apPswd, AP_PSWD, PSWD_SZ);
}

void defaultApip(void) {
  IPAddress ip;
  if (ip.fromString(AP_IP))
    config.apIP = ip;
  else
    config.apIP = 0;
  if (ip.fromString(AP_MASK))
    config.apMask = ip;
  else
    config.apMask = 0;
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
  config.logLevelWebc = LOG_LEVEL_WEBC;
  config.logLevelMqtt = LOG_LEVEL_MQTT;
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
  config.apDelayTime = AP_DELAY_TIME;
}

void defaultTopics(void) {
  strlcpy(config.topicDmtzPub, DMTZ_PUB_TOPIC, MQTT_TOPIC_SZ);
  strlcpy(config.topicDmtzSub, DMTZ_SUB_TOPIC, MQTT_TOPIC_SZ);
  strlcpy(config.topicLog, MQTT_LOG_TOPIC, MQTT_TOPIC_SZ);
  strlcpy(config.topicCmd, MQTT_CMD_TOPIC, MQTT_TOPIC_SZ);
}

uint32_t getConfigHash() {
  uint32_t hash = 0;
  uint8_t *bytes = (uint8_t*)&config;

  for (uint16_t i = 0; i < sizeof(config_t) - sizeof(config.checksum); i++)
    hash += bytes[i]*(i+1);
  return hash;
}

void useDefaultConfig(void) {
  memset(&config, 0x00, sizeof(config_t));
  config.magic = CONFIG_MAGIC;
  config.version = CONFIG_VERSION;

// -- start of user settings
  defaultNames();
  defaultWifiNetwork();
  defaultStaticStaIp();
  defaultAp();
  defaultApip();
  defaultSyslog();
  defaultDmtz();
  defaultIdx();
  defaultTopics();
  defaultMqtt();
  defaultTimes();
  defaultLogLevels();
// end of user settings --

  config.checksum = getConfigHash();
  addToLogP(LOG_INFO, TAG_CONFIG, PSTR("Using default configuration"));
}

void saveConfig(bool force) {
  uint32_t hash = getConfigHash();
  if ((force) || (hash != config.checksum)) {
    config.checksum = hash;
    preferences.begin("md", false); // open read/write
    preferences.clear();
    size_t len = preferences.putBytes("config", (const void*) &config, sizeof(config_t));
    preferences.end();
    if (len == sizeof(config_t))
      addToLogPf(LOG_INFO, TAG_CONFIG, PSTR("Saved config (%d bytes) to NVS"), len);
    else
      addToLogPf(LOG_ERR, TAG_CONFIG, PSTR("Saved %d bytes to NVS while config is %d bytes"), len, sizeof(config_t));
  }
}

bool loadConfigFromNVS(void) {
  preferences.begin("md", true); // open read-only
  //config.checksum = getConfigHash();
  size_t len = preferences.getBytes("config", (void*) &config, sizeof(config_t));
  preferences.end();
  if (len == sizeof(config_t))
    addToLogPf(LOG_INFO, TAG_CONFIG, PSTR("Loaded config (%d bytes) from NVS"), len);
  else {
    addToLogPf(LOG_ERR, TAG_CONFIG, PSTR("Loaded %d bytes from NVS expected %d bytes"), len, sizeof(config_t));
    return false;
  }
  if (config.version != CONFIG_VERSION) {
    addToLogP(LOG_ERR, TAG_CONFIG, PSTR("Loaded config wrong version"));
    return false;
  }
  uint32_t check = getConfigHash();
  if (check != config.checksum) {
    addToLogP(LOG_ERR, TAG_CONFIG, PSTR("Wrong config checksum"));
    return false;
  }
  return true;
}

void loadConfig(void) {
  if (!loadConfigFromNVS()) {
    useDefaultConfig();
    config.checksum = 0; // make sure it is saved to NVS when closing down
    addToLogPf(LOG_INFO, TAG_CONFIG, PSTR("Loaded default configuration version %d, size %d"), config.version, sizeof(config_t));
  }
}
