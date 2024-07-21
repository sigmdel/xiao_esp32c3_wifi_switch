#include <Arduino.h>
#include "logging.h"
#include "version.h"
#include "config.h"
#include "commands.hpp"

// BUG - it is possible that strlcpy could truncate !!!
//   see: https://en.wikibooks.org/wiki/C_Programming/C_Reference/nonstandard/strlcpy#Criticism

enum cmndError_t {etNone, etMissingParam, etUnknownCommand, etUnknownParam, etExtraParam, etInvalidValue};

static const char *cmdsrc[] = {
/* FROM_UART */  "uart",
/* FROW_WEBC */  "webc",
/* FROM_MQTT */  "mqtt"
};

static const char* cmds[] = {
  "config",         //  0 - manage configuration
  "dmtz",           //  1 - domoticz host, port, user, pswd
  "help",           //  2 - help
  "idx",            //  3 - domoticz idx values
  "log",            //  4 - log level for serial, mqtt and web output
  "mqtt",           //  5 - mqtt host, port, user, pswd
  "name",           //  6 - hostname
  "restart",        //  7 - restart the device
  "staip",          //  8 - static station IP //
  "syslog",         //  9 - syslog url, port
  "time",           // 10 - configure time intervals
  "topic",          // 11 - mqtt topics
  "version"         // 12 - returns firmware version
};

#define COMMAND_COUNT (int)(sizeof (cmds) / sizeof (const char *))

static const char *params[COMMAND_COUNT] = {
  "[save|load|default|erase]",                     // config
  "[-d] | [<host> [<port>]] ( [-x] | [-c <user> [<pswd>]] )", // dmtz
  "[<command>]",                                   // help
  "[-d] | [(switch|light|temp) [<value>]]",        // idx
  "[-d] | [(uart|mqtt|syslog|web) [<level>]]",     // log
  "[-d] | [[<host> [<port>]] ( [-x] | [-c <user> <pswd>]] )", // mqtt
  "[<hostname>]",                                  // name
  "[<n>]",                                         // restart
  "[-d | -x] | [<ip> <mask> <gateway>]",           // statip
  "[-d] | [<host> [<port>]]",                      // syslog
  "[-d] | [(poll|update|connect) [<ms>]]",         // time
  "[-d] | [(in|out) [<topic>]]",                   // topic
  ""                                               // version
};

#define TOKENCOUNT 7             // one more than the maximum number of tokens used
String token[TOKENCOUNT];

// Break up the string s into tokens
int parseString(String s) {
  addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("parse \"%s\""), s.c_str());
  int ndx0 = 0;
  int ndx1 = 0;
  int n = 0;
  int lc = s.length() - 1;
  String tok;

  // skip leading spaces;
  s.trim();
  while (ndx0 <= lc) {
    ndx1 = s.indexOf(' ', ndx0);
    if (ndx1 < 0) {
      tok = s.substring(ndx0);
      ndx0 = s.length();
    } else {
      tok = s.substring(ndx0, ndx1);
      ndx0 = ndx1+1;
    }
    tok.trim();
    if ((tok.length() > 0) && (n < TOKENCOUNT)) {
      token[n] = tok;
      addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("token %d = \"%s\""), n, token[n].c_str());
      n++;
    }
  }
  return n;
}


// Returns the command index of token[0] or -1 if not a command
int commandId(int idx = 0) {
  token[idx].toLowerCase();
  for (int i = 0; i<COMMAND_COUNT; i++) {
    if (token[idx].equals(cmds[i])) {
       return i;
    }
  }
  return -1;
}

//================ doCommand ================


//          1     1      1     1      2   <<< count
//          0     0      0     0      1   <<< errIndex
// config [save|load|default|erase] xtra
//
cmndError_t doConfig(int count, int &errIndex) {
  addToLogP(LOG_ERR, TAG_COMMAND, PSTR("not implemented"));
  return etUnknownCommand;
}


