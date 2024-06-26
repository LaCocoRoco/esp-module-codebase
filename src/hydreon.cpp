#ifdef ESP_SU

#include "hydreon.h"

#include <esp_wifi.h>
#include <espnow.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <serial.h>
#include <settings.h>
#include <system.h>

HydreonState hydreonState;
HydreonIntensity hydreonIntensity;
bool hydreonStatus;
bool hydreonStatusBuffer;
bool hydreonWakeupEvent;
bool hydreonAutoConnect;
bool hydreonIntensityResponse;
int hydreonBaudrateIteration;
unsigned long hydreonTime;
int hydreonBaudrate;

void taskHydreon() {
  hydreonController();
}

void hydreonController() {
  switch (hydreonState) {
    case HYDREON_IDLE: {
      if (hydreonStatusBuffer != hydreonStatus) {
        hydreonStatusBuffer = hydreonStatus;
        hydreonState = HYDREON_ESP_NOW_SEND_DATA;
      }

      if (hydreonWakeupEvent) {
        hydreonWakeupEvent = false;
        hydreonState = HYDREON_ESP_NOW_SEND_DATA;
      };

      if (hydreonAutoConnect) {
        hydreonState = HYDREON_REQUEST_BAUDRATE;
      }

      break;
    }

    case HYDREON_ESP_NOW_SEND_DATA: {
      espNowSendHydreonData(hydreonStatus);
      hydreonState = HYDREON_IDLE;
      break;
    }

    case HYDREON_REQUEST_BAUDRATE: {
      /*************************************************/
      /* hydreon response managed by serial2Controller */
      /*************************************************/
      int baudrate = 0;
      switch (hydreonBaudrateIteration++) {
        case 0: baudrate = 1200; break;
        case 1: baudrate = 2400; break;
        case 2: baudrate = 4800; break;
        case 3: baudrate = 9600; break;
        case 4: baudrate = 19200; break;
        case 5: baudrate = 38400; break;
        case 6: baudrate = 57600; break;
      }

      logger(TRACE, "Hydreon Test Baudrate: " + String(baudrate));

      Serial2.flush();
      Serial2.end();
      Serial2.begin(baudrate, SERIAL_8N1, GPIO_RX1, GPIO_TX1);
      Serial2.println();
      Serial2.println(HYDREON_BAUDRATE);

      hydreonTime = millis();
      hydreonState = HYDREON_RESPONSE_BAUDRATE;
      break;
    }

    case HYDREON_RESPONSE_BAUDRATE: {
      if (millis() > hydreonTime + HYDREON_BAUDRATE_INTERVALL) {
        hydreonState = HYDREON_REQUEST_BAUDRATE;
      }

      if (!hydreonAutoConnect) {
        logger(TRACE, "Hydreon Connection Success");
        hydreonBaudrateIteration = 0;
        hydreonState = HYDREON_IDLE;
      }

      if (hydreonBaudrateIteration > 6) {
        logger(TRACE, "Hydreon Connection Failed");
        hydreonAutoConnect = false;
        hydreonBaudrateIteration = 0;
        // reset baudrate to default
        Serial2.begin(hydreonBaudrate, SERIAL_8N1, GPIO_RX1, GPIO_TX1);
        hydreonState = HYDREON_IDLE;
      }

      break;
    }
  }
}

void hydreonResponseHandler(String value) {
  logger(DEBUG, "Hydreon Response: " + value);

  // hydreon intensity repsonse
  if (value.indexOf("R ") != -1) {
    value.remove(0, 2);

    if (value.indexOf(" TooCold") != -1) {
      int index = value.indexOf(" TooCold");
      int count = value.length() - 1;
      value.remove(index, count);
    }

    hydreonIntensity = (HydreonIntensity)value.toInt();
  }

  // hydreon baudrate response
  if (value.indexOf("Baud ") != -1) {
    value.remove(0, 5);
    hydreonBaudrate = value.toInt();
    preferences.putInt(PREFERENCES_KEY_HYDREON_BAUDRATE, hydreonBaudrate);

    // update serial2 baudrate
    Serial2.flush();
    Serial2.end();
    Serial2.begin(hydreonBaudrate, SERIAL_8N1, GPIO_RX1, GPIO_TX1);
    Serial2.println();

    // reset auto connect
    hydreonAutoConnect = false;

    // log baudrate response
    logger(DEBUG, "Hydreon Connection updated");
  }
}

int hydreonBaudrateToCode(int baudrate) {
  switch (baudrate) {
    case 1200: return BAUDRATE_1200; break;
    case 2400: return BAUDRATE_2400; break;
    case 4800: return BAUDRATE_4800; break;
    case 9600: return BAUDRATE_9600; break;
    case 19200: return BAUDRATE_19200; break;
    case 38400: return BAUDRATE_38400; break;
    case 57600: return BAUDRATE_57600; break;
    default: return -1;
  }
}

#endif
