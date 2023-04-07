// Arduino configuration

#if (!PLATFORMIO)
   #define NO_TESTS          // Undef all TEST_xxxx macros
   //#define USE_SECRETS     // should do only first time around, after that the ESP32 will remember the network cred
#endif