//                                                    c_index  +1     +2       +3
//  1    2   3          2       3      4      n+1  n+2    n+1 n+2    n+3      n+4  <<< max count = 6
//  0    1   2          1       2      3       n   n+1     n  n+1    n+2      n+3  <<< errIndex
// dmtz [-d] xtr1 | [ [<host> [<port>] xtr2]  [-x] xtr3 | [-c <user> [<pswd>]] xtr4 ]
//
void showDmtz(void) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s:%d, user: %s passwd: %s"), config.dmtzHost, config.dmtzPort,
    (strlen(config.dmtzUser)) ? config.dmtzUser : "<not defined>",
    (strlen(config.dmtzPswd)) ? "********" : "<not defined>");
}

cmndError_t doDmtz(int count, int &errIndex) {
  //bool restartAP = false;
  int c_index = 0;
  errIndex = 0;

  if ((count > 1) && token[1].equals("-d")) {
    defaultDmtz();
    showDmtz();
    if (count > 2) {
      errIndex = 2;
      return etExtraParam;
    }
    return etNone;
  }

  if (count > 1) {
    for (int ti=1; ti<count; ti++) {
      if (token[ti].equals("-c")) {
        c_index = ti;
        continue;
      }
      if (token[ti].equals("-x")) {
        config.dmtzUser[0] = '\0';
        config.dmtzPswd[0] = '\0';
        showDmtz();
        if (count > ti+1)  {
          errIndex = ti+1;
        } else if (ti > 3) {
          errIndex = 3;
        }
        if (errIndex)
          return etExtraParam;
        return etNone;
      }
      if (ti==1+c_index) {
        if (c_index == 0) {
          strlcpy(config.dmtzHost, token[ti].c_str(), HOST_SZ);
        } else {
          strlcpy(config.dmtzUser, token[ti].c_str(), USER_SZ);
        }
      } else if (ti==2+c_index) {
        if (c_index == 0) {
          config.dmtzPort = token[ti].toInt();
        } else {
          if (strlen(token[ti].c_str()) < 8) {
            addToLogP(LOG_ERR, TAG_COMMAND, PSTR("WiFi password must be at least 8 characters long"));
            showDmtz();
            errIndex = ti;
            return etInvalidValue;
          }
          strlcpy(config.dmtzPswd, token[ti].c_str(), PSWD_SZ);
        }
      }
    }
  }

  showDmtz();

  if ((c_index > 3) || ((c_index == 0) && (count > 4)))
    errIndex = 4;
  else if ((c_index > 0) && (count > c_index + 3))
    errIndex = c_index + 3;

  if (errIndex)
    return etExtraParam;

  return etNone;
}

//      1       2   <<< counter
//      0       1   <<< index
//  "help   [<command>]"
//
#define HELP_SZ 120
cmndError_t doHelp(int count, int &errIndex) {
  char space = ' ';
  char msg[HELP_SZ] = {0};
  int cid;
  errIndex = 1;

  if (count == 1) {
    strncpy(msg, "commands:", HELP_SZ);
    for (int i=0; i < COMMAND_COUNT; i++) {
      if (strlen(msg) < HELP_SZ) strncat(msg, &space, 1);
      strncat(msg, cmds[i], HELP_SZ-strlen(msg));
    }
    addToLog(LOG_INFO, TAG_COMMAND, msg);
  } else {
    cid = commandId(1);
    if (cid < 0) return etUnknownParam;
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s %s"), cmds[cid], params[cid]);
  }

  if (count > 2) {
    errIndex = 2;
    return etExtraParam;
  }
  return etNone;
}



void showIdx(void) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("switch idx: %d"), config.dmtzSwitchIdx);
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("light sensor idx: %d"), config.dmtzLSIdx);
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("temp sensor idx: %d"), config.dmtzTHSIdx);
}

