#ifdef ESP_OU

#include "touch.h"

#include <Wifi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <espnow.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <settings.h>
#include <twi.h>

RTC_DATA_ATTR KeyStatus keyStatus;
RTC_DATA_ATTR AT42QT2120 at42qt2120;
TouchState touchState;
bool touchChangeWakeupEvent;
bool touchChangeEvent;
bool touchDefaultsLoaded;

void setupTouch() {
  if (!touchDefaultsLoaded) {
    preferences.putBool(PREFERENCES_KEY_TOUCH_DEFAULTS_LOADED, true);
    touchWriteRegisterDefaults();
  }
}

void taskTouch() {
  touchController();
}

void touchController() {
  switch (touchState) {
    case TOUCH_IDLE: {
      if (touchChangeEvent || touchChangeWakeupEvent) {
        touchChangeWakeupEvent = false;
        touchState = TOUCH_CHANGE;
      }

      break;
    };

    case TOUCH_CHANGE: {
      logger(TRACE, "touch change");
      KeyStatus keyStatus;
      twiRead(AT42QT2120_ADDRESS, DETECTION_STATUS);
      keyStatus.first = twiRead(AT42QT2120_ADDRESS, KEY_STATUS_FIRST);
      keyStatus.second = twiRead(AT42QT2120_ADDRESS, KEY_STATUS_SECOND);

      const byte *keyStatusPointer01 = (byte *)&keyStatus.value;
      const byte *keyStatusPointer02 = (byte *)&::keyStatus.value;

      for (int i = 0; i < 2; i++) {
        const byte keyStatusByte01 = *(keyStatusPointer01 + i);
        const byte keyStatusByte02 = *(keyStatusPointer02 + i);

        for (int j = 0; j < 8; j++) {
          const byte keyStatusBit01 = (keyStatusByte01 >> j) & 1;
          const byte keyStatusBit02 = (keyStatusByte02 >> j) & 1;

          if (keyStatusBit01 != keyStatusBit02) {
            const int key = j + (i * 8);
            const bool state = keyStatusBit01;
            espNowSendTouchData(key, state);
          }
        }
      }

      at42qt2120.keyStatus.first = ::keyStatus.first = keyStatus.first;
      at42qt2120.keyStatus.second = ::keyStatus.second = keyStatus.second;
      touchState = TOUCH_IDLE;
      break;
    }
  }
}

void touchLogRegisterData(AT42QT2120 at42at2120) {
  logger(TRACE, "chip id:\t\t" + String(at42at2120.chipID.value));
  logger(TRACE, "firmware major:\t" + String(at42at2120.firmwareVersion.majorVersion));
  logger(TRACE, "firmware minor:\t" + String(at42at2120.firmwareVersion.minorVersion));
  logger(TRACE, "detect calibrate:\t" + String(at42at2120.detectionStatus.calibrate));
  logger(TRACE, "detect overflow:\t" + String(at42at2120.detectionStatus.overflow));
  logger(TRACE, "detect sdet:\t\t" + String(at42at2120.detectionStatus.sdet));
  logger(TRACE, "detect tdet:\t\t" + String(at42at2120.detectionStatus.tdet));
  logger(TRACE, "slider status:\t" + String(at42at2120.sliderStatus.value));
  logger(TRACE, "calibrate:\t\t" + String(at42at2120.calibrate.value));
  logger(TRACE, "reset:\t\t" + String(at42at2120.reset.value));
  logger(TRACE, "low power mode:\t" + String(at42at2120.lowPowerMode.value));
  logger(TRACE, "toward touch drift:\t" + String(at42at2120.towardsTouchDrift.value));
  logger(TRACE, "away touch drift:\t" + String(at42at2120.awayFromTouchDrift.value));
  logger(TRACE, "detection integration:\t" + String(at42at2120.detectionIntegration.value));
  logger(TRACE, "touch recal delay:\t" + String(at42at2120.touchRecalDelay.value));
  logger(TRACE, "drift hold time:\t" + String(at42at2120.driftHoldTime.value));
  logger(TRACE, "slider opt enable:\t" + String(at42at2120.sliderOptions.en));
  logger(TRACE, "slider opt wheel:\t" + String(at42at2120.sliderOptions.wheel));
  logger(TRACE, "charge time:\t\t" + String(at42at2120.chargeTime.time));

  for (int i = 0; i < AT42QT2120_KEYS; i++) {
    const String number = i < 10 ? "0" + String(i) : String(i);
    const byte value01 = at42at2120.keyStatus.first;
    const byte value02 = at42at2120.keyStatus.second;
    const byte status = (i < 8) ? (value01 >> i) & 1 : (value02 >> i - 8) & 1;
    const byte threshold = at42at2120.detectThreshold[i].value;
    const byte control = at42at2120.keyControl[i].value;
    const byte en = at42at2120.keyControl[i].en;
    const byte aks = at42at2120.keyControl[i].aks;
    const byte gpo = at42at2120.keyControl[i].gpo;
    const byte guard = at42at2120.keyControl[i].guard;
    const byte pulse = at42at2120.keyPulseScale[i].pulse;
    const byte scale = at42at2120.keyPulseScale[i].scale;
    const int signal = at42at2120.keySignal[i].value;
    const int reference = at42at2120.referenceData[i].value;

    logger(TRACE, "key " + number + " status:\t" + status);
    logger(TRACE, "key " + number + " threshold:\t" + threshold);
    logger(TRACE, "key " + number + " control:\t" + threshold);
    logger(TRACE, "key " + number + " control en:\t" + en);
    logger(TRACE, "key " + number + " control aks:\t" + aks);
    logger(TRACE, "key " + number + " control gpo:\t" + gpo);
    logger(TRACE, "key " + number + " control guard:" + guard);
    logger(TRACE, "key " + number + " pulse:\t" + pulse);
    logger(TRACE, "key " + number + " scale:\t" + scale);
    logger(TRACE, "key " + number + " signal:\t" + signal);
    logger(TRACE, "key " + number + " reference:\t" + reference);
  }
}

