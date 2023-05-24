// Arduino configuration

#if (!PLATFORMIO)
   #define NO_TESTS          // Undef all TEST_xxxx macros
// overrides for hdw_mock/hardware.cpp I/O pin assignments   
//   #define RELAY_PIN  11      // RGB_RED_PIN
//   #define BUTTON_PIN 37
#endif
