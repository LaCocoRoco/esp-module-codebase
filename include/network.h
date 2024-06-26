#ifndef WLAN_H
#define WLAN_H

#include <Arduino.h>

static const int WIFI_NETWORK_CONNECT_STATION_TIMEOUT = 20000;
static const int WIFI_SCANNER_TIMEOUT = 15000;
static const int WIFI_UPDATE_CONNECTION_TIMEOUT = 5000;

static const IPAddress WIFI_APIP = IPAddress(190, 160, 1, 1);
static const IPAddress WIFI_GATEWAY = IPAddress(190, 160, 1, 1);
static const IPAddress WIFI_SUBNET = IPAddress(255, 255, 255, 0);

enum WifiState {
  WIFI_IDLE,
  WIFI_NETWORK_INITIALIZE,
  WIFI_NETWORK_CONNECT_STATION,
  WIFI_NETWORK_ACCESS_POINT,
  WIFI_STANDALONE_INITIALIZE,
  WIFI_SCANNER_INITIALIZE,
  WIFI_SCANNER_CONNECT,
  WIFI_UPDATE_INITIALIZE,
  WIFI_UPDATE_CONNECTION,
  WIFI_RESET,
};

extern WifiState wifiState;
extern String wifiSSID;
extern String wifiPassword;
extern int wifiChannel;
extern bool wifiInitializeNetwork;
extern bool wifiInitializeScanner;
extern bool wifiInitializeUpdate;
extern bool wifiInitializeStandalone;
extern bool wifiReset;
extern IPAddress wifiAddress;

void setupNetwork();
void taskNetwork();
void wifiController();
bool wifiClientConnected();

#endif