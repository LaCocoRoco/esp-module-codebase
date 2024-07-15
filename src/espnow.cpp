#include "espnow.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <settings.h>
#include <sleep.h>
#ifdef ESP_CU
#include <io.h>
#endif
#ifdef ESP_SU
#include <anemometer.h>
#include <hydreon.h>
#endif

PeerInfoState peerInfoState;
esp_now_peer_info_t peerInfo;
unsigned long peerInfoTime;
int peerInfoRequestChannel;
int peerInfoRequestRetrys;
bool peerInfoRequest;
bool peerInfoReceived;
RTC_DATA_ATTR bool espNowSendError;
byte espNowMac[ESP_NOW_MAC_SIZE] = {0};
EspNowData espNowData;
EspNowPeerInfoResponse espNowPeerInfoResponse;
EspNowPeerInfoRequest espNowPeerInfoRequest;
EspNowTouchData espNowTouchData;
EspNowHydreonData espNowHydreonData;
EspNowAnemometerData espNowAnemometerData;

void setupEspNow() {
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceive);
#ifndef ESP_CU
  peerInfoSetup();
#endif
}

void taskEspNow() {
#ifndef ESP_CU
  peerInfoController();
#endif
}

void onDataSent(const byte *mac, const esp_now_send_status_t status) {
  if (!peerInfoRequest) {
    if (status != ESP_NOW_SEND_SUCCESS) {
      // It is not immediately clear why the EspNow transmission failed.
      // The receiver might be unreachable, or there could be an issue with the WiFi channel configuration.
      // If the module is operating in low power mode, it is also not directly connected to the WiFi network.
      // Consequently, there is a possibility that the network has changed the channel.
      // As an error routine, a new peer information request is initiated.
      // However, this could drain the battery due to the time required for execution.
      logger(TRACE, "EspNow Send Failure");
      peerInfoRequest = true;
      espNowSendError = true;
    } else {
      // TODO: should only happen when peerInfoRequest is finished
      // TODO: hopefully can be used for failure recovery
      espNowSendError = false;
    }
  }
}

void onDataReceive(const byte *mac, const byte *data, const int len) {
  memcpy(&espNowData, data, sizeof(EspNowData));
  resetSleep = true;
#ifdef ESP_CU
  receiverPeerInfoRequest(data, mac);
  receiverTouchData(data);
  receiverHydreonData(data);
  receiverAnemometerData(data);
#endif
#ifndef ESP_CU
  receiverPeerInfoResponse(data);
#endif
}

void peerInfoSetup() {
  memcpy(peerInfo.peer_addr, ESP_NOW_MAC_BROADCAST, ESP_NOW_MAC_SIZE);
  esp_now_add_peer(&peerInfo);
  memcpy(peerInfo.peer_addr, espNowMac, ESP_NOW_MAC_SIZE);
  esp_now_add_peer(&peerInfo);

  const byte cmpDefault = memcmp(espNowMac, ESP_NOW_MAC_BLANK, ESP_NOW_MAC_SIZE);
  const bool isDefault = cmpDefault == 0 ? true : false;
  const byte cmpBroadcast = memcmp(espNowMac, ESP_NOW_MAC_BROADCAST, ESP_NOW_MAC_SIZE);
  const bool isBroadcast = cmpBroadcast == 0 ? true : false;
  if (isDefault || isBroadcast) peerInfoRequest = true;
}

