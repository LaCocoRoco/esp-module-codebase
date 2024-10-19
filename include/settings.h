#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <Arduino.h>
#include <Preferences.h>
#include <logger.h>
#include <system.h>

static const ModeState PREFERENCES_DEFAULT_MODE = MODE_WEB;
static const LogLevel PREFERENCES_DEFAULT_LOG_LEVEL = INFO;
static const String PREFERENCES_DEFAULT_WIFI_SSID = "";
static const String PREFERENCES_DEFAULT_WIFI_PASSWORD = "";
static const int REFERENCES_DEFAULT_WIFI_CHANNEL = 1;
static const byte PREFERENCES_DEFAULT_ESP_NOW_MAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#ifdef ESP_SU
static const int PREFERENCES_DEFAULT_HYREON_BAUDRATE = 9600;
static const int PREFERENCES_DEFAULT_ANEMOMETER_VELOCITY = 50;
static const int PREFERENCES_DEFAULT_ANEMOMETER_ANALOG = 500;
#endif

static const char* PREFERENCES_NAME = "preferences";
static const char* PREFERENCES_KEY_DEFAULTS_LOADED = "defaults";
static const char* PREFERENCES_KEY_MODE = "mode";
static const char* PREFERENCES_KEY_LOG_LEVEL = "logLevel";
static const char* PREFERENCES_KEY_WIFI_SSID = "wifiSSID";
static const char* PREFERENCES_KEY_WIFI_PASSWORD = "wifiPassword";
static const char* PREFERENCES_KEY_WIFI_CHANNEL = "wifiChannel";
static const char* PREFERENCES_KEY_ESP_NOW_MAC = "espNowMac";
#ifdef ESP_SU
static const char* PREFERENCES_KEY_HYDREON_BAUDRATE = "hydreon";
static const char* PREFERENCES_KEY_ANEMOMETER_CALIBRATED_VELOCITY = "velocity";
static const char* PREFERENCES_KEY_ANEMOMETER_CALIBRATED_ANALOG = "analog";
#endif
#ifdef ESP_OU
static const char* PREFERENCES_KEY_TOUCH_DEFAULTS_LOADED = "touchDefaults";
#endif

extern Preferences preferences;

void setupSettings();
void setDefaultPreferences();

#endif