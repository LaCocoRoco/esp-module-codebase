#ifdef ESP_OU

#include "twi.h"

#include <Wire.h>
#include <gpio.h>

void setupTwi() {
  Wire.begin(GPIO_SDA, GPIO_SCL);
}

byte twiRead() {
  return Wire.read();
}

byte twiRead(const byte address, const byte command) {
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  return Wire.read();
}

void twiRequest(const byte address, const byte command, const byte size) {
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.endTransmission();
  Wire.requestFrom(address, size);
}

int twiWrite(const byte address, const byte command, const byte data) {
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.write(data);
  return Wire.endTransmission();
}

#endif