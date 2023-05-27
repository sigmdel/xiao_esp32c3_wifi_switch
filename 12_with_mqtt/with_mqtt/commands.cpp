#include <Arduino.h>
#include "logging.h"
#include "version.h"
#include "config.h"
#include "wifiutils.hpp"
#include "mqtt.hpp"
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
  "ap",             // - manage access point
  "apip",           // - access point IP
  "config",         // - manage configuration
  "dmtz",           // - domoticz host, port, user, pswd
  "help",           // - help
  "idx",            // - domoticz idx values
  "log",            // - log level for serial, mqtt and web output
  "mqtt",           // - mqtt host, port, user, pswd
  "name",           // - hostname
  "restart",        // - restart the device
  "staip",          // - static station IP
  "status",         // - system status
  "syslog",         // - syslog url, port
  "time",           // - configure time intervals
  "topic",          // - mqtt topics
  "wifi"            // - returns wifi status
};


#define COMMAND_COUNT (int)(sizeof (cmds) / sizeof (const char *))

static const char *params[COMMAND_COUNT] = {
  /* ap      */ "[-d|-x] | [<suffix> [<pswd>]]",
  /* apip    */ "[-d|-x] | [<ip> <mask>]",
  /* config  */ "[load|default|save [force]]",
  /* dmtz    */ "[-d] | [<host> [<port>]] ( [-x] | [-c <user> [<pswd>]] )",
  /* help    */ "[<command>]",
  /* idx     */ "[-d] | [(switch|temp|lux) [<id>]]",
  /* log     */ "[-d] | [(uart|mqtt|syslog|webc) [ERR|inf|dbg|<level>]]",
  /* mqtt    */ "[-d] | [[<host> [<port>]] ( [-x] | [-c <user> <pswd>]] )",
  /* name    */ "[-d] | [-h [<hostname>]] | [-n [<device name>]]",
  /* restart */ "[[0|1|...|7]",
  /* staip   */ "[-d|-x] | [<ip> <gateway> <mask>]",
  /* status  */  "",
  /* syslog  */ "[-d] | [<hostIP> [<port>]]",
  /* time    */ "[-d] | [(poll|update|http|ap) [<ms>]]",
  /* topic   */ "[-d] | [(log|cmd|pub|sub) [<topic>]]",
  /* wifi    */ "[-d] | [<ssid> [<pswd]]"
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


//
//   1      2     3         2       3       4     +1  +2    +3        <<< count
//   0      1     2         1       2       3     +1  +2    +3        <<< errIndex
//  ap   [-d|-x] xtra1 | [<suffix> [pswd]]  extra
//
cmndError_t doAp(int count, int &errIndex) {
  errIndex = 1;
  if (count > 1) {
    errIndex = 2;
    if ( (token[1].equals("-d")) || (token[1].equals("-D")) )
      defaultAp();
    else if  ( (token[1].equals("-x")) || (token[1].equals("-X")) ) {
      config.apSuffix[0] = '\0';
      config.apPswd[0] = '\0';
    } else {
      strlcpy(config.apSuffix, token[1].c_str(), AP_SUFFIX_SZ);
      if (count > 2) {
        errIndex = 3;
        strlcpy(config.apPswd, token[2].c_str(), PSWD_SZ);
      } else
        config.apPswd[0] = '\0';
    }
  }
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("AP suffix: %s, password: %s"),
    config.apSuffix,
    (strlen(config.apPswd)) ? "********" : "<none>");
  if (count > errIndex)
    return etExtraParam;
  return etNone;
}