void peerInfoController() {
  switch (peerInfoState) {
    case PEER_INFO_IDLE: {
      if (wifiState == WIFI_IDLE) {
        if (peerInfoRequest) {
          logger(TRACE, "Peer Info Request Initialized");
          peerInfoRequestRetrys = 0;
          peerInfoRequestChannel = 1;
          peerInfoState = PEER_INFO_REQUEST_SEND;
        }
      }

      break;
    }

    case PEER_INFO_REQUEST_SEND: {
      espNowPeerInfoRequest.channel = peerInfoRequestChannel;
      esp_read_mac(espNowPeerInfoRequest.mac, ESP_MAC_WIFI_STA);
      esp_wifi_set_channel(peerInfoRequestChannel, WIFI_SECOND_CHAN_NONE);
      esp_now_send(ESP_NOW_MAC_BROADCAST, (byte *)&espNowPeerInfoRequest, sizeof(EspNowPeerInfoRequest));

      logger(DEBUG, "Send Peer Info Channel: " + String(espNowPeerInfoRequest.channel));
      logger(DEBUG, "Send Peer Info Address: " + macToString(espNowPeerInfoRequest.mac));

      peerInfoTime = millis();
      peerInfoState = PEER_INFO_AWAIT_RESPONSE;
      break;
    }

    case PEER_INFO_AWAIT_RESPONSE: {
      if (millis() > peerInfoTime + PEER_INFO_REQUEST_AWAIT_TIME) {
        if (peerInfoReceived) {
          peerInfoState = PEER_INFO_REQUEST_SUCCESS;
        } else if (peerInfoRequestRetrys > PEER_INFO_REQUEST_RETRYS) {
          peerInfoState = PEER_INFO_REQUEST_TIMEOUT;
        } else if (peerInfoRequestChannel > ESP_NOW_WIFI_CHANNEL_SIZE) {
          peerInfoRequestRetrys++;
          peerInfoRequestChannel = 0;
          peerInfoState = PEER_INFO_REQUEST_SEND;
        } else {
          peerInfoRequestChannel++;
          peerInfoState = PEER_INFO_REQUEST_SEND;
        }
      }

      break;
    }

    case PEER_INFO_REQUEST_TIMEOUT: {
      logger(TRACE, "Peer Info Not Received");
      wifiInitializeNetwork = true;
      peerInfoRequest = false;
      peerInfoState = PEER_INFO_IDLE;
      break;
    }

    case PEER_INFO_REQUEST_SUCCESS: {
      peerInfoState = PEER_INFO_BUZZER_ENABLE;
      break;
    }

    case PEER_INFO_BUZZER_ENABLE: {
      ledcWrite(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE_ON);
      peerInfoTime = millis();
      peerInfoState = PEER_INFO_BUZZER_TIME_ON;
      break;
    };

    case PEER_INFO_BUZZER_TIME_ON: {
      if (millis() > peerInfoTime + PEER_INFO_REQUEST_BUZZER_TIME) {
        peerInfoState = PEER_INFO_BUZZER_DISABLE;
      }

      break;
    };

    case PEER_INFO_BUZZER_DISABLE: {
      ledcWrite(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE_OFF);
      peerInfoState = PEER_INFO_RECEIVED;
      break;
    };

    case PEER_INFO_RECEIVED: {
      logger(TRACE, "Peer Info Received");
      wifiChannel = preferences.getInt(PREFERENCES_KEY_WIFI_CHANNEL);
      preferences.getBytes(PREFERENCES_KEY_ESP_NOW_MAC, espNowMac, ESP_NOW_MAC_SIZE);
      memcpy(peerInfo.peer_addr, espNowMac, ESP_NOW_MAC_SIZE);
      esp_now_add_peer(&peerInfo);
      wifiInitializeNetwork = true;
      peerInfoRequest = false;
      peerInfoReceived = false;
      peerInfoState = PEER_INFO_IDLE;
      break;
    }
  }
};

void receiverPeerInfoResponse(const byte *data) {
  if (String(espNowData.id) == String(espNowPeerInfoResponse.id)) {
    memcpy(&espNowPeerInfoResponse, data, sizeof(EspNowPeerInfoResponse));

    if (espNowPeerInfoResponse.channel != wifiChannel) {
      preferences.putInt(PREFERENCES_KEY_WIFI_CHANNEL, espNowPeerInfoResponse.channel);
    }

    if (memcmp(espNowPeerInfoResponse.mac, ::espNowMac, ESP_NOW_MAC_SIZE)) {
      preferences.putBytes(PREFERENCES_KEY_ESP_NOW_MAC, espNowPeerInfoResponse.mac, ESP_NOW_MAC_SIZE);
    }

    logger(DEBUG, "Recv Peer Info Channel: " + String(espNowPeerInfoResponse.channel));
    logger(DEBUG, "Recv Peer Info Address: " + macToString(espNowPeerInfoResponse.mac));

    peerInfoReceived = true;
  }
}

