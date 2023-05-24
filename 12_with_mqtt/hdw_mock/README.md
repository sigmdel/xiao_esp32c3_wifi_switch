# Mock Hardware Drivers

Emulates the brightness and temperature & humidity sensors which facilitates
running the firmware on other ESP32 development boards without having to 
marshall extra hardware. However, it is assumed that the development card has 
a built-in LED or an external LED to emulate the relay and a push button.

Set the I/O pin assignment directly in `hdw_mock\hardware.cpp` or override the
default RELAY_PIN and BUTTON_PIN assignments in `hdw_mock\hardware.cpp` in
`platformio.ini`

```ini
build_flags =
  -DRELAY_PIN=11        ; RGB RED PIN
  -DBUTTON_PIN=37
...
```  

or in `arduino_config.h` 

```cpp
#if (!PLATFORMIO)
  #define RELAY_PIN  11      // RGB_RED_PIN
  #define BUTTON_PIN 37
...
#endif
```
depending on which development environment is used.