//  1       2        3       2     3      4       <<< count
//  0       1        2       1     2      3       <<< errIndex
// apip [-d | -x ] xtra1 | [<ip> <mask>] xtra2
//
cmndError_t doApip(int count, int &errIndex) {
  cmndError_t errCode = etNone;
  errIndex = 0;
  if (count > 1) {
    if (token[1].equals("-d")) {
      defaultApip();
      if (count > 2  ) {
        errIndex = 2;
        errCode = etExtraParam;
      }

    } else if (token[1].equals("-x")) {
      config.staStaticIP = 0;
      config.staNetmask = 0;
      config.staGateway = 0;
      if (count > 2  ) {
        errIndex = 2;
        errCode = etExtraParam;
      }

    } else if (count < 3) {
      errCode = etMissingParam;

    } else {
      IPAddress ipa;
      IPAddress mask;
      if (!ipa.fromString(token[1]))
        errIndex = 1;
      else if (!mask.fromString(token[2]))
        errIndex = 2;
      if (errIndex)
        errCode = etInvalidValue;
      else {
        // only update values if all two iP are valid
        config.apIP = ipa;
        config.apMask = mask;
        if (count > 3) {
          errCode = etExtraParam;
          errIndex = 3;
        }
      }
    }
  }
  // show current station static ip configuration
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Static access point IP %s, and netmask %s"),
    (config.apIP) ? IPAddress(config.apIP).toString().c_str() : "192.168.4.1",
    (config.apMask) ? IPAddress(config.apMask).toString().c_str() : "255.255.255.0");
  return errCode;
}



//  [ ] change syntax to config [-d] | [-r] | [-w | -f] | [-a (off | on)]
//  but  wait until restart is better defined.
///  1        2    3       2      3       2     2      3     4          2        3       4     <<< count
///  0        1    2       1      2       1     1      2     3          1        2       3      <<< errIndex
// config [ load xtra1 | default xtra2 | save xtra3 [force] xtra4 ]
//
cmndError_t doConfig(int count, int &errIndex) {
  bool force = false;
  if (count == 1)
    errIndex = 1;
  else {
    errIndex = 2; // assume xtra1, extr2 or invalid param (off | on)
    token[1].toLowerCase();
    if (token[1].equals("load"))
      loadConfig();
    else if (token[1].equals("default"))
      useDefaultConfig();
    else if (token[1].equals("save")) {
      if (count > 2) {
        errIndex = 3;
        token[2].toLowerCase();
        if (!token[2].equals("force"))
          return etUnknownParam;
        force = true;
      }
      saveConfig(force); // save even if config has not changed
    } else {
      errIndex = 1;
      return etUnknownCommand;
    }
  } // count > 1
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Config version: %d, size: %d"), config.version, sizeof(config_t));

  if (count > errIndex)
    return etExtraParam;
  return etNone;
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

  if (count < 2) {
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
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Domoticz virtual Idx: switch = %d, light sensor = %d, temp + humid sensor = %d"),
    config.dmtzSwitchIdx, config.dmtzLSIdx, config.dmtzTHSIdx);
}

