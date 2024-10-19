#ifdef ESP_OU

#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>

static const byte DEFAULT_TOUCH_KEY_THRESHOLD = 10;
static const byte DEFAULT_TOUCH_KEY_CONTROL_EN = 0;
static const byte DEFAULT_TOUCH_KEY_CONTROL_AKS = 0;
static const byte DEFAULT_TOUCH_KEY_CONTROL_GPO = 0;
static const byte DEFAULT_TOUCH_KEY_CONTROL_GUARD = 0;
static const byte DEFAULT_TOUCH_KEY_PULSE = 0;
static const byte DEFAULT_TOUCH_KEY_SCALE = 0;

static const byte DEFAULT_TOUCH_LOW_POWER_MODE = 1;
static const byte DEFAULT_TOUCH_TOWARDS_TOUCH_DRIFT = 20;
static const byte DEFAULT_TOUCH_AWAY_FROM_TOUCH_DRIFT = 5;
static const byte DEFAULT_TOUCH_DETECTION_INTERGRATION = 4;
static const byte DEFAULT_TOUCH_RECAL_DELAY = 255;
static const byte DEFAULT_TOUCH_DRIFT_HOLD_TIME = 25;
static const byte DEFAULT_TOUCH_SLIDER_OPTION_WHEEL = 0;
static const byte DEFAULT_TOUCH_SLIDER_OPTION_ENABLE = 0;

static const int AT42QT2120_ADDRESS = 0x1C;
static const int AT42QT2120_KEYS = 12;
static const int AT42QT2120_THRESHOLD = 10;

enum TouchState {
  TOUCH_IDLE,
  TOUCH_CHANGE
};

enum AT42QT2120Register {
  CHIP_ID,
  FIRMWARE_VERSION,
  DETECTION_STATUS,
  KEY_STATUS_FIRST,
  KEY_STATUS_SECOND,
  SLIDER_POSITION,
  CALIBRATE,
  RESET,
  LOW_POWER,
  TOWARDS_TOUCH_DRIFT,
  AWAY_FROM_TOUCH_DRIFT,
  DETECTION_INTEGRATOR,
  TOUCH_RECAL_DELAY,
  DRIFT_HOLD_TIME,
  SLIDER_OPTIONS,
  CHARGE_TIME,
  KEY_00_DETECT_THRESHOLD,
  KEY_01_DETECT_THRESHOLD,
  KEY_02_DETECT_THRESHOLD,
  KEY_03_DETECT_THRESHOLD,
  KEY_04_DETECT_THRESHOLD,
  KEY_05_DETECT_THRESHOLD,
  KEY_06_DETECT_THRESHOLD,
  KEY_07_DETECT_THRESHOLD,
  KEY_08_DETECT_THRESHOLD,
  KEY_09_DETECT_THRESHOLD,
  KEY_10_DETECT_THRESHOLD,
  KEY_11_DETECT_THRESHOLD,
  KEY_00_CONTROL,
  KEY_01_CONTROL,
  KEY_02_CONTROL,
  KEY_03_CONTROL,
  KEY_04_CONTROL,
  KEY_05_CONTROL,
  KEY_06_CONTROL,
  KEY_07_CONTROL,
  KEY_08_CONTROL,
  KEY_09_CONTROL,
  KEY_10_CONTROL,
  KEY_11_CONTROL,
  KEY_00_PULSE_SCALE,
  KEY_01_PULSE_SCALE,
  KEY_02_PULSE_SCALE,
  KEY_03_PULSE_SCALE,
  KEY_04_PULSE_SCALE,
  KEY_05_PULSE_SCALE,
  KEY_06_PULSE_SCALE,
  KEY_07_PULSE_SCALE,
  KEY_08_PULSE_SCALE,
  KEY_09_PULSE_SCALE,
  KEY_10_PULSE_SCALE,
  KEY_11_PULSE_SCALE,
  KEY_SIGNAL_00_MSB,
  KEY_SIGNAL_00_LSB,
  KEY_SIGNAL_01_MSB,
  KEY_SIGNAL_01_LSB,
  KEY_SIGNAL_02_MSB,
  KEY_SIGNAL_02_LSB,
  KEY_SIGNAL_03_MSB,
  KEY_SIGNAL_03_LSB,
  KEY_SIGNAL_04_MSB,
  KEY_SIGNAL_04_LSB,
  KEY_SIGNAL_05_MSB,
  KEY_SIGNAL_05_LSB,
  KEY_SIGNAL_06_MSB,
  KEY_SIGNAL_06_LSB,
  KEY_SIGNAL_07_MSB,
  KEY_SIGNAL_07_LSB,
  KEY_SIGNAL_08_MSB,
  KEY_SIGNAL_08_LSB,
  KEY_SIGNAL_09_MSB,
  KEY_SIGNAL_09_LSB,
  KEY_SIGNAL_10_MSB,
  KEY_SIGNAL_10_LSB,
  KEY_SIGNAL_11_MSB,
  KEY_SIGNAL_11_LSB,
  REFERENCE_DATA_00_MSB,
  REFERENCE_DATA_00_LSB,
  REFERENCE_DATA_01_MSB,
  REFERENCE_DATA_01_LSB,
  REFERENCE_DATA_02_MSB,
  REFERENCE_DATA_02_LSB,
  REFERENCE_DATA_03_MSB,
  REFERENCE_DATA_03_LSB,
  REFERENCE_DATA_04_MSB,
  REFERENCE_DATA_04_LSB,
  REFERENCE_DATA_05_MSB,
  REFERENCE_DATA_05_LSB,
  REFERENCE_DATA_06_MSB,
  REFERENCE_DATA_06_LSB,
  REFERENCE_DATA_07_MSB,
  REFERENCE_DATA_07_LSB,
  REFERENCE_DATA_08_MSB,
  REFERENCE_DATA_08_LSB,
  REFERENCE_DATA_09_MSB,
  REFERENCE_DATA_09_LSB,
  REFERENCE_DATA_10_MSB,
  REFERENCE_DATA_10_LSB,
  REFERENCE_DATA_11_MSB,
  REFERENCE_DATA_11_LSB,
};

