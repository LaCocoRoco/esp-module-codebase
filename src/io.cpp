#ifdef ESP_CU

#include "io.h"

#include <RunningMedian.h>
#include <driver/rtc_io.h>
#include <gpio.h>
#include <logger.h>
#include <system.h>

Io io;
Io ioBuffer;
Timer timer;
int anemometerVelocity;
int anemometerVelocityGpioBuffer;
bool hydreonStatus;
bool hydreonStatusGpioBuffer;
LightState disableLightState;
int dissableLightIteration;
int disableLightDimmerTime;
bool disableLight;

void taskIo() {
  inputController();
  outputController();
  lightController();
  sensorController();
}

void sensorController() {
  // hydreon sensor changed
  if (hydreonStatusGpioBuffer != hydreonStatus) {
    // hydreon sensor active
    if (hydreonStatus) {
      // motor left & right down
      io.outputs.output07 = true;
      io.outputs.output08 = false;
      io.outputs.output09 = false;
      io.outputs.output10 = true;
    }

    if (!hydreonStatus) {
      // motor left & right up
      io.outputs.output07 = false;
      io.outputs.output08 = true;
      io.outputs.output09 = true;
      io.outputs.output10 = false;
    }

    // set buffer hydreon status
    hydreonStatusGpioBuffer = hydreonStatus;
  }

  // anemometer velocity changed
  if (anemometerVelocityGpioBuffer != anemometerVelocity) {
    // anemometer vecolity exceeded
    if (anemometerVelocity > 40) {
      // motor left & right up
      io.outputs.output07 = false;
      io.outputs.output08 = true;
      io.outputs.output09 = true;
      io.outputs.output10 = false;
    }

    // set buffer anemometer velocity
    anemometerVelocityGpioBuffer = anemometerVelocity;
  }
}

void inputController() {
  // update inputs
  if (io.inputs.state ^ ioBuffer.inputs.state) {
    ioBuffer.inputs.state = io.inputs.state ^ ioBuffer.inputs.state;

    // motor left up event
    if (ioBuffer.inputs.input05) {
      // motor left up pressed
      if (io.inputs.input05) {
        // motor left up
        io.outputs.output07 = false;
        io.outputs.output08 = !io.outputs.output08;
      }
    }

    // motor down button event
    if (ioBuffer.inputs.input06) {
      // motor down button pressed
      if (io.inputs.input06) {
        // motor left down
        io.outputs.output07 = !io.outputs.output07;
        io.outputs.output08 = false;
      }
    }

    // motor right up event
    if (ioBuffer.inputs.input04) {
      // motor right up pressed
      if (io.inputs.input04) {
        // motor right up
        io.outputs.output09 = !io.outputs.output09;
        io.outputs.output10 = false;
      }
    }

    // motor right down event
    if (ioBuffer.inputs.input03) {
      // motor right down pressed
      if (io.inputs.input03) {
        // motor right down
        io.outputs.output09 = false;
        io.outputs.output10 = !io.outputs.output10;
      }
    }

    // motor left & right up event
    if (ioBuffer.inputs.input02) {
      // motor left & right up pressed
      if (io.inputs.input02) {
        // motor left & right up
        io.outputs.output07 = false;
        io.outputs.output09 = !io.outputs.output08;
        io.outputs.output08 = !io.outputs.output08;
        io.outputs.output10 = false;
      }
    }

    // motor left & right down event
    if (ioBuffer.inputs.input01) {
      // motor left & right down pressed
      if (io.inputs.input01) {
        // motor left & right down
        io.outputs.output08 = false;
        io.outputs.output09 = false;
        io.outputs.output10 = !io.outputs.output07;
        io.outputs.output07 = !io.outputs.output07;
      }
    }

    // light left event
    if (ioBuffer.inputs.input10) {
      // light left pressed
      if (io.inputs.input10) {
        // light left on
        io.outputs.output06 = true;
      }

      // light left released
      if (!io.inputs.input10) {
        // light left off
        io.outputs.output06 = false;
      }
    }

    // light right event
    if (ioBuffer.inputs.input11) {
      // light right pressed
      if (io.inputs.input11) {
        // light right on
        io.outputs.output11 = true;
      }

      // light right released
      if (!io.inputs.input11) {
        // light right off
        io.outputs.output11 = false;
      }
    }

    // heater left event
    if (ioBuffer.inputs.input07) {
      // heater left pressed
      if (io.inputs.input07) {
        // heater left on/off
        io.outputs.output01 = !io.outputs.output01;
      }
    }

    // heater right event
    if (ioBuffer.inputs.input12) {
      // heater right pressed
      if (io.inputs.input12) {
        // heater right on/off
        io.outputs.output16 = !io.outputs.output16;
      }
    }

    // home position event
    if (ioBuffer.inputs.input08) {
      // home position pressed
      if (io.inputs.input08) {
        // home position
        disableLight = true;
        io.outputs.output01 = false;
        io.outputs.output07 = false;
        io.outputs.output08 = true;
        io.outputs.output09 = true;
        io.outputs.output10 = false;
        io.outputs.output16 = false;
      }
    }

    // log inputs state
    for (int i = 0; i < INPUT_SIZE; i++) {
      bool value = ioBuffer.inputs.state >> i & 1;
      if (ioBuffer.inputs.state >> i & 1) {
        bool state = io.inputs.state >> i & 1;
        logger(INFO, "Input " + String(i + 1) + ":  " + String(state));
      }
    }

    // set buffer inputs state
    ioBuffer.inputs.state = io.inputs.state;
  }
}