//   1   2                  2            3       4  <<< count
//   0   1                  1            2       3  <<< index
// idx [-d] xtra1 | [(switch|temp|lux) [<id>]] xtra2
//
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
  bool setvalue = (count > 2);

  if (setvalue) {
    anIdx = token[2].toInt();
    if (anIdx <= 0) {
      errIndex = 2;
      return etInvalidValue;
    }
  }

  token[1].toLowerCase();

  if (token[1].equals("switch")) {
    if (setvalue) config.dmtzSwitchIdx = anIdx;
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("switch idx: %d"), config.dmtzSwitchIdx);

  } else if (token[1].equals("lux")) {
    if (setvalue) config.dmtzLSIdx = anIdx;
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("light sensor idx: %d"), config.dmtzLSIdx);

  } else if (token[1].equals("temp")) {
    if (setvalue) config.dmtzTHSIdx = anIdx;
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

//    1           2                   3      4 <<< count
//    0           1                   2      3 <<< errIndex
//  "log (uart|mqtt|syslog|webc) [<level>] extr"
//
#define LOG_FACILITY_COUNT 4
const char *logFacility[LOG_FACILITY_COUNT] = {
  "uart",
  "mqtt",
  "syslog",
  "webc"
};

cmndError_t doLog(int count, int &errIndex) {
  if (count < 2) {
    String st("Log levels ");
    for (int i=0; i<LOG_FACILITY_COUNT; i++) {
      st += logFacility[i];
      st += ": ";
      int level = -1;
      switch (i) {
        case 0: level = config.logLevelUart; break;
        case 1: level = config.logLevelMqtt; break;
        case 2: level = config.logLevelSyslog; break;
        case 3: level = config.logLevelWebc; break;
      }
      if ((level < 0) || (level >= LOG_LEVEL_COUNT))
        st += "unknown";
      else
        st += logLevelString[level];
      if (i < LOG_FACILITY_COUNT-1)
        st += ", ";
    }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Log levels %s"), st.c_str());
    return etNone;
  }
  int facility = - 1;
  int level = -1;
  token[1].toLowerCase();
  facility = -1;
  for (int i=0; i < LOG_FACILITY_COUNT; i++) {
    if (token[1].equals(logFacility[i])) {
      facility = i;
      break;
    }
  }
  if (facility < 0) {
    errIndex = 1;
    return etUnknownParam;
  }
  errIndex = 2;
  bool setvalue = (count > 2);
  if (setvalue) {
    // [ ] BUG: logLevelString[] are mixed case!
    //token[2].toLowerCase();
    for (int i = 0; i<LOG_LEVEL_COUNT; i++) {
      if (token[2].equals(logLevelString[i])) {
        level = i;
        break;
      }
    }
    // not a string, assume it is a number
    if ((level < 0) && (token[2].length()==1)) {
      level = (byte)token[2][0] - '0';
    }
    if ( (level < 0) || (level >= LOG_LEVEL_COUNT) ) {
      return etInvalidValue;
    }
  }
  switch (facility) {
    case 0: {
      if (setvalue)
        config.logLevelUart = level;
      else
        level = config.logLevelUart;
      break;
    }
    case 1: {
      if (setvalue)
        config.logLevelMqtt = level;
      else
        level = config.logLevelMqtt;
      break;
    }
    case 2: {
      if (setvalue)
        config.logLevelSyslog = level;
      else
        level = config.logLevelSyslog;
      break;
    }
    case 3: {
      if (setvalue)
        config.logLevelWebc = level;
      else
        level = config.logLevelWebc;
      break;
    }
  }
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s log level: %s (%d)"), logFacility[facility], logLevelString[level], level);
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
          /*
          if (strlen(token[ti].c_str()) < 8) {
            addToLogP(LOG_ERR, TAG_COMMAND, PSTR("Password must be at least 8 characters long"));
            showDmtz();
            errIndex = ti;
            return etInvalidValue;
          } */
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

//   1     2    3       2    3         2    3      4  <<< count
//   0     1    2       1    2         1    2      3 <<< errIndex
// name  [-d] xtra1 | [-h [name>]] | [-n [name]] xtra2
//
cmndError_t doName(int count, int &errIndex) {
  bool disconnect = false;
  errIndex = 2; // assume presence of xtra1
  if (count < 2)
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Host name: %s, device name: %s"), config.hostname, config.devname);
  else {
    if (token[1].equals("-d"))
      defaultNames();
    else if (token[1].equals("-h")) {
      if (count > 2) {
        // test for valid host name
        int len = token[2].length();
        for (int i=0; i < len; i++) {
          if (!isValidHostnameChar(token[2].charAt(i))) {
            return etInvalidValue;
          }
        }
        if ( token[2].startsWith("-") || token[2].endsWith("-") )
          return etInvalidValue;
        // valid, continue
        errIndex = 3; // assume presence of xtra2
        disconnect = true;
        strlcpy(config.hostname, token[2].c_str(), HOSTNAME_SZ);
      }
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Host name: %s"), config.hostname);
      if (disconnect)
        addToLogP(LOG_INFO, TAG_COMMAND, PSTR("The change will take effect on the next connection to the network"));
    } else if (token[1].equals("-n")) {
       if (count > 2) {
         errIndex = 3;
         while (errIndex < count) {
            token[2] += " " + token[errIndex];
            errIndex++;
         }
         strlcpy(config.devname, token[2].c_str(), HOST_SZ);
       }
       addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Device name: %s"), config.devname);
    } else {
      errIndex = 1;
      return etUnknownParam;
    }
  }
  if (count > errIndex)
    return etExtraParam;
  return etNone;
}


extern void espRestart(int level = 0);

//     1    2     3   <<< count
//     0    1     2   <<< errIndex
// restart [n]  xtra2
//
cmndError_t doRestart(int count, int &errIndex) {
  int n = 0;
  if (count > 1) {
    n = (byte)token[2][0] - '0';
    if ( (token[2].length()>1)  || (n < 0) ) {
      errIndex = 1;
      return etInvalidValue;
    }
  }
  if (count > 2) {
    errIndex = 2;
    return etExtraParam;
  }
  espRestart(n);
  return etNone; // keep compiler happy
}


//    1       2        3       2     3      4          5 <<< count
//    0       1        2       1     2      3          4 <<< errIndex
// statip [-d | -x ] xtra1 | [<ip> <gateway> <mask>] xtra2
//
cmndError_t doStaip(int count, int &errIndex) {
  cmndError_t errCode = etNone;
  errIndex = 0;
  if (count > 1) {
    if (token[1].equals("-d")) {
      defaultStaticStaIp();
      if (count > 2  ) {
        errIndex = 2;
        errCode = etExtraParam;
      }

    } else if (token[1].equals("-x")) {
      config.staStaticIP = 0;
      config.staNetmask = 0;
      config.staGateway = 0;
      if (count > 2  ) {
        errIndex = 2;
        errCode = etExtraParam;
      }

    } else if (count < 4) {
      errCode = etMissingParam;

    } else {
      IPAddress ipa;
      IPAddress gateway;
      IPAddress mask;
      if (!ipa.fromString(token[1])) {
        errIndex = 1;
      } else if (!gateway.fromString(token[2])) {
        errIndex = 2;
      } else if (!mask.fromString(token[3])) {
        errIndex = 3;
      }
      if ((ipa & mask) != (gateway & mask)) {
        addToLogP(LOG_ERR, TAG_COMMAND, PSTR("The station IP and gateway are not on the same subnet"));
        return etNone;
      }
      if (errIndex)
        errCode = etInvalidValue;
      else {
        // only update values if all three iP are valid
        config.staStaticIP = ipa;
        config.staGateway = gateway;
        config.staNetmask = mask;
        if (count > 4) {
          errCode = etExtraParam;
          errIndex = 4;
        }
      }
    }
  }
  // show current station static ip configuration
  if (config.staStaticIP)
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Static station IP %s, gateway %s and netmask %s"),
      IPAddress(config.staStaticIP).toString().c_str(),
      IPAddress(config.staGateway).toString().c_str(),
      IPAddress(config.staNetmask).toString().c_str());
  else
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Using DHCP obtained IP address"));
  if (count > 1)
    addToLogP(LOG_INFO, TAG_COMMAND, PSTR("Any change will take effect on the next connection to the network"));

  return errCode;
}


#define APP_NAME "Firmware"
//
//     1      2  <<< count
//     0      1  <<< index
//  status  extra
//
cmndError_t doStatus(int count, int &errIndex) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("%s version %s"), APP_NAME, FirmwareVersionToString().c_str());
  wifiLogStatus();
  mqttLogStatus();
  if (count > 1)  {
    errIndex = 1;
    return etExtraParam;
  }
  return etNone;
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


