#ifdef ESP_CU

#include "io.h"

#include <RunningMedian.h>
#include <driver/rtc_io.h>
#include <gpio.h>
#include <logger.h>
#include <system.h>

Io io;
Io ioBuffer;
bool hydreonStatus;
int anemometerVelocity;

void taskIo() {
  /***********************************************/
  /* custom process for handling inputs, outputs */
  /* anemometer and hydreon variables.           */
  /***********************************************/
  ioExmaple();
}

void ioExmaple() {
  // inputs update event
  if (io.inputs.state ^ ioBuffer.inputs.state) {
    ioBuffer.inputs.state = io.inputs.state ^ ioBuffer.inputs.state;

    // input01 event
    if (ioBuffer.inputs.input01) {
      // input01 pressed
      if (io.inputs.input01) {
        // set output01
        io.outputs.output01 = true;
      }

      // input01 released
      if (!io.inputs.input01) {
        // reset output01
        io.outputs.output01 = false;
      }
    }

    // set buffer inputs state
    ioBuffer.inputs.state = io.inputs.state;
  }

  // output button event
  if (io.outputs.state ^ ioBuffer.outputs.state) {
    ioBuffer.outputs.state = io.outputs.state ^ ioBuffer.outputs.state;

    // transfer outputs to shift register
    digitalWrite(GPIO_STCP, LOW);
    shiftOut(GPIO_DS, GPIO_SHCP, MSBFIRST, io.outputs.byte02);
    shiftOut(GPIO_DS, GPIO_SHCP, MSBFIRST, io.outputs.byte01);
    digitalWrite(GPIO_STCP, HIGH);

    // set buffer outputs state
    ioBuffer.outputs.state = io.outputs.state;
  }
}

void ioHandler(String name, bool state) {
  if (name == "input01") io.inputs.input01 = state;
  if (name == "input02") io.inputs.input02 = state;
  if (name == "input03") io.inputs.input03 = state;
  if (name == "input04") io.inputs.input04 = state;
  if (name == "input05") io.inputs.input05 = state;
  if (name == "input06") io.inputs.input06 = state;
  if (name == "input07") io.inputs.input07 = state;
  if (name == "input08") io.inputs.input08 = state;
  if (name == "input09") io.inputs.input09 = state;
  if (name == "input10") io.inputs.input10 = state;
  if (name == "input11") io.inputs.input11 = state;
  if (name == "input12") io.inputs.input12 = state;
}

#endif