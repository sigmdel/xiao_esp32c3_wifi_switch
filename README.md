# xiao_esp32c3_wifi_switch

XIAO ESP32C3 base Wi-Fi Switch

**Source code that accompanies the March 2023 version [A Wi-Fi Switch for Domoticz using XIAO ESP32C3](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_en.html)**

## Overview of the Wi-Fi Switch

The goal is to try to reproduce the basic functionality of Theo Arends' powerful Tasmota software which is used in many home automation devices. Admitidly, this is an ambitious goal, and it will only achieved in a number of steps and even then the result will be far less accomplished. The image below shows the interaction between the microcontroler board, a XIAO ESP32C3, with attached push button, light level, and temperature and humidity sensors and connected to web browsers and a Domoticz home automation server.

![Landscape](img/xiao_esp32c3_in_ha.jpg)

 It's not just a matter of toggling a LED on and off with a button on a Web page; numerous examples can be found on the Web. Whenever the state of the light is changed locally with the button, the light's status has to be updated on the Web page displayed by all clients connected to the Web server and in the home automation system. Similarly, if the toggle button on a client's Web page is clicked, then the hardware controlling the light must be activated accordingly and the light's status must be updated in the home automation system and on all connected clients' Web page simultaneously. Likewise, if the virtual light switch in the home automation system is toggled on or off, the actual relay on the Wi-Fi switch must be updated and the new status of the light must be shown on all connected clients' Web pages.
