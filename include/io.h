#ifdef ESP_CU

#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>
#include <driver/adc.h>

const int INPUT_SIZE = 12;
const int OUTPUT_SIZE = 16;
const int MOTOR_MAX_ON_TIME = 60000;
const int LIGHT_DIMMER_TIME = 200;

enum LightState {
  LIGHT_IDLE,
  LIGHT_DIMMER_ENABLE,
  LIGHT_DIMMER_DISABLE,
  LIGHT_DIMMER_TIME_ON,
  LIGHT_DIMMER_TIME_OFF,
};

enum KeyInputMap {
  KEY00_INPUT06,
  KEY01_INPUT05,
  KEY02_INPUT04,
  KEY03_INPUT03,
  KEY04_INPUT01,
  KEY05_INPUT02,
  KEY06_INPUT09,
  KEY07_INPUT12,
  KEY08_INPUT11,
  KEY09_INPUT07,
  KEY10_INPUT08,
  KEY11_INPUT10,
};

union InputUnion {
  int state;
  struct {
    bool input01 : 1;
    bool input02 : 1;
    bool input03 : 1;
    bool input04 : 1;
    bool input05 : 1;
    bool input06 : 1;
    bool input07 : 1;
    bool input08 : 1;
    bool input09 : 1;
    bool input10 : 1;
    bool input11 : 1;
    bool input12 : 1;
  };
};

union OutputUnion {
  int state;
  struct {
    byte byte01;
    byte byte02;
  };
  struct {
    bool output01 : 1;
    bool output02 : 1;
    bool output03 : 1;
    bool output04 : 1;
    bool output05 : 1;
    bool output06 : 1;
    bool output07 : 1;
    bool output08 : 1;
    bool output09 : 1;
    bool output10 : 1;
    bool output11 : 1;
    bool output12 : 1;
    bool output13 : 1;
    bool output14 : 1;
    bool output15 : 1;
    bool output16 : 1;
  };
};

struct Timer {
  unsigned long time01;
  unsigned long time02;
  unsigned long time03;
  unsigned long time04;
  unsigned long time05;
  unsigned long time06;
  unsigned long time07;
  unsigned long time08;
  unsigned long time09;
  unsigned long time10;
};

struct Io {
  InputUnion inputs;
  OutputUnion outputs;
};

extern Io io;
extern int anemometerVelocity;
extern bool hydreonStatus;

void taskIo();
void sensorController();
void lightController();
void inputController();
void outputController();
void inputHandler(String name, bool state);

#endif
#endif