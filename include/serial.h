#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

static const int SERIAL_BAUDRATE = 115200;

void setupSerial();
void taskSerial();
void serialTransmitTerminal(const String text);
void serial1Controller();
#ifdef ESP_SU
void serial2Controller();
#endif

#endif