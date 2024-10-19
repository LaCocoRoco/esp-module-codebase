#include "gpio.h"

#include <RunningMedian.h>
#include <driver/rtc_io.h>
#include <logger.h>
#include <system.h>
#ifdef ESP_OU
#include <touch.h>
#endif
#ifdef ESP_SU
#include <anemometer.h>
#include <hydreon.h>
#endif

RunningMedian batteryVoltageMedian(50);

#ifndef ESP_CU
float batteryVoltage;
#endif

void setupGpio() {
  ledcSetup(BUZZER_PWM_CHANNEL, BUZZER_PWM_FREQUENCY, BUZZER_PWM_RESOLUTION);
  ledcAttachPin(GPIO_BUZ, BUZZER_PWM_CHANNEL);
  rtc_gpio_hold_dis((gpio_num_t)GPIO_BUZ);
  pinMode(GPIO_MOD, INPUT_PULLUP);
#ifdef ESP_CU
  pinMode(GPIO_MR, OUTPUT);
  pinMode(GPIO_DS, OUTPUT);
  pinMode(GPIO_SHCP, OUTPUT);
  pinMode(GPIO_STCP, OUTPUT);
  digitalWrite(GPIO_MR, 0);
  digitalWrite(GPIO_STCP, LOW);
  shiftOut(GPIO_DS, GPIO_SHCP, MSBFIRST, 0);
  shiftOut(GPIO_DS, GPIO_SHCP, MSBFIRST, 0);
  digitalWrite(GPIO_STCP, HIGH);
  digitalWrite(GPIO_MR, 1);
#endif
#ifdef ESP_OU
  pinMode(GPIO_CHG, INPUT_PULLUP);
#endif
#ifdef ESP_SU
  pinMode(GPIO_OUT, INPUT_PULLUP);
#endif
}

void taskGpio() {
  gpioController();
#ifndef ESP_CU
  batteryController();
#endif
}

void gpioController() {
  modeButtonEvent = !digitalRead(GPIO_MOD);
#ifdef ESP_OU
  touchChangeEvent = !digitalRead(GPIO_CHG);
#endif
#ifdef ESP_SU
  hydreonStatus = !digitalRead(GPIO_OUT);
  anemometerAnalog = analogRead(GPIO_SNS);
#endif
}

#ifndef ESP_CU
void batteryController() {
  int batteryVoltageMilliVolts = analogReadMilliVolts(GPIO_BAT);
  batteryVoltageMedian.add(batteryVoltageMilliVolts * 2.0 / 1000.0);
  batteryVoltage = batteryVoltageMedian.getAverage();
}
#endif
