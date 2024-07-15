#ifdef ESP_CU

#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>
#include <driver/adc.h>

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

struct Io {
  InputUnion inputs;
  OutputUnion outputs;
};

extern Io io;
extern bool hydreonStatus;
extern int anemometerVelocity;

void taskIo();
void ioHandler(String name, bool state);

#endif
#endif