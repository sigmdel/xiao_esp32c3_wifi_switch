# mdSimpleButton Library

**Version 0.2.0** (March 16, 2023)

An [Arduino](https://www.arduino.cc/) library that returns changes in the state of an on/off button.

## mdSimpleButton Class Constructor

An instance of the `mdSimpleButton` class must be created for each button. 

```cpp
mdSimpleButton(uint8_t pin, bool activeLow = true, bool useInternalPullResistor = true, buttonCallback cb = nullptr);
```

The GPIO pin connected to one terminal of the button must be specified in the constructor. The other parameters of the constructor 
have default values that correspond to a normally open push button with the other pin connected to ground:

```cpp
mdSimpleButton myButton mdSimpleButton(3);
```

When the button is mechanically set to the ON position
the signal from the button transitions from a `HIGH` to a `LOW` value.
Consequently, when the button is returned to the OFF position 
the signal from the button transitions from `LOW` to `HIGH`.
This can be reversed by setting `activeLow` to `false` in which case a 
normally open push button would be connected to Vcc. 

If `useInternalPullResistor` is set to `true`, which is the default, 
then an internal pull-up or pull-down resistor will be enabled. If 
`activeLow = true`, then an internal pull-up resistor will be 
used to keep the GPIO pin `HIGH`  when the button is in the OFF position.
If `activeLow = false`, then an internal pull-down resistor will be enabled to keep the GPIO pin `HIGH` when the button is in the OFF position 
assuming that the microcontroller has pull-down resistors. If 
`useInternalPullResistor` is set to `false` neither pull-up nor pull-down
resistors are enabled. It will probably be necessary to have an external pull-up 
or pull-down resistor connected to the GPIO pin used with the button.

## Polling

The state of the push button is updated when the function 

```cpp
  buttonEvent update(void)
```    
is invoked. This function must be called at regular intervals which
is usually done in the `loop()` function of the sketch. The value 
returned by the function indicates the state of the push button:

```cpp
enum buttonEvent {
  BUTTON_UNCHANGED = 0,   // the button state has not changed 
  BUTTON_PRESSED,         // the button has been put into the ON position
  BUTTON_RELEASED         // the button has been put into the OFF position
};
```

## Callback Functions

Instead of testing the returned value of `update()` to see if the button 
has been pressed or released, a callback function can be specified. 

```cpp
  void buttonPressed(buttenEvent event)
```

This callback is assigned with the `onButtonEvent()` method or 
in the constructor. 

It is still necessary to call `update()` regularly.

# Examples

Hopefully, the examples illustrate how to use this simple library. 
The first, `simple_button_basic.ino` uses the returned value from the
`update()` function directly and must therefore correctly handle a 
returned `BUTTON_UNCHANGED` which indicates that there has been no change in the state of the button.

The second example, `simple_button_callback.ino` is similar but uses a 
callback function which will never have to handle a `BUTTON_UNCHANGED` event. 
Note that `update()` is still called in the `loop()` function. 

# Architecture and Testing

The library should work with most microcontrollers with an Arduino core. However it has only been tested 
with the ESP32-C3 in Arduino-ESP32.

# Credits

Based on [MicroPython-Button](https://github.com/ubidefeo/MicroPython-Button) by Ubi de Feo (ubidefeo)


# Licence

The **BSD Zero Clause** ([SPDX](https://spdx.dev/): [0BSD](https://spdx.org/licenses/0BSD.html)) licence applies.
