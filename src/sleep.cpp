#include "sleep.h"

#include <Wifi.h>
#include <driver/rtc_io.h>
#include <esp_now.h>
#include <espnow.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <system.h>
#ifdef ESP_OU
#include <touch.h>
#endif
#ifdef ESP_SU
#include <EspWakeStub.h>
#include <anemometer.h>
#include <hydreon.h>
#endif

SleepState sleepState;
int sleepTime;
bool resetSleep;

void setupSleep() {
  switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_EXT0: {
      logger(INFO, "Esp WakeUp Mode Button Event");
      modeButtonWakeupEvent = true;
      break;
    }
#ifdef ESP_OU
    case ESP_SLEEP_WAKEUP_EXT1: {
      logger(INFO, "Esp WakeUp Touch Event");
      touchChangeWakeupEvent = true;
      break;
    }
#endif
#ifdef ESP_SU
    case ESP_SLEEP_WAKEUP_EXT1: {
      logger(INFO, "Esp WakeUp Hydreon Event");
      hydreonWakeupEvent = true;
      break;
    }

    case ESP_SLEEP_WAKEUP_TIMER: {
      logger(INFO, "Esp WakeUp Timer Event");
      anemometerWakeupEvent = true;
      break;
    }
#endif
  }
}

void taskSleep() {
  sleepController();
}

void sleepController() {
  switch (sleepState) {
    case SLEEP_IDLE: {
      if (mode == MODE_LOW_POWER) {
        sleepState = SLEEP_ENABLED;
      }
#ifndef ESP_CU
      if (mode == MODE_HYPRID && batteryVoltage < 3.8) {
        sleepState = SLEEP_ENABLED;
      }
#endif
      break;
    }

    case SLEEP_ENABLED: {
      resetSleep = false;
      sleepTime = millis();
      sleepState = SLEEP_WAIT;
      break;
    }

    case SLEEP_WAIT: {
      if (millis() > sleepTime + SLEEP_DELAY_TIME) {
        sleepState = SLEEP_EVALUATE;
      }

      break;
    }

    case SLEEP_EVALUATE: {
      if (systemActive() || resetSleep) {
        sleepState = SLEEP_IDLE;
      } else {
        sleepState = SLEEP_START;
      }

      break;
    }

    case SLEEP_START: {
      logger(TRACE, "System Sleep");
      sleepConfiguration();
      sleepState = SLEEP_IDLE;
      break;
    }
  }
}

int systemActive() {
  int state;
  state += changeModeState;
  state += moduleResetState;
  state += modeButtonState;
  state += peerInfoState;
#ifdef ESP_OU
  state += touchState;
#endif
#ifdef ESP_SU
  state += anemometerState;
  state += hydreonState;
#endif
  return state;
}

void sleepConfiguration() {
#ifdef ESP_OU
  esp_sleep_enable_ext0_wakeup(gpio_num_t(GPIO_MOD), LOW);
  esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << GPIO_CHG)), ESP_EXT1_WAKEUP_ALL_LOW);
#endif
#ifdef ESP_SU
  esp_sleep_enable_timer_wakeup(SLEEP_WAKE_UP_TIME);
  esp_sleep_enable_ext0_wakeup(gpio_num_t(GPIO_MOD), LOW);
  esp_sleep_ext1_wakeup_mode_t allLow = ESP_EXT1_WAKEUP_ALL_LOW;
  esp_sleep_ext1_wakeup_mode_t anyHigh = ESP_EXT1_WAKEUP_ANY_HIGH;
  esp_sleep_enable_ext1_wakeup((uint64_t)1 << GPIO_OUT, !hydreonStatus ? allLow : anyHigh);
  esp_set_deep_sleep_wake_stub(&espWakeStub);
#endif

  Serial.flush();
  rtc_gpio_isolate((gpio_num_t)GPIO_BUZ);
  esp_now_deinit();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_deep_sleep_start();
}

#ifdef ESP_SU
void RTC_IRAM_ATTR espWakeStub(void) {
  esp_default_wake_deep_sleep();

  switch (rtcWakeupCause()) {
    case ESP_SLEEP_WAKEUP_EXT0: return;
    case ESP_SLEEP_WAKEUP_EXT1: return;
    case ESP_SLEEP_WAKEUP_TIMER: {
      int analogValue = rtcAdc1GetRaw(GPIO_SNS_CH);
      int analogVelocity = rtcMap(analogValue, 0, anemometerCalibratedAnalog, 0, anemometerCalibratedVelocity);
      if (rtcAnemometerThreshold(analogVelocity)) return;
      break;
    }
  }

  rtcSetWakeupTime(SLEEP_WAKE_UP_TIME);
  rtcSleep(&espWakeStub);
}
#endif