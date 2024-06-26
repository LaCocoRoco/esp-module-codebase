#ifdef ESP_SU

#ifndef HYDREON_H
#define HYDREON_H

#include <Arduino.h>

static const char HYDREON_READ_DATA = 'R';
static const char HYDREON_KILL = 'K';
static const char HYDREON_POLLING_MODE = 'P';
static const char HYDREON_CONTINUES_MODE = 'C';
static const char HYDREON_BAUDRATE = 'B';

static const int HYDREON_BAUDRATE_INTERVALL = 2000;

enum HydreonBaudrate {
  BAUDRATE_1200,
  BAUDRATE_2400,
  BAUDRATE_4800,
  BAUDRATE_9600,
  BAUDRATE_19200,
  BAUDRATE_38400,
  BAUDRATE_57600
};

enum HydreonIntensity {
  INTENSITY_NONE,
  INTENSITY_RAIN_DROPS,
  INTENSITY_VERY_LIGHT,
  INTENSITY_MEDIUM_LIGHT,
  INTENSITY_MEDIUM,
  INTENSITY_MEDIUM_HEAVY,
  INTENSITY_HEAVY,
  INTENSITY_VIOLENT
};

enum HydreonState {
  HYDREON_IDLE,
  HYDREON_ESP_NOW_SEND_DATA,
  HYDREON_RESPONSE_BAUDRATE,
  HYDREON_REQUEST_BAUDRATE
};

extern HydreonState hydreonState;
extern HydreonIntensity hydreonIntensity;
extern bool hydreonStatus;
extern bool hydreonWakeupEvent;
extern bool hydreonAutoConnect;
extern int hydreonBaudrate;

void taskHydreon();
void hydreonController();
void hydreonResponseHandler(String value);
int hydreonBaudrateToCode(int baudrate);

#endif
#endif