#pragma once

#include <Arduino.h>

/*
    Major: Versions of the firmware with different major versions are not interchangeable.
           A higher version number indicates a major rewrite where backward compatibility
           cannot be assumed.

    Minor: If the major version number is the same but the but the minor version number is
           different, this indicates significant change but backward compatibility is
           guaranteed.

    Build: Assemblies with the same major, and minor version numbers but different build number
              should be fully interchangeable. A higher build number might be used when fixing a
              bug.

  https://docs.microsoft.com/en-us/dotnet/api/system.version.-ctor?view=net-6.0
*/

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_BUILD 1

union Version_t {
  uint8_t Data[3];
  uint32_t Version;
};

String VersionToString(Version_t aVersion);
String FirmwareVersionToString(void);


extern Version_t firmwareVersion;
