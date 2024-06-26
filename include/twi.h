#ifdef ESP_OU

#ifndef TWI_H
#define TWI_H

#include <Arduino.h>

void setupTwi();
byte twiRead();
byte twiRead(const byte address, const byte command);
int twiWrite(const byte address, const byte command, const byte data);
void twiRequest(const byte address, const byte command, const byte size);

#endif
#endif