//   1    2    3            2                 3        4  <<< count
//   0    1    2            1                 2        3  <<< index
//  idx [-d] xtra1 | [(switch|light|temp) [<value>]] xtra2
//  "idx -d"
cmndError_t doIdx(int count, int &errIndex) {

  if (count == 1) {
    showIdx();
    return etNone;
  }

  if ((count > 1) and token[1].equals("-d")) {
    defaultIdx();
    showIdx();
    if (count > 2) {
      errIndex = 2;
      return etExtraParam;
    }
    return etNone;
  }

  long anIdx = -1;
  if (count > 2) {
    anIdx = token[2].toInt();
    if (anIdx <= 0) {
      errIndex = 2;
      return etInvalidValue;
    }
  }

  token[1].toLowerCase();

  if (token[1].equals("switch")) {
    if (count > 2) config.dmtzSwitchIdx = anIdx;
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("switch idx: %d"), config.dmtzSwitchIdx);

  } else if (token[1].equals("light")) {
    if (count > 2) config.dmtzLSIdx = anIdx;
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("light sensor idx: %d"), config.dmtzLSIdx);

  } else if (token[1].equals("temp")) {
    if (count > 2) config.dmtzTHSIdx = anIdx;
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("temp sensor idx: %d"), config.dmtzTHSIdx);

  } else {
    errIndex = 1;
    return etUnknownParam;
  }

  if (count > 3) {
    errIndex = 3;
    return etExtraParam;
  }
  return etNone;
}

extern const char *logLevelString[];

//    1         2             3  <<< count
//    0         1             2  <<< errIndex
//  "log (uart|mqtt|web) [<level>]"
//
cmndError_t doLog(int count, int &errIndex) {
  if (count < 2) return etMissingParam;
  errIndex = 1;
  int lg;
  int lv = -1;
  if (count > 1) {
    token[1].toLowerCase();
    if (token[1].equals("uart"))
      lg = 0;
    else if (token[1].equals("mqtt"))
      lg = 1;
    else if (token[1].equals("syslog"))
      lg = 2;
    else if (token[1].equals("web"))
      lg = 3;
    else
      return etUnknownParam;
    if (count > 2) {
      token[2].toLowerCase();
      for (int i = 0; i<LOG_LEVEL_COUNT; i++) {
        if (token[2].equals(logLevelString[i])) {
          lv = i;
          break;
        }
      }
      if ((lv < 0) && (token[2].length()==1)) {
        lv = (byte)token[2][0] - '0';
      }
      if ( (lv < 0) || (lv >= LOG_LEVEL_COUNT) ) {
        errIndex = 2;
        return etInvalidValue;
      }
    }
  }
  switch (lg) {
    case 0: {
      if (count > 2) {config.logLevelUart = lv;}
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s log level: %s"), "uart", logLevelString[config.logLevelUart]);
      break;
    }
    case 1: {
      if (count > 2) {config.logLevelMqtt = lv;}
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s log level: %s"), "mqtt", logLevelString[config.logLevelMqtt]);
      break;
    }
    case 2: {
      if (count > 2) {config.logLevelSyslog = lv;}
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s log level: %s"), "syslog", logLevelString[config.logLevelSyslog]);
      break;
    }
    case 3: {
      if (count > 2) {config.logLevelWeb = lv;}
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s log level: %s"), "web", logLevelString[config.logLevelWeb]);
      break;
    }
  }
  if (count > 3) {
    errIndex = 3;
    return etExtraParam;
  }
  return etNone;
}


void showMqtt(void) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s:%d, user: %s passwd: %s"), config.mqttHost, config.mqttPort,
    (strlen(config.mqttUser)) ? config.mqttUser : "<not defined>",
    (strlen(config.mqttPswd)) ? "********" : "<not defined>");
}

