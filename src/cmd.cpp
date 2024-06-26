#include "cmd.h"

#include <espnow.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <serial.h>
#include <settings.h>
#include <system.h>
#ifdef ESP_SU
#include <hydreon.h>
#endif
#ifdef ESP_OU
#include <touch.h>
#include <twi.h>
#endif

void cmdHandler(String text) {
  text.toLowerCase();
  int startIndex = 0;
  int spaceIndex = 0;
  int tokenLength = 3;

  String token[tokenLength];
  for (int index = 0; index < tokenLength; index++) {
    int spaceIndex = text.indexOf(' ', startIndex);
    if (spaceIndex != -1 && index != tokenLength - 1) {
      token[index] = text.substring(startIndex, spaceIndex);
      startIndex = spaceIndex + 1;
    } else {
      token[index] = text.substring(startIndex, spaceIndex);
      break;
    }
  }

  const String cmd1 = token[0];
  const String cmd2 = token[1];
  const String cmd3 = token[2];

  if (cmd1 == "help") {
    logger(TRACE, "Available Terminal commands");
    logger(TRACE, "AT RESET WIFI:   Reset Wifi");
    logger(TRACE, "AT RESET MODULE: Reset Wodule");
    logger(TRACE, "AT RESTART:      Restart Module");
    logger(TRACE, "AT SCANNER:      Scan for Wifi Networks");
    logger(TRACE, "AT PEERINFO:     Request new PeerInfo");
    logger(TRACE, "AT LOG TRACE:    Log Level Trace");
    logger(TRACE, "AT LOG DEBUG:    Log Level Debug");
    logger(TRACE, "AT LOG INFO:     Log Level Info");
    logger(TRACE, "AT LOG WARNING:  Log Level Warning");
    logger(TRACE, "AT LOG ERROR:    Log Level Error");
    logger(TRACE, "AT MODE LOW:     Activate Low Power Mode");
    logger(TRACE, "AT MODE HYPRID:  Activate Hyprid Mode");
    logger(TRACE, "AT MODE WEB:     Activate Web Mode");

#ifdef ESP_OU
    logger(TRACE, "TC REGISTER:     Log Touch Register Data");
    logger(TRACE, "TC CALIBRATE:    Calibrate Touch Sensor");
#endif

#ifdef ESP_SU
    logger(TRACE, "HD READ:         Hydreon Read Data");
    logger(TRACE, "HD RESTART:      Hydreon Restart");
    logger(TRACE, "HD POLLING:      Hydreon Polling Mode");
    logger(TRACE, "HD CONTINUES:    Hydreon Continues Mode");
    logger(TRACE, "HD AUTO:         Hydreon Auto Connect");
    logger(TRACE, "HD BAUD:         Hydreon Get Baudrate");
    logger(TRACE, "HD BAUD xxxxx:   Hydreon Set Baudrate");
#endif
  }

  if (cmd1 == "at") {
    if (cmd2 == "reset") {
      if (cmd3 == "wifi") {
        logger(TRACE, "Reset Wifi");
        wifiReset = true;
      }

      if (cmd3 == "module") {
        logger(TRACE, "Reset Module");
        moduleReset = true;
      }
    }

    if (cmd2 == "scanner") {
      logger(TRACE, "Scan for Network");
      wifiInitializeScanner = true;
    }

    if (cmd2 == "peerinfo") {
      logger(TRACE, "Request new PeerInfo");
      peerInfoRequest = true;
    }

    if (cmd2 == "restart") {
      logger(TRACE, "Restart Module");
      ESP.restart();
    }

    if (cmd2 == "log") {
      if (cmd3 == "trace") {
        logger(TRACE, "Log Level TRACE");
        logLevel = TRACE;
        preferences.putInt("logLevel", logLevel);
      }

      if (cmd3 == "debug") {
        logger(TRACE, "Log Level DEBUG");
        logLevel = DEBUG;
        preferences.putInt("logLevel", logLevel);
      }

      if (cmd3 == "info") {
        logger(TRACE, "Log Level INFO");
        logLevel = INFO;
        preferences.putInt("logLevel", logLevel);
      }

      if (cmd3 == "warning") {
        logger(TRACE, "Log Level WARNING");
        logLevel = WARNING;
        preferences.putInt("logLevel", logLevel);
      }

      if (cmd3 == "error") {
        logger(TRACE, "Log Level ERROR");
        logLevel = ERROR;
        preferences.putInt("logLevel", logLevel);
      }
    }

    if (cmd2 == "mode") {
      if (cmd3 == "low") {
        logger(TRACE, "Activate Low Power Mode");
        preferences.putInt("mode", MODE_LOW_POWER);
        ESP.restart();
      }

      if (cmd3 == "hyprid") {
        logger(TRACE, "Activate Hypride Mode");
        preferences.putInt("mode", MODE_HYPRID);
        ESP.restart();
      }

      if (cmd3 == "web") {
        logger(TRACE, "Activate Web Mode");
        preferences.putInt("mode", MODE_WEB);
        ESP.restart();
      }
    }
  }

#ifdef ESP_OU
  if (cmd1 == "tc") {
    if (cmd2 == "register") {
      logger(TRACE, "Log Touch Register");
      touchReadRegisterData();
      touchLogRegisterData(at42qt2120);
    }

    if (cmd2 == "calibrate") {
      logger(TRACE, "Calibrate Touch");
      twiWrite(AT42QT2120_ADDRESS, CALIBRATE, true);
    }
  }
#endif

#ifdef ESP_SU
  if (cmd1 == "hd") {
    if (cmd2 == "read") {
      logger(TRACE, "Hydrean Request Read Data");
      Serial2.println(HYDREON_READ_DATA);
    }

    if (cmd2 == "restart") {
      logger(TRACE, "Hydrean Request Restart");
      Serial2.println(HYDREON_KILL);
    }

    if (cmd2 == "polling") {
      logger(TRACE, "Hydrean Request Polling Mode");
      Serial2.println(HYDREON_POLLING_MODE);
    }

    if (cmd2 == "continues") {
      logger(TRACE, "Hydrean Request Continues Mode");
      Serial2.println(HYDREON_CONTINUES_MODE);
    }

    if (cmd2 == "auto") {
      hydreonAutoConnect = true;
    }

    if (cmd2 == "baud") {
      if (cmd3.isEmpty()) {
        logger(TRACE, "Hydrean Get Baudrate");
        Serial2.println(HYDREON_BAUDRATE);
      } else {
        /*************************************************/
        /* hydreon response managed by serial2Controller */
        /*************************************************/
        int baudrate = cmd3.toInt();
        int code = hydreonBaudrateToCode(baudrate);
        if (code != -1) {
          Serial2.print(HYDREON_BAUDRATE);
          Serial2.print(' ');
          Serial2.println(code);
          logger(TRACE, "Hydreon Request Baudrate: " + String(baudrate));
        } else {
          logger(TRACE, "Hydreon Request Baudrate Error");
        }
      }
    }
  }
#endif
}