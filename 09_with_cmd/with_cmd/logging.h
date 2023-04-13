#pragma once

#include <Arduino.h>

/*
   Syslog priority levels

   LOG_EMERG = 0,      // system is unusable, index = 0 just to be sure
   LOG_ALERT = 1,      // action must be taken immediately
   LOG_CRIT = 2,       // critical conditions
   LOG_ERR = 3,        // error conditions
   LOG_WARNING = 4,    // warning conditions
   LOG_NOTICE = 5,     // normal but significant condition
   LOG_INFO = 6,       // informational
   LOG_DEBUG = 7,      // debug-level messages

   However only 3 will be used here
 */

enum Log_level {
   LOG_ERR = 0,    // error conditions
   LOG_INFO,       // informational
   LOG_DEBUG,      // debug-level messages
   LOG_LEVEL_COUNT // number of log levels
};

extern bool wifiConnected;
//extern const char *logLevelString[LOG_LEVEL_COUNT];

enum Log_tag {
  TAG_UNDEF = 0, // "???"
  TAG_SYSTEM,    // "SYS"  // setup
  TAG_COMMAND,   // "CMD"
  TAG_CONFIG,    // "CFG"
  TAG_HARDWARE,  // "HDW"
  TAG_WIFI,      // "WIF"
  TAG_MQTT,      // "MQT"
  TAG_WEBSERVER, // "WEB"
  TAG_DOMOTICZ,  // "DMZ"
  TAG_COUNT      // number of tags
};

/*
 * A string or strings sent to the log with any of the logging functions will be stored in a
 * circular queue.
 *
 * When sendLog() is invoked the oldest entry in the queue, that has not already been sent, will
 *  a) sent to the syslog server if level has a higher or equal priority to config.logLevelSyslog
 *  b) printed to the serial port if level has a higher or equal priority to config.logLevelUart
 *  c) sent to the web server if level has a higher or equal priority to config.logLevelWeb
 *  d) sent to the mqtt server if level has a higher or equal priority to config.logLevelMqtt
 *  *** (d) NOT YET IMPLEMENTED ***
 *
 * To see the syslog messages as they come in on the system log server thepi.local
 *    pi@thepi:~$ sudo tail -f /var/log/syslog
 *
 * Try to send an UDP first with netcat to test any networking issue:
 *    ~ $ echo "test message" |nc -w1 -u IP_ADDRESS UDP_PORT
 *
 * michel@hp:~$ echo "test message" | nc -w1 -u 192.168.1.22 514
 *
 * pi@thepi:~$ sudo tail -f /var/log/syslog
 * ...
 * Mar 30 23:04:26 test message
 */

  // Typical use: sendToLog(LOG_INFO, TAG_SYSTEM, "Some information");
void addToLog(Log_level level, Log_tag tag, const char *line );

  // Typical use: sendToLogf(LOG_INFO, TAG_HARDWARE, "Count: %d, free: %d at %s", 32, 12498, "some_string");
void addToLogf(Log_level level, Log_tag tag, const char *format, ...);

  // Typical use: sendToLogP(LOG_ERR, TAG_SYSTEM, PSTR("Fatal Error"));
void addToLogP(Log_level level, Log_tag tag, const char *line);

  // Typical use: sendToLogP(LOG_ERR, TAG_HARDWARE, PSTR("Fatal Error"), PSTR("REBOOTING"));
void addToLogP(Log_level level, Log_tag tag, const char *linep1, const char *linep2);

  // Typical use: sendToLogPf(LOG_DEBUG, TAG_MQTT, PSTR("Count: %d, free: %d at %s"), 32, 12498, "some_string");
void addToLogPf(Log_level level, Log_tag tag, const char *pline, ...);

  // Transmits the oldest message in the log buffer not already sent.
  // Call in the loop() when it should be safe to access the Serial device, etc.
int sendLog(void);

void mstostr(unsigned long milli, char* sbuf, int sbufsize);

// Returns the content of the log from the oldes entry to the newest
// in one string where each entry is terminated with a line feed.
String logHistory(void);
