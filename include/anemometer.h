#ifdef ESP_SU

#ifndef ANEMOMETER_H
#define ANEMOMETER_H

#include <Arduino.h>

static const int ANEMOMETER_UPDATE_THRESHOLD = 2;
static const int ANEMOMETER_ACTIVATION_THRESHOLD = 5;
static const int ANEMOMETER_CALIBRATE_CALM_DOWN_TIME = 5000;

enum AnemometerState {
  ANEMOMETER_IDLE,
  ANEMOMETER_UPDATE_VELOCITY,
  ANEMOMETER_ESP_NOW_SEND,
  ANEMOMETER_CALIBRATE
};

extern AnemometerState anemometerState;
extern bool anemometerCalibrate;
extern bool anemometerWakeupEvent;
extern int anemometerVelocity;
extern int anemometerCalibratedVelocity;
extern int anemometerCalibratedAnalog;
extern int anemometerAnalog;

void taskAnemometer();
void anemometerController();
void calibrateAnemometer(int velocity);
bool anemometerThreshold(int value);
bool rtcAnemometerThreshold(int value);

#endif
#endif