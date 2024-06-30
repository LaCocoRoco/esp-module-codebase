#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>

static const int MODE_BUTTON_HOLD_TIME = 2000;
static const int CHANGE_MODE_BUZZER_TIME = 100;
static const int CLEAR_MODULE_BUZZER_TIME = 1000;

enum ChangeModeState {
  CHANGE_MODE_IDLE,
  CHANGE_MODE_BUZZER_ENABLE,
  CHANGE_MODE_BUZZER_DISABLE,
  CHANGE_MODE_BUZZER_TIME_ON,
  CHANGE_MODE_BUZZER_TIME_OFF,
  CHANGE_MODE
};

enum ModuleResetState {
  MODULE_RESET_IDLE,
  MODULE_RESET_BUZZER_ENABLE,
  MODULE_RESET_BUZZER_DISABLE,
  MODULE_RESET_BUZZER_TIME_ON,
  MODULE_RESET
};

enum ModeButtonState {
  MODE_BUTTON_IDLE,
  MODE_BUTTON_HOLD,
  MODE_BUTTON_CHANGE_MODE,
  MODE_BUTTON_CLEAR_MODULE,
  MODE_BUTTON_RELEASED
};

enum ModeState {
  MODE_NONE,
  MODE_WEB,
  MODE_HYPRID,
  MODE_LOW_POWER,
};

extern ModeState mode;
extern ChangeModeState changeModeState;
extern ModuleResetState moduleResetState;
extern ModeButtonState modeButtonState;
extern bool moduleReset;
extern bool modeButtonEvent;
extern bool modeButtonWakeupEvent;

void setupSystem();
void taskSystem();
void systemController();
void modeButtonController();
void changeModeController();
void moduleResetController();

#endif