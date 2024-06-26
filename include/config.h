#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#ifdef ESP_CU
static const String MODULE_HWV = "1.0";
static const String MODULE_SWV = "1.0";
static const String MODULE_NAME = "CU";
static const String MODULE_SSID = "ESPCU";
static const String MODULE_HOSTNAME = "espcu";
#endif
#ifdef ESP_OU
static const String MODULE_HWV = "1.0";
static const String MODULE_SWV = "1.0";
static const String MODULE_NAME = "CU";
static const String MODULE_SSID = "ESPOU";
static const String MODULE_HOSTNAME = "espou";
#endif
#ifdef ESP_SU
static const String MODULE_HWV = "1.0";
static const String MODULE_SWV = "1.0";
static const String MODULE_NAME = "CU";
static const String MODULE_SSID = "ESPSU";
static const String MODULE_HOSTNAME = "espsu";
#endif

#endif