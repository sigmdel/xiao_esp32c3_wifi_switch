# Alternate Hardware Configuration 

This implementation of `hardware.cpp` assumes generic type sensors are used in conjuction with the XIAO ESP32C3 board.

  - The brightness sensor is a light dependant resistor (LDR).
  
  - The temperature & humidity sensor can be a DHT11 or DHT22.

  - Implements a rolling average measure of the brightness if LS_FIFO > 1.
  
  - Implements a constant verification of the 1 Wire connection to a DHT11 or DHT22.

If averaging the light level measurements, set the interval between readings of the sensor
with the `LS_READ` macro. It makes no sense to set a shorter `LS_READ` time interval
than `config.hdwPollTime`.  

If averaging brightness measurements makes sense, then it could be used for the temperature and humidity measurements also. In any event, if averaging were to be included in the final product, then consider changing the `LS_READ` macro to a user-settable configuration variable.

See [Alternate Hardware](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html#alt_hardware) 
for details about the generic hardware.