void outputController() {
  // update outputs
  if (io.outputs.state ^ ioBuffer.outputs.state) {
    ioBuffer.outputs.state = io.outputs.state ^ ioBuffer.outputs.state;

    // set motor timers
    if (ioBuffer.outputs.output07) timer.time01 = io.outputs.output07 ? millis() : 0;
    if (ioBuffer.outputs.output08) timer.time02 = io.outputs.output08 ? millis() : 0;
    if (ioBuffer.outputs.output09) timer.time03 = io.outputs.output09 ? millis() : 0;
    if (ioBuffer.outputs.output10) timer.time04 = io.outputs.output10 ? millis() : 0;

    // set outputs
    digitalWrite(GPIO_STCP, LOW);
    shiftOut(GPIO_DS, GPIO_SHCP, MSBFIRST, io.outputs.byte02);
    shiftOut(GPIO_DS, GPIO_SHCP, MSBFIRST, io.outputs.byte01);
    digitalWrite(GPIO_STCP, HIGH);

    // log outputs state
    for (int i = 0; i < OUTPUT_SIZE; i++) {
      bool value = ioBuffer.outputs.state >> i & 1;
      if (ioBuffer.outputs.state >> i & 1) {
        bool state = io.outputs.state >> i & 1;
        logger(INFO, "Output " + String(i + 1) + ": " + String(state));
      }
    }

    // set buffer outputs state
    ioBuffer.outputs.state = io.outputs.state;
  }

  // handle motor timesrs
  if (timer.time01 && millis() > timer.time01 + MOTOR_MAX_ON_TIME) io.outputs.output07 = false;
  if (timer.time02 && millis() > timer.time02 + MOTOR_MAX_ON_TIME) io.outputs.output08 = false;
  if (timer.time03 && millis() > timer.time03 + MOTOR_MAX_ON_TIME) io.outputs.output09 = false;
  if (timer.time04 && millis() > timer.time04 + MOTOR_MAX_ON_TIME) io.outputs.output10 = false;
}

void lightController() {
  switch (disableLightState) {
    case LIGHT_IDLE: {
      if (disableLight) {
        dissableLightIteration = 0;
        disableLightState = LIGHT_DIMMER_ENABLE;
      }

      break;
    }

    case LIGHT_DIMMER_ENABLE: {
      dissableLightIteration++;
      io.outputs.output06 = true;
      io.outputs.output11 = true;
      disableLightDimmerTime = millis();
      disableLightState = LIGHT_DIMMER_TIME_ON;
      break;
    }

    case LIGHT_DIMMER_TIME_ON: {
      if (millis() > disableLightDimmerTime + LIGHT_DIMMER_TIME) {
        disableLightState = LIGHT_DIMMER_DISABLE;
      };

      break;
    }

    case LIGHT_DIMMER_DISABLE: {
      io.outputs.output06 = false;
      io.outputs.output11 = false;
      disableLightDimmerTime = millis();
      disableLightState = LIGHT_DIMMER_TIME_OFF;
      break;
    }

    case LIGHT_DIMMER_TIME_OFF: {
      if (millis() > disableLightDimmerTime + LIGHT_DIMMER_TIME) {
        disableLightState = LIGHT_DIMMER_ENABLE;
      };

      if (dissableLightIteration > 1) {
        disableLight = false;
        disableLightState = LIGHT_IDLE;
      };

      break;
    }
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