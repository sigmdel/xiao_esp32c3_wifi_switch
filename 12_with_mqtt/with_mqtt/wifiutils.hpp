// wifiutils.hpp

#pragma once

// Reports the Wi-FI network status to the log
void wifiLogStatus(void);

// Attempt to connect to the specified Wi-Fi network.
//   If config.wifiSsid == "" then will do nothing
//   Will set a static ip address if config.staStaticIP != 0.0.0.0
void wifiConnect();

// Disconnects from the wireless network, flushing the log before.
//   Does nothin if Wi-Fi is not connected
void wifiDisconnect();

//  Checks if state of the Wi-Fi connection has changed.
//    Starts an access point if disconnected for config.apDelayTime or longer
//    Stops the access point when the Wi-Fi connection is re-established
//    Call from loop()
void wifiLoop(void);

