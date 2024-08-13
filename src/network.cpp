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
  wifiMode = WIFI_AP_STA;

  switch (mode) {
    case MODE_LOW_POWER: {
      wifiInitializeStandalone = true;
      break;
    }

    case MODE_HYPRID: {
      wifiInitializeStandalone = true;
      break;
    }

    case MODE_WEB: {
      wifiInitializeNetwork = true;
      break;
    }
  }
}

void taskNetwork() {
  wifiController();
}

void wifiController() {
  switch (wifiState) {
    case WIFI_IDLE: {
      if (wifiMode == WIFI_MODE_STA) {
        if (WiFi.status() != WL_CONNECTED) {
          logger(TRACE, "WiFi Retry Connect Station");
          wifiState = WIFI_NETWORK_CONNECT_STATION;
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

      break;
    }

    case WIFI_NETWORK_INITIALIZE: {
      wifiSSID = preferences.getString(PREFERENCES_KEY_WIFI_SSID, String());
      wifiPassword = preferences.getString(PREFERENCES_KEY_WIFI_PASSWORD, String());

      if (!WIFI_SSID.isEmpty()) wifiSSID = WIFI_SSID;
      if (!WIFI_PASSWORD.isEmpty()) wifiPassword = WIFI_PASSWORD;

      WiFi.disconnect();
      WiFi.setHostname(MODULE_HOSTNAME.c_str());

      if (!wifiSSID.isEmpty() && !wifiPassword.isEmpty()) {
        logger(TRACE, "WiFi Connect Station");
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
        wifiTime = millis();
        wifiMode = WIFI_STA;
        wifiState = WIFI_NETWORK_CONNECT_STATION;
      } else {
        logger(TRACE, "WiFi Setup Access Point");
        WiFi.softAP(MODULE_SSID.c_str(), NULL, WIFI_AP_CHANNEL, 0);
        WiFi.softAPConfig(WIFI_APIP, WIFI_GATEWAY, WIFI_SUBNET);
        wifiMode = WIFI_AP;
        wifiState = WIFI_NETWORK_ACCESS_POINT;
      }

      break;
    }

    case WIFI_NETWORK_CONNECT_STATION: {
      if (millis() > wifiTime + WIFI_NETWORK_CONNECT_STATION_TIMEOUT) {
        logger(TRACE, "WiFi Connect Station Failed");
        wifiState = WIFI_IDLE;
      }

      if (WiFi.isConnected()) {
        logger(TRACE, "WiFi Station Connected");
        logger(TRACE, "WiFi SSID:      " + WiFi.SSID());
        logger(TRACE, "WiFi Hostname:  " + MODULE_HOSTNAME + ".local");
        logger(TRACE, "WiFi Address:   " + WiFi.localIP().toString());
        logger(TRACE, "WiFi Channel:   " + String(WiFi.channel()));
        wifiAddress = WiFi.localIP();
        wifiChannel = WiFi.channel();
        wifiState = WIFI_NETWORK_MANAGE_CHANNEL;
      }

      break;
    }

    case WIFI_NETWORK_ACCESS_POINT: {
      logger(TRACE, "WiFi Access Point Initialized");
      logger(TRACE, "WiFi SSID:      " + MODULE_SSID);
      logger(TRACE, "WiFi Hostname:  " + MODULE_HOSTNAME + ".local");
      logger(TRACE, "WiFi Address:   " + WIFI_APIP.toString());
      logger(TRACE, "WiFi Channel:   " + String(WiFi.channel()));
      dnsServer.start(53, "*", WiFi.softAPIP());
      wifiAddress = WIFI_APIP;
      wifiChannel = WIFI_AP_CHANNEL;
      wifiState = WIFI_NETWORK_MANAGE_CHANNEL;
      break;
    }

    case WIFI_NETWORK_MANAGE_CHANNEL: {
      if (wifiChannel != preferences.getInt(PREFERENCES_KEY_WIFI_CHANNEL)) {
        preferences.putInt(PREFERENCES_KEY_WIFI_CHANNEL, wifiChannel);
      }

      wifiState = WIFI_IDLE;
      break;
    }

    case WIFI_UPDATE_INITIALIZE:
      logger(TRACE, "WiFi Update Station Setup");
      WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
      wifiTime = millis();
      wifiState = WIFI_UPDATE_CONNECTION;
      break;

    case WIFI_UPDATE_CONNECTION: {
      if (WiFi.isConnected()) {
        logger(TRACE, "WiFi Update Connected");
        preferences.putString(PREFERENCES_KEY_WIFI_SSID, wifiSSID);
        preferences.putString(PREFERENCES_KEY_WIFI_PASSWORD, wifiPassword);
        ESP.restart();
      }

      if (millis() > wifiTime + WIFI_UPDATE_CONNECTION_TIMEOUT) {
        logger(TRACE, "WiFi Update Failed");
        wifiState = WIFI_NETWORK_INITIALIZE;
      }

      break;
    }

    case WIFI_SCANNER_INITIALIZE: {
      logger(TRACE, "WiFi Scanner Initialized");
      WiFi.scanDelete();
      WiFi.scanNetworks(true);
      wifiTime = millis();
      wifiState = WIFI_SCANNER_CONNECT;
      break;
    }

    case WIFI_SCANNER_CONNECT: {
      if (WiFi.scanComplete() > 0) {
        logger(TRACE, "WiFi Scanner Finished");
        for (int i = 0; i < WiFi.scanComplete(); i++) {
          const String ssid = WiFi.SSID(i);
          const String rssi = String(WiFi.RSSI(i));
          logger(TRACE, "RSSI: " + rssi + " - SSID: " + ssid);
        }

        socketTransmitWifi();
        wifiState = WIFI_IDLE;
      }

      if (millis() > wifiTime + WIFI_SCANNER_TIMEOUT) {
        logger(TRACE, "WiFi Scanner Timeout");
        wifiState = WIFI_IDLE;
      }

      break;
    }

    case WIFI_STANDALONE_INITIALIZE: {
      logger(TRACE, "WiFi Standalone Initialized");
      WiFi.disconnect();
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("HIDDEN", NULL, WIFI_AP_CHANNEL, 1);
      wifiMode = WIFI_AP;
      wifiState = WIFI_IDLE;
      break;
    }

    case WIFI_RESET: {
      preferences.putString(PREFERENCES_KEY_WIFI_SSID, wifiSSID);
      preferences.putString(PREFERENCES_KEY_WIFI_PASSWORD, wifiPassword);
      logger(TRACE, "WiFi Cleared", false);
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