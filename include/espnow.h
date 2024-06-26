#ifndef ESPNOW_H
#define ESPNOW_H

#include <Arduino.h>
#include <esp_now.h>

static const byte ESP_NOW_WIFI_CHANNEL_SIZE = 14;
static const byte ESP_NOW_ID_SIZE = 20;
static const byte ESP_NOW_MAC_SIZE = 6;
static const byte ESP_NOW_MAC_BROADCAST[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const byte ESP_NOW_MAC_BLANK[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const int PEER_INFO_REQUEST_RETRYS = 3;
static const int PEER_INFO_REQUEST_AWAIT_TIME = 200;
static const int PEER_INFO_REQUEST_BUZZER_TIME = 5;

enum PeerInfoState {
  PEER_INFO_IDLE,
  PEER_INFO_REQUEST_SEND,
  PEER_INFO_REQUEST_SUCCESS,
  PEER_INFO_REQUEST_TIMEOUT,
  PEER_INFO_AWAIT_RESPONSE,
  PEER_INFO_RECEIVED,
  PEER_INFO_BUZZER_ENABLE,
  PEER_INFO_BUZZER_DISABLE,
  PEER_INFO_BUZZER_TIME_ON,
};

struct EspNowData {
  char id[ESP_NOW_ID_SIZE] = {0};
};

struct EspNowPeerInfoResponse {
  const char id[ESP_NOW_ID_SIZE] = {"PeerInfoResponse"};
  int channel;
  byte mac[ESP_NOW_MAC_SIZE] = {0};
};

struct EspNowPeerInfoRequest {
  const char id[ESP_NOW_ID_SIZE] = {"PeerInfoRequest"};
  int channel;
  byte mac[ESP_NOW_MAC_SIZE] = {0};
};

struct EspNowTouchData {
  const char id[ESP_NOW_ID_SIZE] = {"TouchData"};
  int key;
  int state;
};

struct EspNowHydreonData {
  const char id[ESP_NOW_ID_SIZE] = {"HydreonData"};
  bool status;
};

struct EspNowAnemometerData {
  const char id[ESP_NOW_ID_SIZE] = {"AnemometerData"};
  int velocity;
};

extern PeerInfoState peerInfoState;
extern byte espNowMac[];
extern bool peerInfoRequest;

void setupEspNow();
void taskEspNow();
void onDataReceive(const byte *mac, const byte *incomingData, int len);
void onDataSent(const byte *mac, esp_now_send_status_t status);
void peerInfoSetup();
void peerInfoController();
void receiverPeerInfoResponse(const byte *data);
#ifdef ESP_CU
void receiverPeerInfoRequest(const byte *data, const byte *mac);
void receiverTouchData(const byte *data);
void receiverHydreonData(const byte *data);
void receiverAnemometerData(const byte *data);
#endif
#ifdef ESP_OU
void espNowSendTouchData(int key, bool state);
#endif
#ifdef ESP_SU
void epsNowSendAnemometerData(int velocity);
void espNowSendHydreonData(bool status);
#endif
String macToString(const byte *mac);

#endif