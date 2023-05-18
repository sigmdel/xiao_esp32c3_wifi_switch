// wifiutils.hpp

#pragma once

// Reports the Wi-FI network status to the log
void wifiLogStatus(void);

// Attempt to connect to the specified Wi-Fi network. If ssid == "" then will do nothing
// Always connects obtaining a DHCP assigned dynamic address
void wifiConnect();

void startAp(void);
void stopAp(void);
