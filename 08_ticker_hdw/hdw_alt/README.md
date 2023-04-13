# hdw_alt

This directory contains alternate hardware drivers. The file `hardware.cpp` is drop-in replacement for the one found in the project. See [Alternate Hardware](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_wifi_switch_1_en.html#alt_hardware).

## Supported temperature and humidity sensors:

  + DHT11
  + DHT20
  + DHT22
  + MOCK

Use the 
```cpp
#define THS XXXX
```
macro to specify which sensor is used. If `MOCK` is specified, a temperature and humidity sensor will be emulated.

## Supported light sensors:

  + LS_LDR
  + LS_DIODE
  + MOCK

Use the 
```cpp
#define LS XXXX
```
macro to specify which sensor is used. If `MOCK` is specified, light level sensor will be emulated.

## Light reading averaging

The macro
```cpp
#define LS_FIFO   10
```
sets the size of a FIFO queue to use a rolling average of the light values. Of course if `LS_FIFO` is less than 2 there can be no averaging. If `LS_FIFO` is greater than one, then the macro
```cpp
#define LS_READ   10000
```
defines the number of milliseconds between raw readings of the light level. Of course that value should probably be less than `SENSOR_DELAY` although there are no constraints in that regard.
