#include "settings.h"

#include <espnow.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <serial.h>
#include <system.h>
#ifdef ESP_SU
#include <anemometer.h>
#include <hydreon.h>
#endif

Preferences preferences;

void setupSettings() {
  preferences.begin(PREFERENCES_NAME, false);

  if (!preferences.isKey(PREFERENCES_KEY_DEFAULTS_LOADED)) {
    setDefaultPreferences();
  }

  mode = (ModeState)preferences.getInt(PREFERENCES_KEY_MODE);
  logLevel = (LogLevel)preferences.getInt(PREFERENCES_KEY_LOG_LEVEL);
  wifiSSID = preferences.getString(PREFERENCES_KEY_WIFI_SSID);
  wifiPassword = preferences.getString(PREFERENCES_KEY_WIFI_PASSWORD);
  wifiChannel = preferences.getInt(PREFERENCES_KEY_WIFI_CHANNEL);
  preferences.getBytes(PREFERENCES_KEY_ESP_NOW_MAC, espNowMac, ESP_NOW_MAC_SIZE);
#ifdef ESP_SU
  hydreonBaudrate = preferences.getInt(PREFERENCES_KEY_HYDREON_BAUDRATE);
  anemometerCalibratedVelocity = preferences.getInt(PREFERENCES_KEY_ANEMOMETER_CALIBRATED_VELOCITY);
  anemometerCalibratedAnalog = preferences.getInt(PREFERENCES_KEY_ANEMOMETER_CALIBRATED_ANALOG);
#endif
}

void setDefaultPreferences() {
  preferences.putBool(PREFERENCES_KEY_DEFAULTS_LOADED, true);
  preferences.putInt(PREFERENCES_KEY_MODE, PREFERENCES_DEFAULT_MODE);
  preferences.putInt(PREFERENCES_KEY_LOG_LEVEL, PREFERENCES_DEFAULT_LOG_LEVEL);
  preferences.putString(PREFERENCES_KEY_WIFI_SSID, PREFERENCES_DEFAULT_WIFI_SSID);
  preferences.putString(PREFERENCES_KEY_WIFI_PASSWORD, PREFERENCES_DEFAULT_WIFI_PASSWORD);
  preferences.putInt(PREFERENCES_KEY_WIFI_CHANNEL, REFERENCES_DEFAULT_WIFI_CHANNEL);
  preferences.putBytes(PREFERENCES_KEY_ESP_NOW_MAC, PREFERENCES_DEFAULT_ESP_NOW_MAC, ESP_NOW_MAC_SIZE);
#ifdef ESP_SU
  preferences.putInt(PREFERENCES_KEY_HYDREON_BAUDRATE, PREFERENCES_DEFAULT_HYREON_BAUDRATE);
  preferences.putInt(PREFERENCES_KEY_ANEMOMETER_CALIBRATED_VELOCITY, PREFERENCES_DEFAULT_ANEMOMETER_VELOCITY);
  preferences.putInt(PREFERENCES_KEY_ANEMOMETER_CALIBRATED_ANALOG, PREFERENCES_DEFAULT_ANEMOMETER_ANALOG);
#endif
}