#include <Arduino.h>
#include "version.h"

// ESP32 is little endian
Version_t firmwareVersion = { .Data = {VERSION_BUILD, VERSION_MINOR, VERSION_MAJOR}};

String VersionToString(Version_t aVersion) {
  char cstr[12]; //333.333.333
  sprintf(cstr, "%d.%d.%d", aVersion.Data[2], aVersion.Data[1], aVersion.Data[0]);
  return String(cstr);
}

String FirmwareVersionToString(void) {
  //return VersionToString(firmwareVersion);
  String str = VersionToString(firmwareVersion);
  str += " (" + String(__DATE__);
  str += " " + String(__TIME__) + ")";
  str.replace("  ", " ");
  return str;
}
