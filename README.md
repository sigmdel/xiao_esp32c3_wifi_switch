# xiao_esp32c3_wifi_switch

***XIAO ESP32C3 based Wi-Fi Switch*** (*Version 0.0.7*)

Source code that accompanies **A Wi-Fi Switch for Domoticz using a XIAO ESP32C3:**

   + [*Part 1 - Demonstration Projects*](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html)
   + [*Part 2 - Asynchronious Web Page Updates*](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_2_en.html)
   + *Part 3 - Better User Experience...* 
   + [*Part 4 - Commands - version 0.0.6*](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_4_en.html)

## Overview of the Wi-Fi Switch

The goal is to try to reproduce the basic functionality of Theo Arends' powerful [Tasmota](https://github.com/arendst/Tasmota) firmware which is used in many home automation devices. This is very ambitious, and consequently a step-by-step approach will be adopted. The end result will not be equivalent to Tasmota by any means but hopefully something will have been learned.

The image below shows the interaction between the microcontroller board, a XIAO ESP32C3, with an attached push button, a light level sensor, and a temperature and humidity sensor. As can be seen, it is connected to web browsers and a Domoticz home automation server.

![Landscape](img/xiao_esp32c3_in_ha.jpg)

 It's not just a matter of toggling a LED on and off with a button on a Web page; numerous examples can be found on the Web. Whenever the state of the light is changed locally with the button, the light's status has to be updated on the Web page displayed by all clients connected to the Web server and in the home automation system. Similarly, if the toggle button on a client's Web page is clicked, then the hardware controlling the light must be activated accordingly and the light's status must be updated in the home automation system and on all connected clients' Web page simultaneously. Likewise, if the virtual light switch in the home automation system is toggled on or off, the actual relay on the Wi-Fi switch must be updated and the new status of the light must be shown on all connected clients' Web pages.


## Projects 

All projects are independent and self-contained.

Private copies of the needed libraries are provided in the shared [`libraries`](libraries) directory, so it should be possible to compile all the projects without installing any library.

