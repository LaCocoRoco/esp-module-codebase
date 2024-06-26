#include "logger.h"

#include <serial.h>
#include <socket.h>

LogLevel logLevel;
String logBuffer;

void logger(const LogLevel level, const String text, bool async) {
  if (logLevel >= level) {
    if (async) {
      logBuffer += logBuffer.length() > 0 ? "\n<< " + text : "<< " + text;
      serialTransmitTerminal("<< " + text);
    } else {
      socketTransmitTerminal("<< " + text);
      serialTransmitTerminal("<< " + text);
    }
  }
}

void loggerClearBuffer() {
  logBuffer = String();
}

String loggerGetBuffer() {
  return logBuffer;
}

bool loggerBufferIsEmpty() {
  return logBuffer.isEmpty();
}

void loggerAsyncHandler() {
  if (!logBuffer.isEmpty()) {
    socketTransmitTerminal(logBuffer);
    logBuffer = String();
  }
}
