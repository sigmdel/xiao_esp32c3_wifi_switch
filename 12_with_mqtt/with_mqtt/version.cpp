/*
Semantic Versioning 2.0.0
https://semver.org/

    Given a version number MAJOR.MINOR.PATCH, increment the:

    MAJOR version when you make incompatible API changes
    MINOR version when you add functionality in a backward compatible manner
    PATCH version when you make backward compatible bug fixes

    Additional labels for pre-release and build metadata are available as extensions to the MAJOR.MINOR.PATCH format.

(Microsof .NET) Version Constructors
https://docs.microsoft.com/en-us/dotnet/api/system.version.-ctor?view=net-6.0

    Major: Versions of the firmware with different major versions are not interchangeable.
           A higher version number indicates a major rewrite where backward compatibility
           cannot be assumed.

    Minor: If the major version number is the same but the but the minor version number is
           different, this indicates significant change but backward compatibility is
           guaranteed.

    Build: Assemblies with the same major, and minor version numbers but different build number
              should be fully interchangeable. A higher build number might be used when fixing a
              bug.
*/

#include <Arduino.h>
#include "version.h"

Version_t Version(VERSION);   // set the current firmware version

Version_t::Version_t(String value) {
  fromString(value);
}

void Version_t::fromString(String value) {
  int n = sscanf(value.c_str(), "%d.%d.%d", &major, &minor, &patch);
  if (n < 3) patch = 0;
  if (n < 2) minor = 0;
  if (n < 1) major = 0;
}

String Version_t::toString() {
    String str(major);
    str += '.' + String(minor);
    str += '.' + String(patch);
    return str;
}

bool Version_t::operator<(const Version_t &otherVersion) {
  if (major < otherVersion.major)
    return true;
  else if (major > otherVersion.major)
    return false;
  if (minor < otherVersion.minor)
    return true;
  else if (minor > otherVersion.minor)
    return false;
  if(patch < otherVersion.patch)
    return true;
  else
    return false;
}

String FirmwareVersion(void) {
  String str = Version.toString();
  str += " (" + String(__DATE__);
  str += " " + String(__TIME__) + ")";
  str.replace("  ", " ");
  return str;
}