All projects will compile and run in the PlatformIO and Arduino IDEs as long as the ESP32 platform in installed and the correct board is selected. Details are in section 6 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 1](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en#compile1.html). 

 
### Opening a project in PlatformIO

Open a PlatformIO project by clicking on the Quick Access `Open Project` button in the Home page of PlatformIO. Then navigate to the project directory (such as `01_simplified_hdw_version`) which contains the project configuration file `platformio.ini` and click on the `Open "&lt;project&nbsp;name&gt;".

### Opening a sketch in Arduino

Open an Arduino sketch by clicking on the `File`/`Open` menu. Then navigate to the sketch file (such as `01_simplified_hdw_version\simple_wifi_switch\simple_wifi_switch.ino`) and then click on the `Open` button in the file open dialog window.


## 01_simplified_hdw_version

In this first iteration of the ESP32-C3 firmware, a hardware abstraction layer is built to handle 
  - a normally open push button,
  - a DHT20 temperature and humidity sensor connected over the I²C serial bus,
  - a light sensor based on the LS06-S.

A future hardware layer [03_alt_wifi_switch](03_alt_wifi_switch) includes support for more types of sensors.

The asynchronous web server is capable of handling multiple clients at the same time. Each Web client reloads the Web page every five seconds because the HTML page served has a refresh meta tag. 

This project corresponds to sections 2 to 5 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 1](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html).


## 02_basic_wifi_switch

Integration of the Wi-Fi switch into the home automation system Domoticz is added in this second iteration of the ESP32-C3 firmware. Communication with the Domoticz server is done with HTTP requests.

Domoticz can be used to easily modify the behaviour of the Wi-Fi switch so that it becomes a timed light which stays off after a given period of time or an auxiliary light that gets turned on and off whenever another Domoticz controlled switch is turned on or off. It is also very easy to transform the Wi-Fi switch into a night light that turns on at dusk and back off at sunrise in Domoticz. The [../lua](../lua/) directory contains a dzVents Lua script that turns the Wi-Fi switch on or off based on the light level measured by the on-board sensor.

The [../lua](../lua/) directory contains a dzVents Lua script that automatically adjusts the humidity status displayed for all Domoticz supported devices that measure temperature and humidity and optionally barometric pressure. 

This project corresponds to sections 7 and 8 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 1](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html).


## 03_alt_wifi_switch

This third project is project 2 but with a different hardware abstraction layer. It can handle three types of temperature and humidity sensors (DHT11, DHT20 or DHT22) and two types of light level sensors (LDR or LS06-S photodiode). Additionally, a temperature and humidity sensor can be emulated and the same is possible with the light level sensor. Finally, a rolling average of the light level measurements can be enabled to smooth out fluctuations caused by clouds or other passing shadows. 

This project corresponds to section 10 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 1](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html).


## 04_ajax_update

Full Web page reloading using a refresh meta tag is replaced by asynchronous JavaScript and XML (AJAX) based Web page updates.

This project is described in section 1 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 2 (*Asynchronious Web Page Updates*)](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_2_en.html).


## 05_websocket_update

Full Web page reloading using a refresh meta tag is replaced by asynchronous websocket based Web page updates.

This project is described in section 2 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 2 (*Asynchronious Web Page Updates*)](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_2_en.html).

## 06_sse_update

Full Web page reloading using a refresh meta tag is replaced by asynchronous Server-Sent Events (SSE) based Web page updates.


This project is described in section 3 of [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 2 (*Asynchronious Web Page Updates*)](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_2_en.html)

> ### About the last 3 xxx_update projects
>
> They present 3 techniques that could be used to update the information displayed on the Web page without reloading the page itself as done in the first three projects. While Tasmota uses AJAX, and Websockets probably represent the most powerful technique, Server-Sent Events and AJAX will be used starting in <code>08_ticker_hdw</code>.

<!-- 
https://stackoverflow.com/questions/5195452/websockets-vs-server-sent-events-eventsource/5326159
https://stackoverflow.com/questions/5195452/websockets-vs-server-sent-events-eventsource

https://web.dev/eventsource-basics/
Stream Updates with Server-Sent Events
Nov 30, 2010
Eric Bidelman
-->

## 07_with_log

Carrying on from `06_sse_update`, this version adds a *private* logging facility. It is implemented as a FIFO queue with replacement of older entries when adding a log message when the queue is already full. That way, it is possible to log messages in an interrupt service routine and even before the serial port is up. Actually sending log messages is done safely in the `loop()` thread. Started to remove blocking operations. There is no longer any waiting for the WiFi connection in `setup()`. Similarly failed initialization of the temperature sensor will no longer block the execution of the firmware. 

There is still a problem with HTTP requests to a missing Domoticz server or to an incorrect address. Request will still block until a timeout is reached. Currently, the timeout is set at the lowest possible value (1 second), so firmware will not respond to the button during that time. The solution would seem to be to find an asynchronous HTTP request library or to make each request in a separate thread or task.

## 08_ticker_hdw

No luck so far with asynchronous HTTP request libraries. Tests of embedding a blocking HTTP request in a (RTOS) task are positive. However, if tasks are used for this, they should be used for everything else including button presses. Consequently, another approach is used: HTTP requests are buffered in a FIFO queue with replacement. This is the same technique as used with the log. To see how that works, ensure that the serial log level is set to LOG_DEBUG (recompile and upload the firmware if the level had to be changed), stop Domoticz, and press the button many times in quick succession. Quickly, the buffer will be filled and old requests will be dropped. Nevertheless, the LED will follow the state of the button because the hardware is now polled with a `Ticker`. Every 25 milliseconds (`POLL_TIME` is set in `hardware.h`), a hardware timer interrupt takes care of checking the state of the button and the sensors if need be. Since this is an interrupt, the button state is monitored even as an HTTP request might be in progress.

The alternate hardware drivers, first shown in `03_alt_wifi_switch` have been updated and are contained in `hdw_alt/hardware.cpp`. This is a drop-in replacement for `ticker_hdw/hardware.cpp`. Note that now change is requires to the header file `hardware.h`.

## 09_with_cmd

This version continues segregating all configuration values that can be modified by the user at run-time into `config.h` and `config.cpp`. As before the default values are in `user_config.h` which is not included in the repository. Use `user_config.h.template` as a model.

The important addition in this version is a command interpreter in `commands.hpp` and `commands.cpp`. Currently commands, which for the most part consist of viewing or changing the configuration at run-time, can be entered in the serial console or in the Web interface console.  There will not be a `Backlog` command à la Tasmota. Instead, multiple commands can be specified at once as long as they are separated by a ";".

```
00:02:53.737 CMD/inf: Command from webc: help config; config; idx; version
00:02:53.737 CMD/inf: config [load|default|save [force]] | [auto (off|on)]
00:02:53.738 CMD/inf: Config version: 1, size: 548, auto(save): on
00:02:53.738 CMD/inf: Domoticz virtual Idx: switch = 1, light sensor = 3, temp + humid sensor = 2
00:02:53.738 CMD/inf: Firmware version 0.0.5 (Apr 26 2023 21:51:48)
```

This first version of the command interpreter is just a *proof of concept* and it will not be truly operational until the configuration can be saved to non-volatile memory and read back from it when the ESP32C3 boots. Right now, that seems the simplest way of a  change from dynamic IP address to a static IP address.

## 10_with_config

The user-defined configuration can be saved to or loaded from non-volatile storage (NVS). With improvements and a revamped ESP restart command, the command interpreter is now working properly. In version 0.0.6, the new unit `wifiutils` which takes care of the network details has been simplified. 

Network configuration is handled with two commands: a new `wifi` command used to specify the Wi-Fi access point and the `staip` command to choose between getting a dynamic IP address from the network DHCP server or using a static IP address. Changes to the configuration with these two commands have no effect until the device is restarted. There is no reason to keep `secrets.h` and `secrets.h.template` since network credentials are now included in the configuration . 

The list of command and their syntax can be found in [A Wi-Fi Switch for Domoticz using a XIAO ESP32C3 - Part 4: Commands](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_4_en.html). This is still very much a work in progress, so commands will be added and changes to existing commands may be made. Also, the command interpreter needs much reworking. It's as if each command is parsed with a different approach in an attempt to find a common approach that could be used in all cases. 

Added the <a href="https://github.com/ayushsharma82/AsyncElegantOTA" target="_blank">AsyncElegantOTA</a> library by Ayush Sharma. It is not how I usually handle over-the-air updates, but it was very simple to add and does handle authentication if that is required. 

## 11_with_wm

A Wi-Fi manager is added. Well, that's a bit pretentious; only an alternate root Web is provided. When the Wi-Fi switch has lost the connection to the wireless network for more than a specified time interval (5 minutes as defined in the default user configuration), it starts an access point. It will be necessary to log onto that new Wi-Fi network, named <code>KITCHENLIGHT-AP</code> with <code>12345678</code> as a password, to open the Web server to get access to a form used to specify the Wi-Fi credentials and from there try to connect to the Wi-Fi network. As soon as a Wi-Fi connection is established, the access point is brought down.

This required adding two new commands 

  - <code>ap</code> to manage the access point name and password
  - <code>apip</code> to manage the access point IP address and subnet mask

and modifying the <code>time</code> command, adding the <code>ap</code> parameter to set the disconnection time interval before starting the access point.

The corresponding fields had to be added to the configuration structure <codd>config_t</code>. Also, three Web pages were added in <code>html.h</code> not just one and two functions were appended to <code>wifiutils</code> to start and stop the access point. Starting and stopping the access point is done in the <code>WiFiModule</code> in <code>main.cpp</code>.

When connected to the wireless network started by the Wi-Fi switch, it is possible to get access to the main Web page at <code>192.168.4.1/index.html</code> (or whatever the AP's IP address is). That way, the relay can be turned on or off, the sensor values will be updated and the firmware can be updated. Unfortunately, the console is not updated although commands entered at the console do work. If one prefers to have a completely separate Wi-Fi manager, then look at <code>this does not seem right, then look at <code>webserver_xl/webserver.cpp</code>.

For some reason, it is very difficult to connect to the access point with my desktop (Linux Mint 20.1 with a 5.15.0-72-generic kernel) if the wireless interface has been connected at a Wi-Fi network before hand. Turning off and then back on the radio usually solves that problem.

```bash
$ sudo nmcli radio wifi off
$ sudo lshw -C network       
$ sudo nmcli radio wifi on
```

The <code>lshw</code> is used to verify that the interface is actually off. Interestingly, that problem is not encountered with Android devices.


## Upcoming

With version 0.0.7 (11_with_wm), the project has attained a level such that it could be used as is if the Wi-Fi switch can be given a static IP address. This is not always practical, but using dynamic IP address will break the Domoticz on and off actions for the relay when, inevitably, the DHCP server assigns a different IP address to the XIAO. The obvious solution is to communicate with Domoticz with MQTT. So that will be the next step.

## License

Copyright 2023, Michel Deslierres. No rights reserved. 

While the copyright pertaining to included libraries by others must be respected, all the source code in this repository created by Michel Deslierres is in the public domain. In those jurisdictions where this may be a problem the [BSD Zero Clause License](https://spdx.org/licenses/0BSD.html) applies.