void touchWriteCommonRegister(String name, const byte value) {
  logger(DEBUG, "Touch Set Register: " + name);
  logger(DEBUG, "Touch Set Value:    " + String(value));

  if (name == "reset") {
    twiWrite(AT42QT2120_ADDRESS, RESET, value);
  }

  if (name == "lowPowerMode") {
    twiWrite(AT42QT2120_ADDRESS, LOW_POWER, value);
  }

  if (name == "towardTouchDrift") {
    twiWrite(AT42QT2120_ADDRESS, TOWARDS_TOUCH_DRIFT, value);
  }

  if (name == "awayTouchDrift") {
    twiWrite(AT42QT2120_ADDRESS, AWAY_FROM_TOUCH_DRIFT, value);
  }

  if (name == "detectionIntegration") {
    twiWrite(AT42QT2120_ADDRESS, DETECTION_INTEGRATOR, value);
  }

  if (name == "touchRecalDelay") {
    twiWrite(AT42QT2120_ADDRESS, TOUCH_RECAL_DELAY, value);
  }

  if (name == "driftHoldTime") {
    twiWrite(AT42QT2120_ADDRESS, DRIFT_HOLD_TIME, value);
  }

  if (name == "sliderOptEnable") {
    SliderOptions sliderOptions;
    sliderOptions.en = value;
    twiWrite(AT42QT2120_ADDRESS, DRIFT_HOLD_TIME, sliderOptions.value);
  }

  if (name == "sliderOptWheel") {
    SliderOptions sliderOptions;
    sliderOptions.wheel = value;
    twiWrite(AT42QT2120_ADDRESS, DRIFT_HOLD_TIME, sliderOptions.value);
  }

  // twiWrite(AT42QT2120_ADDRESS, CALIBRATE, true);
}

void touchWriteKeyRegister(String name, const byte value, const byte key) {
  logger(DEBUG, "Touch Set Register: " + name);
  logger(DEBUG, "Touch Set Key:      " + String(key));
  logger(DEBUG, "Touch Set Value:    " + String(value));

  if (name == "keyThreshold") {
    twiWrite(AT42QT2120_ADDRESS, KEY_00_DETECT_THRESHOLD + key, value);
  }

  if (name == "keyControlEn") {
    KeyControl keyControl;
    keyControl.en = value;
    twiWrite(AT42QT2120_ADDRESS, KEY_00_CONTROL + key, keyControl.value);
  }

  if (name == "keyControlAks") {
    KeyControl keyControl;
    keyControl.aks = value;
    twiWrite(AT42QT2120_ADDRESS, KEY_00_CONTROL + key, keyControl.value);
  }

  if (name == "keyControlGpo") {
    KeyControl keyControl;
    keyControl.gpo = value;
    twiWrite(AT42QT2120_ADDRESS, KEY_00_CONTROL + key, keyControl.value);
  }

  if (name == "keyControlGuard") {
    KeyControl keyControl;
    keyControl.guard = value;
    twiWrite(AT42QT2120_ADDRESS, KEY_00_CONTROL + key, keyControl.value);
  }

  if (name == "keyPulse") {
    KeyPulseScale keyControl;
    keyControl.pulse = value;
    twiWrite(AT42QT2120_ADDRESS, KEY_00_PULSE_SCALE + key, keyControl.value);
  }

  if (name == "keyScale") {
    KeyPulseScale keyControl;
    keyControl.scale = value;
    twiWrite(AT42QT2120_ADDRESS, KEY_00_PULSE_SCALE + key, keyControl.value);
  }

  twiWrite(AT42QT2120_ADDRESS, CALIBRATE, true);
}

