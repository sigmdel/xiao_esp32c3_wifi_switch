#pragma once

#include <Arduino.h>

#define VERSION "0.0.8"

struct Version_t {
public:
  u_int major, minor, patch;

    // Default constructor which sets the
    // version to 0.0.0
  Version_t() {};

    // Constructor with major.minor.patch string.
    // See fromString
  Version_t(String value);

    // Sets the version numbers from the value string. It should be
    // formated as a dot separated major.minor.patch integers such as
    // Version_t Version("1.2.9");
    //
  void fromString(String value);

    // Returns the current version number as a dot separated string
  String toString();

    // Used to test if a newer version is available
    // Version_t Version("0.1.8");
    // Version_t OtherVersion(someString);
    // if Version<OtherVersion { update to OtherVersion }
    //
  bool operator<(const Version_t &otherVersion);
};

// Current firmware version set from VERSION
//   Version.toString() returns the version as a String
//   Version.toString().c_str() return the version as a C string
extern Version_t Version;


// Returns Version + build date as a string: Example "1.9.2 (Apr 26 2023 16:53:40)"
// NOTE: Do not forget to do a clean build to update the build date returned by FirmwareVersion()
String FirmwareVersion(void);
