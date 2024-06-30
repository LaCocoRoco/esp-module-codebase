#include <espnow.h>
#include <gpio.h>
#include <network.h>
#include <serial.h>
#include <settings.h>
#include <sleep.h>
#include <socket.h>
#include <system.h>
#ifdef ESP_CU
#include <io.h>
#endif
#ifdef ESP_OU
#include <touch.h>
#include <twi.h>
#endif
#ifdef ESP_SU
#include <anemometer.h>
#include <hydreon.h>
#endif

void setup() {
  setupSettings();
  setupSerial();
  setupSleep();
  setupNetwork();
  setupEspNow();
  setupSystem();
  setupSocket();
  setupGpio();
#ifdef ESP_OU
  setupTwi();
  setupTouch();
#endif
}

void loop() {
  taskSerial();
  taskSleep();
  taskNetwork();
  taskEspNow();
  taskSystem();
  taskSocket();
  taskGpio();
#ifdef ESP_CU
  taskIo();
#endif
#ifdef ESP_OU
  taskTouch();
#endif
#ifdef ESP_SU
  taskHydreon();
  taskAnemometer();
#endif
}