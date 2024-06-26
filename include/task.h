#ifndef TASK_H
#define TASK_H

#include <Arduino.h>

static const int TASK_TIME = 4;

void task();
void setupTask();
void taskLoop();

#endif