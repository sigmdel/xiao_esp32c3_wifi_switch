# Hardware Drivers When Using the Seeed Studio XIAO Starter Kit

The XIAO ESP32C3 is plugged into the [XIAO Expansion base](https://www.seeedstudio.com/Seeeduino-XIAO-Expansion-board-p-4746.html) and the sensors are Grove devices also included in the [XIAO Starter Kit](https://www.seeedstudio.com/Seeed-XIAO-Starter-Kit-p-5378.html).

  - The [temperature & humidity sensor](https://wiki.seeedstudio.com/Grove-Temperature-Humidity-Sensor-DH20/) is an I²C DHT20.

  - The  [light sensor](https://www.seeedstudio.com/Grove-Light-Sensor-v1-2-LS06-S-phototransistor.html) (called the brightness sensor here to distinguish it from the light presumably controlled by the relay) is based on the LS06-S photodiode.

  - An [external LED](https://www.seeedstudio.com/Grove-LED-Pack-p-4364.html) emulates the relay.

There is no particular reason to use these devices, see `hdw-alt/hardware.cpp` where a generic DHT11 or DHT22 can measure temperature and humidity, a light dependant resistor measures the ambient light, and a simple LED with a fixed resistor emulates the relay. 

Brightness measurements are direct with no averaging as opposed to what is done in 

See [Hardware](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html#hardware) for details about the use of the Seeed Studio starter kit and [Alternate Hardware](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html#alt_hardware) in the same post about the generic hardware.

  