void touchWriteRegisterDefaults() {
  AT42QT2120 at42qt2120;

  at42qt2120.lowPowerMode.value = DEFAULT_TOUCH_LOW_POWER_MODE;
  at42qt2120.towardsTouchDrift.value = DEFAULT_TOUCH_TOWARDS_TOUCH_DRIFT;
  at42qt2120.awayFromTouchDrift.value = DEFAULT_TOUCH_AWAY_FROM_TOUCH_DRIFT;
  at42qt2120.detectionIntegration.value = DEFAULT_TOUCH_DETECTION_INTERGRATION;
  at42qt2120.touchRecalDelay.value = DEFAULT_TOUCH_RECAL_DELAY;
  at42qt2120.driftHoldTime.value = DEFAULT_TOUCH_DRIFT_HOLD_TIME;
  at42qt2120.sliderOptions.en = DEFAULT_TOUCH_SLIDER_OPTION_WHEEL;
  at42qt2120.sliderOptions.wheel = DEFAULT_TOUCH_SLIDER_OPTION_ENABLE;

  twiWrite(AT42QT2120_ADDRESS, AWAY_FROM_TOUCH_DRIFT, at42qt2120.awayFromTouchDrift.value);
  twiWrite(AT42QT2120_ADDRESS, LOW_POWER, at42qt2120.lowPowerMode.value);
  twiWrite(AT42QT2120_ADDRESS, TOWARDS_TOUCH_DRIFT, at42qt2120.towardsTouchDrift.value);
  twiWrite(AT42QT2120_ADDRESS, DETECTION_INTEGRATOR, at42qt2120.detectionIntegration.value);
  twiWrite(AT42QT2120_ADDRESS, TOUCH_RECAL_DELAY, at42qt2120.touchRecalDelay.value);
  twiWrite(AT42QT2120_ADDRESS, DRIFT_HOLD_TIME, at42qt2120.driftHoldTime.value);
  twiWrite(AT42QT2120_ADDRESS, DRIFT_HOLD_TIME, at42qt2120.sliderOptions.value);
  twiWrite(AT42QT2120_ADDRESS, DRIFT_HOLD_TIME, at42qt2120.sliderOptions.value);

  for (int i = 0; i < AT42QT2120_KEYS; i++) {
    at42qt2120.detectThreshold[i].value = DEFAULT_TOUCH_KEY_THRESHOLD;
    at42qt2120.keyControl[i].en = DEFAULT_TOUCH_KEY_CONTROL_EN;
    at42qt2120.keyControl[i].aks = DEFAULT_TOUCH_KEY_CONTROL_AKS;
    at42qt2120.keyControl[i].gpo = DEFAULT_TOUCH_KEY_CONTROL_GPO;
    at42qt2120.keyControl[i].guard = DEFAULT_TOUCH_KEY_CONTROL_GUARD;
    at42qt2120.keyPulseScale[i].pulse = DEFAULT_TOUCH_KEY_PULSE;
    at42qt2120.keyPulseScale[i].scale = DEFAULT_TOUCH_KEY_SCALE;

    twiWrite(AT42QT2120_ADDRESS, KEY_00_DETECT_THRESHOLD + i, at42qt2120.detectThreshold[i].value);
    twiWrite(AT42QT2120_ADDRESS, KEY_00_CONTROL + i, at42qt2120.keyControl[i].value);
    twiWrite(AT42QT2120_ADDRESS, KEY_00_PULSE_SCALE + i, at42qt2120.keyPulseScale[i].value);
  }
}

void touchReadRegisterData() {
  twiRequest(AT42QT2120_ADDRESS, 0, 100);
  at42qt2120.chipID.value = twiRead();
  at42qt2120.firmwareVersion.value = twiRead();
  at42qt2120.detectionStatus.value = twiRead();
  at42qt2120.keyStatus.first = twiRead();
  at42qt2120.keyStatus.second = twiRead();
  at42qt2120.sliderStatus.value = twiRead();
  at42qt2120.calibrate.value = twiRead();
  at42qt2120.reset.value = twiRead();
  at42qt2120.lowPowerMode.value = twiRead();
  at42qt2120.towardsTouchDrift.value = twiRead();
  at42qt2120.awayFromTouchDrift.value = twiRead();
  at42qt2120.detectionIntegration.value = twiRead();
  at42qt2120.touchRecalDelay.value = twiRead();
  at42qt2120.driftHoldTime.value = twiRead();
  at42qt2120.sliderOptions.value = twiRead();
  at42qt2120.chargeTime.value = twiRead();

  for (int i = 0; i < AT42QT2120_KEYS; i++) at42qt2120.detectThreshold[i].value = twiRead();
  for (int i = 0; i < AT42QT2120_KEYS; i++) at42qt2120.keyControl[i].value = twiRead();
  for (int i = 0; i < AT42QT2120_KEYS; i++) at42qt2120.keyPulseScale[i].value = twiRead();

  for (int i = 0; i < AT42QT2120_KEYS; i++) {
    at42qt2120.keySignal[i].msb = twiRead();
    at42qt2120.keySignal[i].lsb = twiRead();
  }

  for (int i = 0; i < AT42QT2120_KEYS; i++) {
    at42qt2120.referenceData[i].msb = twiRead();
    at42qt2120.referenceData[i].lsb = twiRead();
  }
}

#endif