#include "system.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <config.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <settings.h>
#include <socket.h>
#include <web.h>

ChangeModeState changeModeState;
unsigned long changeModeBuzzerTime;
int changeModeBuzzerIteration;
bool changeMode;
ModuleResetState moduleResetState;
unsigned long moduleResetBuzzerTime;
bool moduleReset;
ModeButtonState modeButtonState;
unsigned long modeButtonTime;
bool modeButtonEvent;
bool modeButtonWakeupEvent;
RTC_DATA_ATTR ModeState mode;

void setupSystem() {
  switch (mode) {
    case MODE_LOW_POWER: {
      logger(TRACE, "Low Power Mode Initialized");
      break;
    }

    case MODE_HYPRID: {
      logger(TRACE, "Hyprid Mode Initialized");
      break;
    }

    case MODE_WEB: {
      ArduinoOTA.onStart([]() { logger(TRACE, "OTA Start"); });
      ArduinoOTA.onEnd([]() { ESP.restart(); });
      ArduinoOTA.begin();
      SPIFFS.begin();
      MDNS.begin(MODULE_HOSTNAME.c_str());
      MDNS.addService("http", "tcp", 80);
      webSocket.onEvent(socketEvent);
      webServer.onNotFound(requestHandlerNotFound);
      webServer.on("/index.html", HTTP_GET, requestHandlerIndexHtml);
      webServer.on("/index.css", HTTP_GET, requestHandlerIndexCss);
      webServer.on("/index.js", HTTP_GET, requestHandlerIndexJs);
      webServer.on("/favicon.ico", HTTP_GET, requestHandlerFavicon);
      webServer.addHandler(&webSocket);
      webServer.begin();
      logger(TRACE, "Web Mode Initialized");
      break;
    }
  }
}

void taskSystem() {
  systemController();
  modeButtonController();
  changeModeController();
  moduleResetController();
}

void systemController() {
  if (mode == MODE_WEB) {
    dnsServer.processNextRequest();
    ArduinoOTA.handle();
  }
}

void modeButtonController() {
  switch (modeButtonState) {
    case MODE_BUTTON_IDLE: {
      if (modeButtonEvent || modeButtonWakeupEvent) {
        modeButtonWakeupEvent = false;
        modeButtonTime = millis();
        modeButtonState = MODE_BUTTON_HOLD;
      };

      break;
    }

    case MODE_BUTTON_HOLD: {
      if (millis() > modeButtonTime + MODE_BUTTON_HOLD_TIME) {
        modeButtonState = MODE_BUTTON_CLEAR_MODULE;
      }

      if (!modeButtonEvent) {
        modeButtonState = MODE_BUTTON_CHANGE_MODE;
      };

      break;
    }

    case MODE_BUTTON_CLEAR_MODULE: {
      moduleReset = true;
      modeButtonState = MODE_BUTTON_RELEASED;
      break;
    }

    case MODE_BUTTON_CHANGE_MODE: {
      changeMode = true;
      modeButtonState = MODE_BUTTON_RELEASED;
      break;
    }

    case MODE_BUTTON_RELEASED: {
      if (!modeButtonEvent) {
        modeButtonState = MODE_BUTTON_IDLE;
      };

      break;
    }
  }
}

void changeModeController() {
  switch (changeModeState) {
    case CHANGE_MODE_IDLE: {
      if (changeMode) {
        switch (mode) {
          /* 1 Buzzer Signal: Low Power Mode */
          /* 2 Buzzer Signal: Hyprid Mode    */
          /* 3 Buzzer Signal: Web Mode       */
          case MODE_LOW_POWER: changeModeBuzzerIteration = MODE_HYPRID; break;
          case MODE_HYPRID: changeModeBuzzerIteration = MODE_WEB; break;
          case MODE_WEB: changeModeBuzzerIteration = MODE_LOW_POWER; break;
        }

        changeModeState = CHANGE_MODE_BUZZER_ENABLE;
      }

      break;
    }

    case CHANGE_MODE_BUZZER_ENABLE: {
      ledcWrite(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE_ON);
      changeModeBuzzerIteration--;
      changeModeBuzzerTime = millis();
      changeModeState = CHANGE_MODE_BUZZER_TIME_ON;
      break;
    };

    case CHANGE_MODE_BUZZER_TIME_ON: {
      if (millis() > changeModeBuzzerTime + CHANGE_MODE_BUZZER_TIME) {
        changeModeState = CHANGE_MODE_BUZZER_DISABLE;
      }

      break;
    };

    case CHANGE_MODE_BUZZER_DISABLE: {
      ledcWrite(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE_OFF);
      changeModeBuzzerTime = millis();
      changeModeState = CHANGE_MODE_BUZZER_TIME_OFF;
      break;
    };

    case CHANGE_MODE_BUZZER_TIME_OFF: {
      if (millis() > changeModeBuzzerTime + CHANGE_MODE_BUZZER_TIME) {
        changeModeState = CHANGE_MODE_BUZZER_ENABLE;
      }

      if (changeModeBuzzerIteration == 0) {
        changeModeState = CHANGE_MODE;
      }

      break;
    };

    case CHANGE_MODE: {
      switch (mode) {
        case MODE_LOW_POWER: {
          mode = MODE_HYPRID;
          logger(TRACE, "Hyprid Mode Active");
          break;
        }

        case MODE_HYPRID: {
          mode = MODE_WEB;
          logger(TRACE, "Web Mode Active");
          break;
        }

        case MODE_WEB: {
          mode = MODE_LOW_POWER;
          logger(TRACE, "Low Power Mode Active");
          break;
        }
      }

      changeMode = false;
      preferences.putInt(PREFERENCES_KEY_MODE, mode);
      ESP.restart();
      break;
    };
  }
}

void moduleResetController() {
  switch (moduleResetState) {
    case MODULE_RESET_IDLE: {
      if (moduleReset) {
        moduleResetState = MODULE_RESET_BUZZER_ENABLE;
      }

      break;
    }

    case MODULE_RESET_BUZZER_ENABLE: {
      ledcWrite(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE_ON);
      moduleResetBuzzerTime = millis();
      moduleResetState = MODULE_RESET_BUZZER_TIME_ON;
      break;
    };

    case MODULE_RESET_BUZZER_TIME_ON: {
      if (millis() > moduleResetBuzzerTime + CLEAR_MODULE_BUZZER_TIME) {
        moduleResetState = MODULE_RESET_BUZZER_DISABLE;
      }

      break;
    };

    case MODULE_RESET_BUZZER_DISABLE: {
      ledcWrite(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE_OFF);
      moduleResetBuzzerTime = millis();
      moduleResetState = MODULE_RESET;
      break;
    };

    case MODULE_RESET: {
      setDefaultPreferences();
      logger(TRACE, "Module Cleared");
      ESP.restart();
      break;
    };
  }
}
