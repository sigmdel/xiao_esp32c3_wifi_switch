
#include <Arduino.h>
#include "ESPAsyncWebServer.h"  // for AsyncEventSource
#include "IPAddress.h"
#include "AsyncUDP.h"
#include "webserver.h"
#include "config.h"
#include "logging.h"


void mstostr(unsigned long milli, char* sbuf, int sbufsize) {
  int sec = milli / 1000;
  int min = sec / 60;
  int hr = min / 60;
  min = min % 60;
  sec = sec % 60;
  int frac = (milli % 1000);   //    hr:min:second.millisecond
  snprintf_P(sbuf, sbufsize-1, PSTR("%02d:%02d:%02d.%03d"), hr,min,sec,frac);
}

#define LOGSZ 520  // maximum length of a log message

AsyncUDP udp;

extern config_t config;
extern AsyncEventSource events;

const char *logLevelString[LOG_LEVEL_COUNT] = {
  "ERR",  //"error"
  "inf",  //"information"
  "dbg",  //"debug"
};

const char *tagString[TAG_COUNT] = {
  "???",
  "SYS",
  "CMD",
  "CFG",
  "HDW",
  "WIF",
  "MQT",
  "WEB",
  "DMZ"
};

// The log is implemented as a circurlar queue (called ring below) with replacement of
// the oldest entry when a message is added and the queue is full.
//
// See
//   https://www.pythoncentral.io/circular-queue/ for implementations without replacement


#define LOG_SIZE 64                // Max number of lines in log
#define MSG_SIZE 250               // Maximum size of formatted message

// Log ring
String        Log[LOG_SIZE];       // for log messages
uint8_t       LogLevel[LOG_SIZE];  // for log levels
uint8_t       LogTag[LOG_SIZE];    // for log tags
unsigned long LogTime[LOG_SIZE];   // for time each message is received by log

uint8_t head = 0;                 // Index where next message will be inserted in ring buffer
uint8_t tail = 0;                 // Index of the oldest message to next be send out
uint8_t count = 0;                // Number of messages yet to be sent out
uint8_t validcount = 0;           // Number of valid messages in the buffer, used for logHistory only


void addToLog(Log_level level, Log_tag  tag, const char *message) {
  //Serial.printf("addToLog at head: %d, tail: %d, count (to send): %d, validcount: %d\n", head, tail, count, validcount);
  // save the entry plus the current millisecond count
  LogTime[head] = millis();
  LogLevel[head] = level;
  LogTag[head] = tag;
  Log[head] = message;

  head = (head + 1) % LOG_SIZE; // Advance head to the next slot in the ring buffer
  count++;                      // Increment the count of messages yet to be sent out
  if (count > LOG_SIZE) {       // If more messages to be sent out than the size of the buffer
    tail = head;                // then move the tail to the oldest message in the buffer
    count = LOG_SIZE;           // Can't send out more messanges than the total number in the buffer
  }
  if (validcount < LOG_SIZE)    // Increment the number of valid messages up to the size of slots in the buffer
    validcount++;               // Only used in the

  //Serial.printf("addToLog done with head: %d, tail: %d, count (to send): %d, validcount: %d, LOG_SIZE: %d\n", head, tail, count, validcount, LOG_SIZE);
}

void addToLogf(Log_level level, Log_tag tag, const char *format, ...) {
  va_list args;
  char msg[MSG_SIZE];
  va_start(args, format);
  vsnprintf(msg, MSG_SIZE, format, args);
  va_end(args);
  addToLog(level, tag, msg);
}

void addToLogP(Log_level level, Log_tag tag, const char *message) {
  char msg[MSG_SIZE];
  strcpy_P(msg, message);
  addToLog(level, tag, msg);
}

/* not needed in this project
void addToLogP(Log_level level, Log_tag tag, const char *message1, const char *message2) {
  char msg[MSG_SIZE];
  char msg2[MSG_SIZE];
  strcpy_P(msg, message1);
  strcpy_P(msg2, message2);
  strncat(msg, msg2, sizeof(msg)-1);
  addToLog(level, tag, msg);
}
*/

void addToLogPf(Log_level level, Log_tag tag, const char *format, ...) {
  va_list args;
  char msg[MSG_SIZE];
  char msg2[MSG_SIZE];
  strcpy_P(msg, format);
  va_start(args, format);
  vsnprintf(msg2, sizeof(msg2), msg, args);
  va_end(args);
  addToLog(level, tag, msg2);
}

// Takes care of sending the oldest message in the ring buffer to the various log devices
// Returns the number of sent messages (0 - all messages had already been sent or 1 - one message sent)
int sendLog(void) {
  if (count < 1)
    return 0;

  // log time to string
  char mxtime[15];
  mstostr(LogTime[tail], mxtime, sizeof(mxtime));


  uint8_t level = LogLevel[tail];
  //[ ] TODO refactor following - see logHistory()
  // message = "[tag/lev]: logmessage";
  String message = " ";
  message += tagString[LogTag[tail]];
  message += "/";
  message += logLevelString[level];
  message += ": ";
  message += Log[tail];

  if ((level <= config.logLevelSyslog) && (wifiConnected))  {
    String msg = config.hostname + message;
    if (udp.connect(IPAddress(config.syslogIP), config.syslogPort))
      udp.print(msg);
    udp.close();
  }

  message = String(mxtime) + message;

  if (level <= config.logLevelUart) {
    Serial.printf("%s\n", message.c_str());
    //Serial.flush(); don't do this - uart logging will be blocking, especially if Serial not opened
  }

  if (level <= config.logLevelWebc) {
    //dbg:: Serial.println("log sending to webc");
    events.send(message.c_str(),"logvalue");
  }

/*
  //Serial.printf((!mqttClient.connected()) ? "mqttClient NOT connected\n" : "");
  if ( (level <= config.logLevelMqtt) && mqttClient.connected() )  {
    String topic = String(config.hostname) + "/" + String(config.mqttResponse);
    //Serial.printf("mqtt sending [%s]:  %s\n", topic.c_str(), message.c_str());
    mqttClient.publish(topic.c_str(), message.c_str());
  }

*/

  // remove the log message from the ring buffer
  tail = (tail + 1) % LOG_SIZE;   // move the tail to the next message to send
  count--;                        // reduce the count of message to be sent
  return 1;
}

void flushLog(void) {
  // send out any queued messages;
  while (count) {
    sendLog();
    delay(50);  // needed ??
  }
}

String logHistory(void) {
  //Serial.printf("logHistory, validcount: %d\n", validcount);
  char mxtime[15];
  String hist;
  String msg;
  if (validcount > 0) {
    int j = (validcount < LOG_SIZE) ? 0 : head;
    //Serial.printf("  - starting j: %d, head: %d\n", j, head);
    //Serial.print("  j: ");
    for (int i=0; i<validcount; i++) {
      //Serial.printf("%d, ", j);
      if (LogLevel[j] <= config.logLevelWebc) {
        //[ ] TODO refactor following !
        mstostr(LogTime[j], mxtime, sizeof(mxtime));
        msg = String(mxtime);
        msg += " ";
        msg += tagString[LogTag[j]];
        msg += "/";
        msg += logLevelString[LogLevel[j]];
        msg += ": " + Log[j] + "\n";
        hist += msg;
      }
      j = (j+1) % LOG_SIZE;
    }
    //Serial.printf("\n  - starts with %s\n", hist.substring(0, 64).c_str());
  }
  return hist;
}
