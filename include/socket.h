#ifndef SOCKET_H
#define SOCKET_H

#include <Arduino.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

static const String SOCKET_URL = "/data";

static const int SOCKET_TRANSMIT_INTERVAL_FAST = 50;
static const int SOCKET_TRANSMIT_INTERVAL_SLOW = 1000;

enum SocketState {
  SOCKET_IDLE,
  SOCKET_UPDATE_INTERVALL_FAST,
  SOCKET_UPDATE_INTERVALL_SLOW
};

extern AsyncWebSocket webSocket;
extern SocketState socketState;

void setupSocket();
void taskSocket();
void socketController();
void socketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, byte *data, size_t len);
void socketEventData(AsyncWebSocketClient *client, void *arg, byte *data, size_t len);
void socketEventConnect(AsyncWebSocketClient *client);
void socketReceive(const JsonObject document);
void socketReceiveButton(const JsonObject data);
void socketReceiveWifi(const JsonObject data);
void socketReceiveTerminal(const JsonObject data);
void socketTransmit(const int id, const String data);
void socketTransmitTerminal(const String text, const int id = 0);
void socketTransmitWifi(const int id = 0);
void socketTransmitUpdate(bool update = false);
bool socketEvaluateTerminalData(const JsonObject *data);
#ifdef ESP_CU
bool socketEvaluateSensorData(const JsonObject *data);
bool socketEvaluateInputData(const JsonObject *data);
#endif
#ifdef ESP_OU
void socketReceiveTouch(const JsonObject data);
bool socketEvaluateTouchData(const JsonObject *data);
#endif
#ifdef ESP_SU
bool socketEvaluateSensorData(const JsonObject *data);
#endif

#endif