struct ByteValue {
  byte value;
};

union Bitweight {
  struct {
    byte msb;
    byte lsb;
  };
  int value;
};

union FirmwareVersion {
  struct {
    byte majorVersion : 4;
    byte minorVersion : 4;
  };
  byte value;
};

union DetectionStatus {
  struct {
    byte tdet : 1;
    byte sdet : 1;
    byte : 4;
    byte overflow : 1;
    byte calibrate : 1;
  };
  byte value;
};

union KeyStatus {
  struct {
    byte first;
    byte second;
  };
  struct {
    byte key00 : 1;
    byte key01 : 1;
    byte key02 : 1;
    byte key03 : 1;
    byte key04 : 1;
    byte key05 : 1;
    byte key06 : 1;
    byte key07 : 1;
    byte key08 : 1;
    byte key09 : 1;
    byte key10 : 1;
    byte key11 : 1;
  };
  struct {
    int value;
  };
};

union SliderOptions {
  struct {
    byte : 6;
    byte wheel : 1;
    byte en : 1;
  };
  byte value;
};

union ChargeTime {
  struct {
    byte time : 4;
    byte : 4;
  };
  byte value;
};

union KeyControl {
  struct {
    byte en : 1;
    byte gpo : 1;
    byte aks : 2;
    byte guard : 1;
    byte : 3;
  };
  byte value;
};

union KeyPulseScale {
  struct {
    byte scale : 4;
    byte pulse : 4;
  };
  byte value;
};

struct AT42QT2120 {
  ByteValue chipID;
  FirmwareVersion firmwareVersion;
  DetectionStatus detectionStatus;
  KeyStatus keyStatus;
  ByteValue sliderStatus;
  ByteValue calibrate;
  ByteValue reset;
  ByteValue lowPowerMode;
  ByteValue towardsTouchDrift;
  ByteValue awayFromTouchDrift;
  ByteValue detectionIntegration;
  ByteValue touchRecalDelay;
  ByteValue driftHoldTime;
  SliderOptions sliderOptions;
  ChargeTime chargeTime;
  ByteValue detectThreshold[AT42QT2120_KEYS] = {0};
  KeyControl keyControl[AT42QT2120_KEYS] = {0};
  KeyPulseScale keyPulseScale[AT42QT2120_KEYS] = {0};
  Bitweight keySignal[AT42QT2120_KEYS] = {0};
  Bitweight referenceData[AT42QT2120_KEYS] = {0};
};

extern AT42QT2120 at42qt2120;
extern TouchState touchState;
extern bool touchChangeEvent;
extern bool touchChangeWakeupEvent;
extern bool touchDefaultsLoaded;

void setupTouch();
void taskTouch();
void touchController();
void touchLogRegisterData(AT42QT2120 at42qt2120);
void touchWriteCommonRegister(const String name, const byte value);
void touchWriteKeyRegister(const String name, const byte value, const byte key);
void touchWriteRegisterDefaults();
void touchReadRegisterData();

#endif
#endif