#ifdef ESP_CU
void receiverPeerInfoRequest(const byte *data, const byte *mac) {
  if (String(espNowData.id) == String(espNowPeerInfoRequest.id)) {
    memcpy(&espNowPeerInfoRequest, data, sizeof(EspNowPeerInfoRequest));

    memcpy(peerInfo.peer_addr, espNowPeerInfoRequest.mac, ESP_NOW_MAC_SIZE);
    esp_now_add_peer(&peerInfo);

    logger(DEBUG, "Recv Peer Info Channel: " + String(espNowPeerInfoRequest.channel));
    logger(DEBUG, "Recv Peer Info Address: " + macToString(espNowPeerInfoRequest.mac));

    espNowPeerInfoResponse.channel = WiFi.channel();
    esp_read_mac(espNowPeerInfoResponse.mac, ESP_MAC_WIFI_STA);
    esp_now_send(mac, (byte *)&espNowPeerInfoResponse, sizeof(EspNowPeerInfoResponse));

    logger(DEBUG, "Send Peer Info Channel: " + String(espNowPeerInfoResponse.channel));
    logger(DEBUG, "Send Peer Info Address: " + macToString(espNowPeerInfoResponse.mac));
  }
}

void receiverTouchData(const byte *data) {
  if (String(espNowData.id) == String(espNowTouchData.id)) {
    memcpy(&espNowTouchData, data, sizeof(EspNowTouchData));

    switch (espNowTouchData.key) {
      case 0: io.inputs.input01 = espNowTouchData.state; break;
      case 1: io.inputs.input02 = espNowTouchData.state; break;
      case 2: io.inputs.input03 = espNowTouchData.state; break;
      case 3: io.inputs.input04 = espNowTouchData.state; break;
      case 4: io.inputs.input05 = espNowTouchData.state; break;
      case 5: io.inputs.input06 = espNowTouchData.state; break;
      case 6: io.inputs.input07 = espNowTouchData.state; break;
      case 7: io.inputs.input08 = espNowTouchData.state; break;
      case 8: io.inputs.input09 = espNowTouchData.state; break;
      case 9: io.inputs.input10 = espNowTouchData.state; break;
      case 10: io.inputs.input11 = espNowTouchData.state; break;
      case 11: io.inputs.input12 = espNowTouchData.state; break;
    }

    logger(DEBUG, "Recv Touch Key:   " + String(espNowTouchData.key));
    logger(DEBUG, "Recv Touch State: " + String(espNowTouchData.state));
  }
}

void receiverHydreonData(const byte *data) {
  if (String(espNowData.id) == String(espNowHydreonData.id)) {
    memcpy(&espNowHydreonData, data, sizeof(EspNowHydreonData));
    hydreonStatus = espNowHydreonData.status;

    logger(DEBUG, "Recv Hydreon Status: " + String(espNowHydreonData.status));
  }
}

void receiverAnemometerData(const byte *data) {
  if (String(espNowData.id) == String(espNowAnemometerData.id)) {
    memcpy(&espNowAnemometerData, data, sizeof(EspNowAnemometerData));
    anemometerVelocity = espNowAnemometerData.velocity;

    logger(DEBUG, "Recv Anemometer Velocity: " + String(espNowAnemometerData.velocity));
  }
}
#endif

#ifdef ESP_OU
void espNowSendTouchData(int key, bool state) {
  if (peerInfoRequest) return;

  espNowTouchData.key = key;
  espNowTouchData.state = state;
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);
  esp_now_send(espNowMac, (byte *)&espNowTouchData, sizeof(EspNowTouchData));

  logger(DEBUG, "Send Touch Key:   " + String(espNowTouchData.key));
  logger(DEBUG, "Send Touch State: " + String(espNowTouchData.state));
}
#endif

#ifdef ESP_SU
void epsNowSendAnemometerData(int velocity) {
  if (peerInfoRequest) return;

  espNowAnemometerData.velocity = velocity;
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);
  esp_now_send(espNowMac, (byte *)&espNowAnemometerData, sizeof(EspNowAnemometerData));

  logger(DEBUG, "Send Anemometer Velocity: " + String(espNowAnemometerData.velocity));
}

void espNowSendHydreonData(bool status) {
  if (peerInfoRequest) return;

  espNowHydreonData.status = status;
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);
  esp_now_send(espNowMac, (byte *)&espNowHydreonData, sizeof(EspNowHydreonData));

  logger(DEBUG, "Send Hydreon Status: " + String(espNowHydreonData.status));
}
#endif

String macToString(const byte *mac) {
  String m = String(mac[0], HEX);
  m += ":" + String(mac[1], HEX);
  m += ":" + String(mac[2], HEX);
  m += ":" + String(mac[3], HEX);
  m += ":" + String(mac[4], HEX);
  m += ":" + String(mac[5], HEX);
  m.toUpperCase();
  return m;
}