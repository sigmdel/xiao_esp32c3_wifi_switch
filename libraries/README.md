# Private Repository of Needed Libraries

This directory contains the following libraries used in some or all of the projects.

  - me-no-dev/AsyncTCP@^1.1.1
  - blanchon/ArduinoJson@^6.17.2
  -	ayushsharma82/AsyncElegantOTA@^2.2.7;
  -	me-no-dev/ESPAsyncTCP@^1.2.2
  -	me-no-dev/ESP Async WebServer@^1.2.3     (§)
  -	knolleary/PubSubClient@^2.8
  -	winlinvip/SimpleDHT@^1.0.15
  -	dfrobot/DFRobot_DHT20@^1.0.0             (*)
  -	https://github.com/sigmdel/mdSimpleButton@^0.2.0

With this addition, all the projects are self-contained and will compile without
adding library dependencies in PlatformIO. Similarly, all sketches will compile
without installing libraries in the Arduino IDE, but it is necessary to set the `Sketchbook location` (in `Preferences`) to the parent library of `libraries`. To belabour the point, assuming the full path of the libraries directory is

`/home/me_me/Documents/Projects/esp32c3/wifi_switch\libraries` 

then `Sketchbook location` must be set to

`/home/me_me/Documents/Projects/esp32c3/wifi_switch\`.

(§) The current version of [`ESP Async WebServer`](https://github.com/me-no-dev/ESPAsyncWebServer) by Hristo Gochckov will not compile with the ESP32-C3. The version in this directory has been modified and will compile. Version 1.2.7 of the [`ESP Async WebServer fork`](https://github.com/dvarrel/ESPAsyncWebSrv) by dam74 (dvarrel) should work with the ESP32-C3, although it has not been tried here.

(*) [`DFRobot_DHT20`](https://github.com/DFRobot/DFRobot_DHT20) has been modified to obtain both the temperature and humidity values of the sensor with one reading.