//   1    2                2                 3       4  <<< count
//   0    1                1                 2       3 <<< errIndex
// time [-d] | [(poll|update|http|ap) [<ms>]]  xtra
//
cmndError_t doTime(int count, int &errIndex) {

  if (count < 2) {
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Times in ms: poll = %d, update = %d, http = %d, ap = %d"),
      config.hdwPollTime, config.sensorUpdtTime, config.dmtzReqTimeout, config.apDelayTime);
    return etNone;
  }

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


  } else if (token[1].equals("http")) {
    if (count > 2) { config.dmtzReqTimeout = aTime; }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("HTTP connect timeout: %d ms"), config.dmtzReqTimeout);

  } else if (token[1].equals("update")) {
    if (count > 2) { config.sensorUpdtTime = aTime; }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Sensor data update time: %d ms"), config.sensorUpdtTime);


  } else if (token[1].equals("ap")) {
    if (count > 2) { config.apDelayTime = aTime; }
    addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Disconnect delay before starting the access point: %d ms"), config.apDelayTime);

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



//   1    2    3               2            3         4 <<< count
//   0    1    2               1            2         3 <<< errIndex
// topic [-d] xtra1 | [(log|cmd|pub|sub) [<topic>]]  xtra2
//
void showTopics(void) {
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("MQTT topics: log = \"%s\", cmd = \"%s\", Domoticz pub = \"%s\" and sub = \"%s\""),
  config.topicLog, config.topicCmd, config.topicDmtzPub, config.topicDmtzSub);
}

