#include "network.h"

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Wifi.h>
#include <cmd.h>
#include <config.h>
#include <logger.h>
#include <serial.h>
#include <settings.h>
#include <socket.h>
#include <system.h>
#include <web.h>

String wifiSSID;
String wifiPassword;
int wifiChannel;
wifi_mode_t wifiMode;
WifiState wifiState;
IPAddress wifiAddress;
bool wifiInitializeNetwork;
bool wifiInitializeScanner;
bool wifiInitializeUpdate;
bool wifiInitializeStandalone;
bool wifiReset;
unsigned long wifiTime;
int scanner;

void setupNetwork() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("HIDDEN", NULL, wifiChannel, 1);
  wifiMode = WIFI_AP_STA;
  wifiInitializeNetwork = true;
}

void taskNetwork() {
  wifiController();
}

void wifiController() {
  switch (wifiState) {
    case WIFI_IDLE: {
      if (mode == MODE_WEB) {
        if (wifiMode == WIFI_MODE_STA) {
          // ESP-NOW only works reliably in WIFI_AP_STA mode.
          // Therefore, we use a custom WiFiMode to keep track of it.
          if (WiFi.status() != WL_CONNECTED) {
            logger(TRACE, "Wifi Retry Connect Station");
            wifiState = WIFI_NETWORK_INITIALIZE;
          }
        }

        if (wifiInitializeNetwork) {
          wifiInitializeNetwork = false;
          wifiState = WIFI_NETWORK_INITIALIZE;
        }

        if (wifiInitializeUpdate) {
          wifiInitializeUpdate = false;
          wifiState = WIFI_UPDATE_INITIALIZE;
        }

        if (wifiInitializeScanner) {
          wifiInitializeScanner = false;
          wifiState = WIFI_SCANNER_INITIALIZE;
        }

        if (wifiInitializeStandalone) {
          wifiInitializeStandalone = false;
          wifiState = WIFI_STANDALONE_INITIALIZE;
        }

        if (wifiReset) {
          wifiReset = false;
          wifiState = WIFI_RESET;
        }
      }
      break;
    }

    case WIFI_NETWORK_INITIALIZE: {
      wifiSSID = preferences.getString(PREFERENCES_KEY_WIFI_SSID, String());
      wifiPassword = preferences.getString(PREFERENCES_KEY_WIFI_PASSWORD, String());
      WiFi.disconnect();
      WiFi.setHostname(MODULE_HOSTNAME.c_str());
      WiFi.mode(WIFI_AP_STA);

      if (!wifiSSID.isEmpty() && !wifiPassword.isEmpty()) {
        logger(TRACE, "Wifi Connect Station");
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
        wifiTime = millis();
        wifiMode = WIFI_STA;
        wifiState = WIFI_NETWORK_CONNECT_STATION;
      } else {
        logger(TRACE, "Wifi Setup Access Point");
        WiFi.softAP(MODULE_SSID.c_str(), NULL, wifiChannel, 0);
        WiFi.softAPConfig(WIFI_APIP, WIFI_GATEWAY, WIFI_SUBNET);
        wifiMode = WIFI_AP;
        wifiState = WIFI_NETWORK_ACCESS_POINT;
      }

      break;
    }

    case WIFI_NETWORK_CONNECT_STATION: {
      if (millis() > wifiTime + WIFI_NETWORK_CONNECT_STATION_TIMEOUT) {
        logger(TRACE, "Wifi Connect Station Failed");
        wifiState = WIFI_IDLE;
      }

      if (WiFi.isConnected()) {
        logger(TRACE, "Wifi Station Connected");
        logger(TRACE, "SSID:      " + WiFi.SSID());
        logger(TRACE, "Hostname:  " + MODULE_HOSTNAME + ".local");
        logger(TRACE, "IPAddress: " + WiFi.localIP().toString());
        logger(TRACE, "Channel:   " + String(WiFi.channel()));
        wifiAddress = WiFi.localIP();
        wifiState = WIFI_IDLE;
      }

      break;
    }

    case WIFI_NETWORK_ACCESS_POINT: {
      logger(TRACE, "Wifi Access Point Initialized");
      logger(TRACE, "SSID:      " + MODULE_SSID);
      logger(TRACE, "Hostname:  " + MODULE_HOSTNAME + ".local");
      logger(TRACE, "IPAddress: " + WIFI_APIP.toString());
      logger(TRACE, "Channel:   " + String(WiFi.channel()));
      dnsServer.start(53, "*", WiFi.softAPIP());
      wifiAddress = WIFI_APIP;
      wifiState = WIFI_IDLE;
      break;
    }

    case WIFI_UPDATE_INITIALIZE:
      logger(TRACE, "Wifi Update Station Setup");
      WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
      wifiTime = millis();
      wifiState = WIFI_UPDATE_CONNECTION;
      break;

    case WIFI_UPDATE_CONNECTION: {
      if (WiFi.isConnected()) {
        logger(TRACE, "Wifi Update Connected");
        preferences.putString(PREFERENCES_KEY_WIFI_SSID, wifiSSID);
        preferences.putString(PREFERENCES_KEY_WIFI_PASSWORD, wifiPassword);
        wifiState = WIFI_NETWORK_INITIALIZE;
      }

      if (millis() > wifiTime + WIFI_UPDATE_CONNECTION_TIMEOUT) {
        logger(TRACE, "Wifi Update Failed");
        wifiState = WIFI_NETWORK_INITIALIZE;
      }

      break;
    }

    case WIFI_SCANNER_INITIALIZE: {
      logger(TRACE, "Wifi Scanner Initialized");
      WiFi.scanDelete();
      WiFi.scanNetworks(true);
      wifiTime = millis();
      wifiState = WIFI_SCANNER_CONNECT;
      break;
    }

    case WIFI_SCANNER_CONNECT: {
      if (WiFi.scanComplete() > 0) {
        logger(TRACE, "Wifi Scanner Finished");
        for (int i = 0; i < WiFi.scanComplete(); i++) {
          const String ssid = WiFi.SSID(i);
          const String rssi = String(WiFi.RSSI(i));
          logger(TRACE, "RSSI: " + rssi + " - SSID: " + ssid);
        }

        socketTransmitWifi();
        wifiState = WIFI_IDLE;
      }

      if (millis() > wifiTime + WIFI_SCANNER_TIMEOUT) {
        logger(TRACE, "Wifi Scanner Timeout");
        wifiState = WIFI_IDLE;
      }

      break;
    }

    case WIFI_STANDALONE_INITIALIZE: {
      logger(TRACE, "Wifi Standalone Initialized");
      WiFi.disconnect();
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("HIDDEN", NULL, wifiChannel, 1);
      wifiMode = WIFI_AP_STA;
      wifiState = WIFI_IDLE;
      break;
    }

    case WIFI_RESET: {
      preferences.putString(PREFERENCES_KEY_WIFI_SSID, wifiSSID);
      preferences.putString(PREFERENCES_KEY_WIFI_PASSWORD, wifiPassword);
      logger(TRACE, "Wifi Cleared", false);
      wifiState = WIFI_NETWORK_INITIALIZE;
      break;
    }
  }
}

bool wifiClientConnected() {
  bool clientStationConnected = WiFi.status() != WL_CONNECTED;
  bool clientAccessPointConnected = WiFi.softAPgetStationNum() < 1;
  return clientStationConnected || clientAccessPointConnected;
}