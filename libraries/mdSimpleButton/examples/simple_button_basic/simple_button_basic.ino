/*
 * simple_button_basic.ino
 * 
 * Example program for the mdSimpleButton library
 * <https://github.com/sigmdel/mdSimpleButton>
 *
 */

// SPDX-License-Identifier: 0BSD 

#include <Arduino.h>           // for PlatformIO
#include "mdSimpleButton.h"

#define SERIAL_BAUD 9600
#define BUTTON_PIN 3  
#define ACTIVE LOW

// ---------------------------------------------------


#if (ACTIVE == HIGH) 
  // Connection of an active HIGH normally open button would be
  //                         _________________
  //           __||__       |
  // Vcc ------o    o--+---=|BUTTON_PIN (GPIO)
  //                   |    |________________
  //         ____      |   
  // GND ---[____]-----+  External pull-down resistor (optional in some cases)
  //              
  // If there's no external pull-down resistor, set useInternalPullResistor to true
  // Many micro-controllers do not have internal pull-down resistors 
  //
  mdSimpleButton button = mdSimpleButton(BUTTON_PIN, false, true);
#else  
  // Connection of an active LOW normally open button would be
  //                         ________________
  //           __||__       |
  // GND ------o    o--+---=|BUTTON_PIN (GPIO)
  //                   |    |________________
  //         ____      |   
  // Vcc ---[____]-----+  External pull-up resistor (optional in most cases)
  //              
  // If there's no external pull-up resistor, set useInternalPullResistor to true
  // Most micro-controlers have internal pull-up resistors
  // 
  // Active LOW normally open push button without an external pull-up resistor
  // is the default connection
  //
  mdSimpleButton button = mdSimpleButton(BUTTON_PIN);
#endif


void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10000);
  #if defined(ESP8266)
    Serial.println("\n"); // skip boot garbage
  #endif   

  Serial.print("mdSimpleButton library version ");
  Serial.print(button.version() >> 16);
  Serial.print(".");
  Serial.print( (button.version() & 0x00FF00) >> 8);
  Serial.print(".");
  Serial.println( (button.version() & 0x0000FF));
      
  Serial.println("setup() completed.");
}

bool countsChanged = false;
int pressedCount = 0;
int releasedCount = 0;

void loop() {
  switch (button.update()) {
    case BUTTON_UNCHANGED:
      /* ignore this case */; 
      break;
      
    case BUTTON_PRESSED:  
      pressedCount++;
      countsChanged = true;
      Serial.println("\nButton pressed"); 
      break;
     
    case BUTTON_RELEASED:
      releasedCount++;
      countsChanged = true;
      Serial.print("Button released after "); 
      Serial.print(button.presstime);
      Serial.println(" ms");
      break;   
     
    case BUTTON_LONGPRESS:
      releasedCount++;
      countsChanged = true;
      Serial.print("Button released after long press of "); 
      Serial.print(button.presstime);
      Serial.println(" ms");
      break;
     
    default: 
      Serial.println("*** SOMETHING HORRIBLY WRONG HAS HAPPENED ***"); 
      break;
  }
  delay(40 + random(21)); // rest of loop takes 40 to 60 ms to execute 

  if (countsChanged) {
    countsChanged = false;
    Serial.print("  pressedCount = ");
    Serial.println(pressedCount);
    Serial.print("  releasedCount = ");
    Serial.println(releasedCount);
  }
}
