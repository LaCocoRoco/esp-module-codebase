#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum LogLevel {
  TRACE,
  DEBUG,
  INFO,
  WARNING,
  ERROR
};

extern LogLevel logLevel;

void logger(const LogLevel level, const String text, bool async = true);
void loggerClearBuffer();
bool loggerBufferIsEmpty();
void loggerAsyncHandler();
String loggerGetBuffer();

#endif