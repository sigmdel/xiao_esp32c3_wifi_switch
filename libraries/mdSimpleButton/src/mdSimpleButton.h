/*
 * mdSimpleButton.h
 *
 * A push button Arduino library indicating when a button is pressed
 * or released.
 *
 * Version 0.2.0, 2023-03-16.
 *
 * Michel Deslierres <sigmdel.ca/michel>
 *
 * Based on MicroPython-Button by Ubi de Feo (ubidefeo)
 * @ https://github.com/ubidefeo/MicroPython-Button
 *
 */

 // SPDX-License-Identifier: 0BSD

#pragma once

#include "Arduino.h"

#define DEBOUNCE_TIME    50 // ms of debounce time before accepting release event
#define LONGPRESS_TIME 1000 // ms minimum time between press and release to constitute a long button press

enum buttonEvent {
  BUTTON_UNCHANGED = 0,
  BUTTON_PRESSED,
  BUTTON_RELEASED,
  BUTTON_LONGPRESS
};

class mdSimpleButton; // forward declaration

// Callback type of handler such as buttonClicked(int event)
typedef void (*buttonCallback)(mdSimpleButton* object, buttonEvent);

class mdSimpleButton {
  public:
    //constructor
    //  If activeLow = true, then a button press is a transitiion of the I/O the pin from HIGH to LOW
    //                   and a button release is a transitiion of the I/O the pin from LOW to HIGH
    //  If activeLow = false, then a button press is a transitiion of the I/O the pin from LOW to HIGH
    //                   and a button release is a transitiion of the I/O the pin from HIGH to LOW
    //  If useInternalPullResistor is true then the an internal pull-high resistor will be enabled on the input pin
    //    if the button is active LOW. If the button is active HIGH and if internal pull-down resistors exist then
    //    the internal pull-down resistor to the I/O pin will be enabled
    mdSimpleButton(uint8_t pin, bool activeLow = true, bool useInternalPullResistor = true, buttonCallback cb = nullptr);

    // Set callback routine when the button has been pressed or released
    void onButtonEvent(buttonCallback cb);
     
    // Returns BUTTON_UNCHANGED.. BUTTON_RELEASED, must be called regularly (i.e. in loop())
    buttonEvent update(void);

    uint8_t pin(void) {return _pin;}

    // returns current library version
    int32_t version(void);

    // press key debounce time (ms)
    uint32_t debounce;       // default is DEBOUNCE_TIME

    // minimum key depressed time (ms) to qualify as a long key press
    uint32_t longpress;      // default is LONGPRESS_TIME

    // time the key has been depressed (ms)
    unsigned long presstime;
private:
    uint8_t _pin;
    int _restState; // type matches digitalRead()
    bool _active;
    bool _debouncing;
    unsigned long _timer;
    buttonCallback _onEvent;
    int32_t _version; // Current library version            
};
