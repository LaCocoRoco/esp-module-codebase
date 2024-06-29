#ifndef SLEEP_H
#define SLEEP_H

#include <Arduino.h>

static const int SLEEP_DELAY_TIME = 100;
static const int SLEEP_US_TO_S_FACTOR = 1000000;
static const int SLEEP_WAKE_UP_TIME = 5 * SLEEP_US_TO_S_FACTOR;

enum SleepState {
  SLEEP_IDLE,
  SLEEP_ENABLED,
  SLEEP_WAIT,
  SLEEP_EVALUATE,
  SLEEP_START
};

extern bool resetSleep;

void setupSleep();
void taskSleep();
void sleepController();
void sleepConfiguration();
int systemActive();
#ifdef ESP_SU
void RTC_IRAM_ATTR espWakeStub(void);
#endif

#endif