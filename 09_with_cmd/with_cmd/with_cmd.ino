/*
 * with_cmd.ino
 *
 * Web Interface for a Relay with Additional Sensors
 * integrated into Domoticz home automation
 *
 * New: 
 *   command processor
 *     
 * 
 * Web update technology: Server-Sent Events (SSE)
 *
 * Hardware: XIAO ESP32C3 with XIAO Starter Kit
 *
 * 
 * This is a stub to satisfy the Arduino IDE, the source code is in
 * the file main.cpp in the same directory.
 *
 * This sketch will compile in the Arduino IDE and in PlatformIO
 *
 * Michel Deslierres
 * April 2, 2023
 *
 *
 * Copyright 2023, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html>
 */

// SPDX-License-Identifier: 0BSD

/*********************************************************************************************************************************
Note on libraries (AsyncTCP, ESPAsyncWebServer,  DFRobot_DHT20, SimpleDHT, and mdSimpleButton)

Placing AsyncTCP and ESPAsyncWebServer in a ./scr/ subdirectory did not work in Arduino. The many #include <AsyncTCP.h> in
EPSAsyncWebServer could not be resolved. The solution was to save both libraries in a directory named "../../libraries" as a sibling of
the 01_simplified_hdw_version and then to make their common parent directory the Sketchbook location in the Arduino Preferences.

The other libraries were placed in the same "libraries" directory. Note that DFRobot_DHT20 is a fork of the original in which
the getTempAndHumidity() method was added.

In both Arduino and PlatformIO, private copies of the AsyncTCP and ESPAsyncWebServer must be used for two reasons.

1. The very latest version of the me-no-dev/ESP Async WebServer must be used. The latest stable version (1.2.3) obtained with
   the PIO Libraries manager is incompatible with the latest version of md5.h.  This can be fixed by getting the development branch
   https://github.com/me-no-dev/ESPAsyncWebServer.git
   Ref: https://github.com/me-no-dev/ESPAsyncWebServer/issues/1147

2. The IPAddress AsyncWebSocketClient::remoteIP() in .pio/libdeps/seeed_xiao_esp32c3/ESP Async WebServer/src/AsyncWebSocket.cpp
   must be edited. The 0 null address returned if a client is not defined must be typecaset to uint32_t.
      IPAddress AsyncWebSocketClient::remoteIP() {
        if(!_client) {
            return IPAddress((uint32_t) 0U);
        }
        return _client->remoteIP();
      }
   Reference: https://github.com/me-no-dev/ESPAsyncWebServer/issues/1164

Note that the Arduino library manager uses a fork of the me-no-dev repo,
https://github.com/dvarrel/ESPAsyncWebSrv by dam74 (dvarrel) which updates the version number but does not solve
the remoteIP problem.

***********************************************************************************************************************************/


