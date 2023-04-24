// wifiutils.hpp

#pragma once

void wifiConnect(String ssid, String pswd = "");
void wifiDisconnect(void);
void wifiLogStatus(void);
void wifiReconnect(void);