//                                                    c_index  +1     +2       +3
//  1    2   3          2       3      4      n+1  n+2    n+1 n+2    n+3      n+4  <<< max count = 6
//  0    1   2          1       2      3       n   n+1     n  n+1    n+2      n+3  <<< errIndex
// mqtt [-d] xtr1 | [ [<host> [<port>] xtr2]  [-x] xtr3 | [-c <user> [<pswd>]] xtr4 ]
//
cmndError_t doMqtt(int count, int &errIndex) {
  //bool restartAP = false;
  int c_index = 0;
  errIndex = 0;

  if ((count > 1) && token[1].equals("-d")) {
    defaultMqtt();
    showMqtt();
    if (count > 2) {
      errIndex = 2;
      return etExtraParam;
    }
    return etNone;
  }

  if (count > 1) {
    for (int ti=1; ti<count; ti++) {
      if (token[ti].equals("-c")) {
        c_index = ti;
        continue;
      }
      if (token[ti].equals("-x")) {
        config.mqttUser[0] = '\0';
        config.mqttPswd[0] = '\0';
        showDmtz();
        if (count > ti+1)  {
          errIndex = ti+1;
        } else if (ti > 3) {
          errIndex = 3;
        }
        if (errIndex)
          return etExtraParam;
        return etNone;
      }
      if (ti==1+c_index) {
        if (c_index == 0) {
          strlcpy(config.mqttHost, token[ti].c_str(), HOST_SZ);
        } else {
          strlcpy(config.mqttUser, token[ti].c_str(), USER_SZ);
        }
      } else if (ti==2+c_index) {
        if (c_index == 0) {
          config.dmtzPort = token[ti].toInt();
        } else {
          if (strlen(token[ti].c_str()) < 8) {
            addToLogP(LOG_ERR, TAG_COMMAND, PSTR("WiFi password must be at least 8 characters long"));
            showDmtz();
            errIndex = ti;
            return etInvalidValue;
          }
          strlcpy(config.mqttPswd, token[ti].c_str(), PSWD_SZ);
        }
      }
    }
  }

  showMqtt();

  if ((c_index > 3) || ((c_index == 0) && (count > 4)))
    errIndex = 4;
  else if ((c_index > 0) && (count > c_index + 3))
    errIndex = c_index + 3;

  if (errIndex)
    return etExtraParam;

  return etNone;
}



bool isValidHostnameChar(char c) {
  //Serial.printf("isValidHostChar(\"%c\")\n", c);
  if (c == '-') return true;
  if (c < '0')  return false;
  if (c <= '9') return true;
  if (c < 'A')  return false;
  if (c <= 'Z') return true;
  if (c < 'a')  return false;
  if (c <= 'z') return true;
  return false;
}

//    1         2       3  <<< count
//    0         1       2  <<< errIndex
//  name [<hostname>] xtra
//
cmndError_t doName(int count, int &errIndex) {
  bool disconnect = false;
  if (count > 1) {
    errIndex = 1;
    int len = token[1].length();
    for (int i=0; i < len; i++) {
      if (!isValidHostnameChar(token[1].charAt(i))) {
        return etInvalidValue;
      }
    }
    if ( token[1].startsWith("-") || token[1].endsWith("-") )
      return etInvalidValue;

    disconnect = !token[1].equals(config.hostname); // case does not matter for Wi-Fi hostname but it does matter for MQTT topics
    if (disconnect) strlcpy(config.hostname, token[1].c_str(), HOSTNAME_SZ);
  }
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("hostname: %s"), config.hostname);
  if (count > 2) {
    errIndex = 2;
    return etExtraParam;
  }
  /*
  if (disconnect) {
    mqtt_disconnect();
    setHostName();
    //connectWiFi(config.netSsid, config.netPsk);
  }
  */
  return etNone;
}


//         1    2       1         2          3   <<< count
//         0    1       0         0          1   <<< errIndex
// restart [n]  xtra2
//
cmndError_t doRestart(int count, int &errIndex) {
  addToLogP(LOG_INFO, TAG_HARDWARE, PSTR("Restart not implemented"));
  delay(1000); // wait one second
  return etNone; // keep compiler happy
}