cmndError_t doTopic(int count, int &errIndex) {
  if (count < 2) {
    showTopics();
    return etNone;
  }

  token[1].toLowerCase();

  if (token[1].equals("-d")) {
    defaultTopics();
    showTopics();
    if (count > 2) {
      errIndex = 2;
      return etExtraParam;
    }
    return etNone;
  }

  errIndex = 1;
  int branch = -1;
  if (token[1].equals("log"))
    branch = 0;
  else if (token[1].equals("cmd"))
    branch = 1;
  else if (token[1].equals("pub"))
    branch = 2;
  else if (token[1].equals("sub"))
    branch = 3;
  else {
    return etUnknownParam;
  }
  errIndex++;
  switch (branch) {
    case 0:
     if (count > 2)
        strlcpy(config.topicLog, token[2].c_str(), MQTT_TOPIC_SZ);
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Log topic: \"%s\""), config.topicLog);
      break;
    case 1:
     if (count > 2)
        strlcpy(config.topicCmd, token[2].c_str(), MQTT_TOPIC_SZ);
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Cmd topic: \"%s\""), config.topicCmd);
      break;
    case 2:
     if (count > 2)
        strlcpy(config.topicDmtzPub, token[2].c_str(), MQTT_TOPIC_SZ);
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Pub topic: \"%s\""), config.topicDmtzPub);
      break;
    case 3:
      if (count > 2) {
        strlcpy(config.topicDmtzSub, token[2].c_str(), MQTT_TOPIC_SZ);
        mqttDisconnect();  // so that the new pub topic is used
      }
      addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Pub topic: \"%s\""), config.topicDmtzPub);
      break;
    default:
      addToLogP(LOG_ERR, TAG_COMMAND, PSTR("Parsing ERROR"));
  }
  if (count > 2)
    errIndex++;
  if (count > errIndex)
    return etExtraParam;
  return etNone;
}


//   1     2     3        2      3         4      <<< count
//   0     1     2        1      2         3      <<< index
//  wifi  [-d] xtra1 | [<ssid> [<pswd>]]  xtra2
//
cmndError_t doWifi(int count, int &errIndex) {
  errIndex = 2; // assume xtra1

  if (count > 1) {
    if ((token[1].equals("-d")) || token[1].equals("-D"))
      defaultWifiNetwork();
    else {
      if (count < 3)
        token[2] = "";
      else if (token[2].length() < 8)
        return etInvalidValue;
      errIndex = 3; // assume xtra2
      strlcpy(config.wifiSsid, token[1].c_str(), HOST_SZ);
      strlcpy(config.wifiPswd, token[2].c_str(), PSWD_SZ);
    }
  }

  token[2] = config.wifiPswd;
  if (token[2].length() < 1)
    token[2] = "<none>";
  else if (token[2].length() < 8)
    token[2] = "<too short>";
  else for (int i=0; i < token[2].length(); i++)
    token[2].setCharAt(i, '*');
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Wi-Fi SSID: %s, password: %s"), config.wifiSsid, token[2].c_str());

  if (count > errIndex)
    return etExtraParam;
  return etNone;
}


// --------------------------------------------------


typedef cmndError_t (*dofnct)(const int, int&);

dofnct doFunctionList[COMMAND_COUNT] = {
  doAp,
  doApip,
  doConfig,
  doDmtz,
  doHelp,
  doIdx,
  doLog,
  doMqtt,
  doName,
  doRestart,
  doStaip,
  doStatus,
  doSyslog,
  doTime,
  doTopic,
  doWifi
};

void exec(String command) {
  cmndError_t error = etNone;
  int count = parseString(command);  // fill tokens
  if (count < 1) return;

  addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("exec %s"), command.c_str());

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
  addToLogPf(LOG_INFO, TAG_COMMAND, PSTR("Command from %s: %s"), cmdsrc[source], cmnd.c_str());
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
      exec(command);
    }
  }
  if (!n) addToLogP(LOG_DEBUG, TAG_COMMAND, PSTR("no commands"));
}

void doCommand(cmndSource_t source, const char *cmnd) {
  doCommand(source, String(cmnd));
}
