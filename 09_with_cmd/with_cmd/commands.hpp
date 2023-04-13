#pragma once

enum cmndError_t {etNone, etMissingParam, etUnknownCommand, etUnknownParam, etExtraParam, etInvalidValue};

enum cmndSource_t {FROM_BTTN, FROM_UART, FROM_MQTT, FROM_WEBC};

void doCommand(cmndSource_t source, String cmnd);
void doCommand(cmndSource_t source, const char *cmnd);
