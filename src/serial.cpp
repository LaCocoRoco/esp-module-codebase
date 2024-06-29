#include "serial.h"

#include <cmd.h>
#include <gpio.h>
#include <logger.h>
#ifdef ESP_SU
#include <hydreon.h>
#endif

void setupSerial() {
  Serial.begin(SERIAL_BAUDRATE, SERIAL_8N1, GPIO_RX0, GPIO_TX0);
#ifdef ESP_SU
  Serial2.begin(hydreonBaudrate, SERIAL_8N1, GPIO_RX1, GPIO_TX1);
  Serial2.println();
#endif
}

void taskSerial() {
  serial1Controller();
#ifdef ESP_SU
  serial2Controller();
#endif
}

void serialTransmitTerminal(const String text) {
  Serial.println(text);
}

void serial1Controller() {
  if (Serial.available() > 0) {
    String value = Serial.readString();
    value.trim();
    logger(TRACE, value);
    cmdHandler(value);
  }
}

#ifdef ESP_SU
void serial2Controller() {
  if (Serial2.available()) {
    String value = Serial2.readStringUntil('\n');
    value.trim();
    hydreonResponseHandler(value);
  }
}
#endif