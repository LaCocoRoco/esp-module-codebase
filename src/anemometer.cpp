#ifdef ESP_SU

#include "anemometer.h"

#include <RunningMedian.h>
#include <esp_wifi.h>
#include <espnow.h>
#include <settings.h>

RunningMedian anemometerAnalogValueMedian(50);
AnemometerState anemometerState;
bool anemometerCalibrate;
bool anemometerWakeupEvent;
int anemometerAnalog;
unsigned long anemometerTime;
RTC_DATA_ATTR int anemometerCalibratedVelocity;
RTC_DATA_ATTR int anemometerCalibratedAnalog;
RTC_DATA_ATTR int anemometerVelocity;

void taskAnemometer() {
  anemometerController();
}

void anemometerController() {
  anemometerAnalogValueMedian.add(anemometerAnalog);
  int anemometerAnalogValue = anemometerAnalogValueMedian.getAverage();
  int velocity = map(anemometerAnalogValue, 0, anemometerCalibratedAnalog, 0, anemometerCalibratedVelocity);

  switch (anemometerState) {
    case ANEMOMETER_IDLE: {
      if (anemometerThreshold(velocity)) {
        anemometerState = ANEMOMETER_UPDATE_VELOCITY;
      }

      if (anemometerWakeupEvent) {
        anemometerState = ANEMOMETER_UPDATE_VELOCITY;
      }

      if (anemometerCalibrate) {
        anemometerCalibrate = false;
        anemometerTime = millis();
        anemometerVelocity = anemometerCalibratedVelocity;
        logger(INFO, "Anemometer Start Calibration");
        anemometerState = ANEMOMETER_CALIBRATE;
      }

      break;
    }

    case ANEMOMETER_UPDATE_VELOCITY: {
      if (anemometerAnalogValueMedian.isFull()) {
        anemometerVelocity = velocity > ANEMOMETER_ACTIVATION_THRESHOLD ? velocity : 0;
        anemometerState = ANEMOMETER_ESP_NOW_SEND;
      }

      break;
    }

    case ANEMOMETER_ESP_NOW_SEND: {
      epsNowSendAnemometerData(anemometerVelocity);
      anemometerState = ANEMOMETER_IDLE;
      break;
    }

    case ANEMOMETER_CALIBRATE: {
      if (millis() > anemometerTime + ANEMOMETER_CALIBRATE_CALM_DOWN_TIME) {
        anemometerCalibratedAnalog = anemometerAnalogValue;
        preferences.putInt(PREFERENCES_KEY_ANEMOMETER_CALIBRATED_VELOCITY, anemometerCalibratedVelocity);
        preferences.putInt(PREFERENCES_KEY_ANEMOMETER_CALIBRATED_ANALOG, anemometerCalibratedAnalog);
        logger(INFO, "Anemometer Calibrated Velocity:     " + String(anemometerCalibratedVelocity));
        logger(INFO, "Anemometer Calibrated Analog Value: " + String(anemometerCalibratedAnalog));
        anemometerState = ANEMOMETER_UPDATE_VELOCITY;
      }

      break;
    }
  }
}

void calibrateAnemometer(int velocity) {
  anemometerCalibratedVelocity = velocity;
  anemometerCalibratedAnalog = anemometerAnalogValueMedian.getAverage();
}

bool anemometerThreshold(int velocity) {
  // velocity maximum activation threshold
  if (velocity > ANEMOMETER_ACTIVATION_THRESHOLD) {
    // velocity minimum update threshold
    if (velocity < anemometerVelocity - ANEMOMETER_UPDATE_THRESHOLD) return true;
    // velocity maximum update threshold
    if (velocity > anemometerVelocity + ANEMOMETER_UPDATE_THRESHOLD) return true;
  }

  // velocity minimum activation threshold
  if (velocity < ANEMOMETER_ACTIVATION_THRESHOLD) {
    if (anemometerVelocity > ANEMOMETER_ACTIVATION_THRESHOLD) return true;
  }

  return false;
}

bool RTC_IRAM_ATTR rtcAnemometerThreshold(int velocity) {
  // velocity maximum activation threshold
  if (velocity > ANEMOMETER_ACTIVATION_THRESHOLD) {
    // velocity minimum update threshold
    if (velocity < anemometerVelocity - ANEMOMETER_UPDATE_THRESHOLD) return true;
    // velocity maximum update threshold
    if (velocity > anemometerVelocity + ANEMOMETER_UPDATE_THRESHOLD) return true;
  }

  // velocity minimum activation threshold
  if (velocity < ANEMOMETER_ACTIVATION_THRESHOLD) {
    if (anemometerVelocity > ANEMOMETER_ACTIVATION_THRESHOLD) return true;
  }

  return false;
}

#endif