//    1       2        3       2     3      4          5 <<< count
//    0       1        2       1     2      3          4 <<< errIndex
// statip [-d | -x ] xtra1 | [<ip> <mask> <gateway>] xtra2
//
cmndError_t doStaip(int count, int &errIndex) {
  cmndError_t errCode = etNone;
  errIndex = 0;
  bool restartMessage = false;
  if (count > 1) {
    if (token[1].equals("-d")) {
      defaultStaticStaIp();
      restartMessage = true;
      if (count > 2  ) {
        errIndex = 2;
        errCode = etExtraParam;
      }

    } else if (token[1].equals("-x")) {
      config.staStaticIP = 0;
      config.staNetmask = 0;
      config.staGateway = 0;
      restartMessage = true;
      if (count > 2  ) {
        errIndex = 2;
        errCode = etExtraParam;
      }

    } else if (count < 4) {
      errCode = etMissingParam;

    } else {
      IPAddress ipa;
      IPAddress mask;
      IPAddress gateway;
      if (!ipa.fromString(token[1])) {
        errIndex = 1;
      } else if (!mask.fromString(token[2])) {
        errIndex = 2;
      } else if (!gateway.fromString(token[3])) {
        errIndex = 3;
      }
      if (errIndex)
        errCode = etInvalidValue;
      else {
        // only update values if all three iP are valid
        config.staStaticIP = ipa;
        config.staNetmask = mask;
        config.staGateway = gateway;
        restartMessage = true;
        if (count > 4) {
          errCode = etExtraParam;
          errIndex = 4;
        }
      }
    }
  }
  // show current station static ip configuration
  if (config.staStaticIP)
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Static station IP %s, netmask %s and gateway %s"),
      IPAddress(config.staStaticIP).toString().c_str(),
      IPAddress(config.staNetmask).toString().c_str(),
      IPAddress(config.staGateway).toString().c_str());
  else
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Using DHCP obtained IP address"));
  if (restartMessage)
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Any change will take effect on next boot only - NOT YET IMPLEMENTED"));

  return errCode;
}


void showSyslog(void) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Syslog host: %s, port: %d"),
    IPAddress(config.syslogIP).toString().c_str(), config.syslogPort);
}

//    1     2    3       2      3       4   <<< count
//    0     1    2       1      2       3   <<< errIndex (1, 2 = etInvalidValue, 3 = etExtraParam)
// syslog [-d] xtra1 | [<ip> [<port>]] xtra2
//
cmndError_t doSyslog(int count, int &errIndex) {
  errIndex = 0;
  if (count > 1) {
    if (token[1].equals("-d")) {
      defaultSyslog();
      if (count > 2)
        errIndex = 2;
    } else {
      IPAddress ipa;
      int aPort;
      if (!ipa.fromString(token[1]))
        errIndex = 1;
      else
        config.syslogIP = ipa;
      if (count > 2) {
        aPort = token[2].toInt();
        if (aPort <= 0)
          errIndex = 2;
        else
          config.syslogPort = aPort;
      }
    }
  }
  showSyslog();

  if ((count > 3) && (errIndex == 0))
    errIndex = 3;

  if (errIndex==1)
    return etInvalidValue;
  else if (errIndex)
    return etExtraParam;

  return etNone;
}


//   1    2           2                 3       4  <<< count
//   0    1           1                 2       3 <<< errIndex
// time [-d] | [(poll|update|connect) [<ms>]]  xtra
//
cmndError_t doTime(int count, int &errIndex) {
  long aTime;

  if ((count > 1) and token[1].equals("-d")) {
    defaultTimes();
    if (count > 2)  {
      errIndex = 2;
      return etExtraParam;
    }
    return etNone;
  }

  if (count < 2) return etMissingParam;

  if (count > 2) {
    aTime = token[2].toInt();
    if (aTime <= 0) {
      errIndex = 2;
      return etInvalidValue;
    }
  }

  token[1].toLowerCase();

  if (token[1].equals("poll")) {
    if (count > 2) { config.hdwPollTime = aTime; }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Hardware poll interval time: %li ms"), config.hdwPollTime);


  } else if (token[1].equals("connect")) {
    if (count > 2) { config.dmtzReqTimeout = aTime; }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("HTTP request timeout: %d ms"), config.dmtzReqTimeout);

  } else if (token[1].equals("update")) {
    if (count > 2) { config.sensorUpdtTime = aTime; }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Sensor data update time: %d ms"), config.sensorUpdtTime);

  } else {
      errIndex = 1;
      return etUnknownParam;
  }

  if (count > 3) {
    errIndex = 3;
    return etExtraParam;
  }
  return etNone;
}


