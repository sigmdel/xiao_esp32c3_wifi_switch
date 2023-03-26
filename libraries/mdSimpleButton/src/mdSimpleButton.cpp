/*
 * mdSimpleButton.cpp
 * See mdSimpleButton.h for description, license, etc.
 */

#include <Arduino.h>
#include "mdSimpleButton.h"

#define VERSION 0x000200  // 0.2.0

mdSimpleButton::mdSimpleButton(uint8_t pin, bool activeLow, bool useInternalPullResistor, buttonCallback cb) {
  Serial.println("Constructor");
  _pin = pin;
  _restState = activeLow;
  _active = false;
  _onEvent = cb;
  _debouncing = false;
  debounce = DEBOUNCE_TIME; 
  longpress = LONGPRESS_TIME; 

  int mode = INPUT;
  if (useInternalPullResistor) {
    if (activeLow)
      mode = INPUT_PULLUP;
    else {
      // mode = INPUT_PULLDOWN where defined
      #if defined(INPUT_PULLDOWN)
        mode = INPUT_PULLDOWN;
      #elif defined(ESP8266)
        if (pin == 16) mode = INPUT_PULLDOWN_16;
      #endif
    }
  }
  pinMode(_pin, mode);
  _version = VERSION;
}

int32_t mdSimpleButton::version(void) {
  return _version;
}

void mdSimpleButton::onButtonEvent(buttonCallback cb) {
  _onEvent = cb;
}

buttonEvent mdSimpleButton::update(void) {
  if (_debouncing) {
    if (millis() - _timer < debounce)
      return BUTTON_UNCHANGED;
    else  
      _debouncing = false;
  }
  if ((digitalRead(_pin) != _restState) && (!_active)) {
    _active = true;
    _debouncing = true;
    _timer = millis();
    if (_onEvent) _onEvent(this, BUTTON_PRESSED);
    return BUTTON_PRESSED;
  }
  if ((digitalRead(_pin) == _restState) && (_active)) {
    _active = false;
    presstime = (millis() - _timer);
    buttonEvent event = (presstime < longpress) ? BUTTON_RELEASED : BUTTON_LONGPRESS;
    if (_onEvent) _onEvent(this, event);
    return event;
  }
  return BUTTON_UNCHANGED;
}
