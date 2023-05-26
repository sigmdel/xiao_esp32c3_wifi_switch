#pragma once

enum cmndSource_t {FROM_UART, FROM_WEBC, FROM_MQTT};

void doCommand(cmndSource_t source, String cmnd);
void doCommand(cmndSource_t source, const char *cmnd);