//         1    2       1         2          3   <<< count
//         0    1       0         0          1   <<< errIndex
// topic [-d] xtra1 | [(in|out) [<topic>]]  xtra2
//
cmndError_t doTopic(int count, int &errIndex) {
  addToLogP(LOG_ERR, TAG_COMMAND, PSTR("not implemented"));
  return etUnknownCommand;
}


#define APP_NAME "Firmware"
//
//     1      2  <<< count
//     0      1  <<< index
//  version  extra
//
cmndError_t doVersion(int count, int &errIndex) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s version %s"), APP_NAME, FirmwareVersionToString().c_str());
  if (count > 1)  {
    errIndex = 1;
    return etExtraParam;
  }
  return etNone;
}

typedef cmndError_t (*dofnct)(const int, int&);

dofnct doFunctionList[COMMAND_COUNT] = {
  doConfig,
  doDmtz,
  doHelp,
  doIdx,
  doLog,
  doMqtt,
  doName,
  doRestart,
  doStaip,
  doSyslog,
  doTime,
  doTopic,
  doVersion
};

void exec(String command, int source) {
  cmndError_t error = etNone;
  int count = parseString(command);  // fill tokens
  if (count < 1) return;

  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("From %s: %s"), cmdsrc[source], command.c_str());

  int errIndex = 0;
  int id = commandId();

   if ( (id < 0) || (id >= COMMAND_COUNT) || (doFunctionList[id] == nullptr) )
    error = etUnknownCommand;
  else
    error = doFunctionList[id](count, errIndex);

  switch (error) {
    case etMissingParam:   {
        addToLogP(LOG_ERR, TAG_COMMAND, PSTR("Missing parameter"));
      } break;
    case etUnknownCommand: {
        addToLogPf(LOG_ERR, TAG_COMMAND, PSTR("\"%s\" unknown command"), token[errIndex].c_str());
      } break;
    case etUnknownParam:   {
        addToLogPf(LOG_ERR, TAG_COMMAND, PSTR("\"%s\" unknown parameter"), token[errIndex].c_str());
      } break;
    case etExtraParam:     {
        addToLogPf(LOG_ERR, TAG_COMMAND, PSTR("\"%s\" extra parameter"), token[errIndex].c_str());
      } break;
    case etInvalidValue:  {
        addToLogPf(LOG_ERR, TAG_COMMAND, PSTR("\"%s\" invalid value"), token[errIndex].c_str());
    }
    default:               {
        return;
      } break; // etNone
  }
}

void doCommand(cmndSource_t source, String cmnd) {
  addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Commands from %s: %s"), cmdsrc[source], cmnd.c_str());
  int ndx0 = 0;
  int ndx1 = 0;
  int n = 0;
  int lc = cmnd.length() - 1;

  // skip leading spaces and ';'
  while ((ndx0 <= lc) && (cmnd.charAt(ndx0) == ' ' || cmnd.charAt(ndx0) == ';') ) ndx0++;

  String command;
  while (ndx0 <= lc) {
    ndx1 = cmnd.indexOf(';', ndx0);
    if (ndx1 < 0) {
      command = cmnd.substring(ndx0);
      ndx0 = cmnd.length();
    } else {
      command = cmnd.substring(ndx0, ndx1);
      ndx0 = ndx1+1;
    }
    command.trim();
    if (command.length() > 0) {
      n++;
      //addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("command %d = \"%s\" ndx0: %d, ndx1: %d"), n, command.c_str(), ndx0, ndx1);
      exec(command, source);
    }
  }
  if (!n) addToLogP(LOG_DEBUG, TAG_COMMAND, PSTR("no commands"));
}

void doCommand(cmndSource_t source, const char *cmnd) {
  doCommand(source, String(cmnd));
}
