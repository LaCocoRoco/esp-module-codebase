#include "socket.h"

#include <cmd.h>
#include <gpio.h>
#include <logger.h>
#include <network.h>
#include <serial.h>
#include <system.h>
#ifdef ESP_CU
#include <io.h>
#endif
#ifdef ESP_OU
#include <touch.h>
#endif
#ifdef ESP_SU
#include <anemometer.h>
#include <hydreon.h>
#endif

AsyncWebSocket webSocket(SOCKET_URL);
SocketState socketState;
unsigned long socketFastTime;
unsigned long socketSlowTime;
#ifdef ESP_CU
Io ioSocketBuffer;
int anemometerVelocitySocketBuffer;
bool hydreonStatusSocketBuffer;
#endif
#ifdef ESP_OU
AT42QT2120 at42qt2120SocketBuffer;
#endif
#ifdef ESP_SU
int anemometerVelocitySocketBuffer;
bool hydreonStatusSocketBuffer;
int intensitySocketBuffer;
#endif

void setupSocket() {
  /* empty */
}

void taskSocket() {
  socketController();
}

void socketController() {
  switch (socketState) {
    case SOCKET_IDLE: {
      if (mode == MODE_WEB) {
        if (wifiState == WIFI_IDLE) {
          if (millis() > socketFastTime + SOCKET_TRANSMIT_INTERVAL_FAST) {
            socketState = SOCKET_UPDATE_INTERVALL_FAST;
          }

          if (millis() > socketSlowTime + SOCKET_TRANSMIT_INTERVAL_SLOW) {
            socketState = SOCKET_UPDATE_INTERVALL_SLOW;
          }
        }

        break;
      }

      case SOCKET_UPDATE_INTERVALL_FAST: {
        socketTransmitUpdate();
        socketFastTime = millis();
        socketState = SOCKET_IDLE;
        break;
      }

      case SOCKET_UPDATE_INTERVALL_SLOW: {
        socketTransmitUpdate(true);
        socketSlowTime = millis();
        socketState = SOCKET_IDLE;
        break;
      }
    }
  }
}

void socketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, byte *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: socketEventConnect(client); break;
    case WS_EVT_DATA: socketEventData(client, arg, data, len); break;
  }
}

void socketEventConnect(AsyncWebSocketClient *client) {
  socketTransmitWifi(client->id());
}

void socketEventData(AsyncWebSocketClient *client, void *arg, byte *data, size_t len) {
  const AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    DynamicJsonDocument document(1024);
    deserializeJson(document, (char *)data);
    socketReceive(document.as<JsonObject>());
  }
}

void socketReceive(const JsonObject document) {
  const String id = document["id"];
  const JsonObject data = document["data"].as<JsonObject>();

  if (id == "button") socketReceiveButton(data);
  if (id == "wifi") socketReceiveWifi(data);
  if (id == "terminal") socketReceiveTerminal(data);
#ifdef ESP_OU
  if (id == "touch") socketReceiveTouch(data);
#endif
#ifdef ESP_SU
  if (id == "anemometer") socketReceiveAnemometer(data);
#endif
}

void socketReceiveButton(const JsonObject data) {
  const String name = data["name"];
  const bool state = data["state"];
  cmdHandler(name);
#ifdef ESP_CU
  inputHandler(name, state);
#endif
}

void socketReceiveWifi(const JsonObject data) {
  wifiSSID = (const char *)data["ssid"];
  wifiPassword = (const char *)data["password"];
  wifiInitializeUpdate = true;
}

void socketReceiveTerminal(const JsonObject data) {
  const String text = data["text"];
  serialTransmitTerminal(">> " + text);
  cmdHandler(text);
}

#ifdef ESP_OU
void socketReceiveTouch(const JsonObject data) {
  const String id = data["id"];
  const String setting = data["setting"];
  const byte value = data["value"];
  const byte key = data["key"];
  if (id == "common") touchWriteCommonRegister(setting, value);
  if (id == "key") touchWriteKeyRegister(setting, value, key);
}
#endif

#ifdef ESP_SU
void socketReceiveAnemometer(const JsonObject data) {
  anemometerCalibrate = true;
  anemometerCalibratedVelocity = (int)data["velocity"];
}
#endif

void socketTransmit(const int id, const String data) {
  if (wifiClientConnected()) {
    if (id && webSocket.availableForWrite(id)) {
      webSocket.text(id, data);
    } else if (webSocket.availableForWriteAll()) {
      webSocket.textAll(data);
    }
  }
}

void socketTransmitTerminal(const String text, const int id) {
  DynamicJsonDocument document(5120);
  document["id"] = "terminal";
  const JsonObject data = document.createNestedObject("data");
  data["text"] = text;

  String buffer = String();
  serializeJson(document, buffer);
  socketTransmit(id, buffer);
}

void socketTransmitWifi(const int id) {
  DynamicJsonDocument document(5120);
  document["id"] = "wifi";
  const JsonObject data = document.createNestedObject("data");
  const JsonArray network = data.createNestedArray("network");

  for (int i = 0; i < WiFi.scanComplete(); i++) {
    const String ssid = WiFi.SSID(i);
    const String rssi = String(WiFi.RSSI(i));
    const JsonObject wifi = network.createNestedObject();
    wifi["SSID"] = ssid;
    wifi["RSSI"] = rssi;
  }

  String buffer = String();
  serializeJson(document, buffer);
  socketTransmit(id, buffer);
}

void socketTransmitUpdate(bool update) {
  /********************************************************/
  /* minimize socket workload by evaluate data difference */
  /********************************************************/
  DynamicJsonDocument document(5120);
  document["id"] = "update";

  const JsonArray data = document.createNestedArray("data");
  const JsonObject index0 = data.createNestedObject();
  const JsonObject index1 = data.createNestedObject();
  const JsonObject index2 = data.createNestedObject();
  const JsonObject index3 = data.createNestedObject();
  const JsonObject data0 = index0.createNestedObject("data");
  const JsonObject data1 = index1.createNestedObject("data");
  const JsonObject data2 = index2.createNestedObject("data");
  const JsonObject data3 = index3.createNestedObject("data");

  index0["id"] = "ping";
  index1["id"] = "terminal";
#ifdef ESP_CU
  index2["id"] = "sensor";
  index3["id"] = "output";
#endif
#ifdef ESP_OU
  index2["id"] = "touch";
#endif
#ifdef ESP_SU
  index2["id"] = "sensor";
#endif
  data0["isAlive"] = true;

  if (socketEvaluateTerminalData(&data1)) update = true;
#ifdef ESP_CU
  if (socketEvaluateSensorData(&data2)) update = true;
  if (socketEvaluateInputData(&data3)) update = true;
#endif
#ifdef ESP_OU
  if (socketEvaluateTouchData(&data2)) update = true;
#endif
#ifdef ESP_SU
  if (socketEvaluateSensorData(&data2)) update = true;
#endif

  if (update) {
    String buffer = String();
    serializeJson(document, buffer);
    socketTransmit(0, buffer);
    socketSlowTime = millis();
  }
}

bool socketEvaluateTerminalData(const JsonObject *data) {
  int memory = data->memoryUsage();

  if (!loggerBufferIsEmpty()) {
    (*data)["text"] = loggerGetBuffer();
    loggerClearBuffer();
  }

  return memory != data->memoryUsage() ? true : false;
}

#ifdef ESP_CU
bool socketEvaluateInputData(const JsonObject *data) {
  int memory = data->memoryUsage();

  if (io.outputs.state ^ ioSocketBuffer.outputs.state) {
    ioSocketBuffer.outputs.state = io.outputs.state;
    (*data)["output01"] = (bool)io.outputs.output01;
    (*data)["output02"] = (bool)io.outputs.output02;
    (*data)["output03"] = (bool)io.outputs.output03;
    (*data)["output04"] = (bool)io.outputs.output04;
    (*data)["output05"] = (bool)io.outputs.output05;
    (*data)["output06"] = (bool)io.outputs.output06;
    (*data)["output07"] = (bool)io.outputs.output07;
    (*data)["output08"] = (bool)io.outputs.output08;
    (*data)["output09"] = (bool)io.outputs.output09;
    (*data)["output10"] = (bool)io.outputs.output10;
    (*data)["output11"] = (bool)io.outputs.output11;
    (*data)["output12"] = (bool)io.outputs.output12;
    (*data)["output13"] = (bool)io.outputs.output13;
    (*data)["output14"] = (bool)io.outputs.output14;
    (*data)["output15"] = (bool)io.outputs.output15;
    (*data)["output16"] = (bool)io.outputs.output16;
  }

  return memory != data->memoryUsage() ? true : false;
}
#endif

#ifdef ESP_CU
bool socketEvaluateSensorData(const JsonObject *data) {
  int memory = data->memoryUsage();

  if (anemometerVelocitySocketBuffer != anemometerVelocity) {
    (*data)["anemometerVelocity"] = anemometerVelocity;
    anemometerVelocitySocketBuffer = anemometerVelocity;
  }

  if (hydreonStatusSocketBuffer != hydreonStatus) {
    (*data)["hydreonStatus"] = hydreonStatus ? "On" : "Off";
    hydreonStatusSocketBuffer = hydreonStatus;
  }

  return memory != data->memoryUsage() ? true : false;
}
#endif

#ifdef ESP_SU
bool socketEvaluateSensorData(const JsonObject *data) {
  int memory = data->memoryUsage();

  if (anemometerVelocitySocketBuffer != anemometerVelocity) {
    (*data)["anemometerVelocity"] = anemometerVelocity;
    anemometerVelocitySocketBuffer = anemometerVelocity;
  }

  if (hydreonStatusSocketBuffer != hydreonStatus) {
    (*data)["hydreonStatus"] = hydreonStatus ? "On" : "Off";
    hydreonStatusSocketBuffer = hydreonStatus;
  }

  return memory != data->memoryUsage() ? true : false;
}
#endif

#ifdef ESP_OU
bool socketEvaluateTouchData(const JsonObject *data) {
  const JsonObject common = data->createNestedObject("common");
  const JsonArray keys = data->createNestedArray("key");

  touchReadRegisterData();

  if (at42qt2120SocketBuffer.chipID.value != at42qt2120.chipID.value) {
    common["chipId"] = String(at42qt2120.chipID.value);
  }

  if (at42qt2120SocketBuffer.firmwareVersion.value != at42qt2120.firmwareVersion.value) {
    common["firmwareMinor"] = String(at42qt2120.firmwareVersion.minorVersion);
    common["firmwareMajor"] = String(at42qt2120.firmwareVersion.majorVersion);
  }

  if (at42qt2120SocketBuffer.detectionStatus.value != at42qt2120.detectionStatus.value) {
    common["detectOverflow"] = String(at42qt2120.detectionStatus.overflow);
    common["detectSdet"] = String(at42qt2120.detectionStatus.sdet);
    common["detectTdet"] = String(at42qt2120.detectionStatus.tdet);
  }

  if (at42qt2120SocketBuffer.sliderStatus.value != at42qt2120.sliderStatus.value) {
    common["sliderStatus"] = String(at42qt2120.sliderStatus.value);
  }

  if (at42qt2120SocketBuffer.calibrate.value != at42qt2120.calibrate.value) {
    common["calibrate"] = String(at42qt2120.calibrate.value);
  }

  if (at42qt2120SocketBuffer.reset.value != at42qt2120.reset.value) {
    common["reset"] = String(at42qt2120.reset.value);
  }

  if (at42qt2120SocketBuffer.lowPowerMode.value != at42qt2120.lowPowerMode.value) {
    common["lowPowerMode"] = String(at42qt2120.lowPowerMode.value);
  }

  if (at42qt2120SocketBuffer.towardsTouchDrift.value != at42qt2120.towardsTouchDrift.value) {
    common["towardTouchDrift"] = String(at42qt2120.towardsTouchDrift.value);
  }

  if (at42qt2120SocketBuffer.awayFromTouchDrift.value != at42qt2120.awayFromTouchDrift.value) {
    common["awayTouchDrift"] = String(at42qt2120.awayFromTouchDrift.value);
  }

  if (at42qt2120SocketBuffer.detectionIntegration.value != at42qt2120.detectionIntegration.value) {
    common["detectionIntegration"] = String(at42qt2120.detectionIntegration.value);
  }

  if (at42qt2120SocketBuffer.touchRecalDelay.value != at42qt2120.touchRecalDelay.value) {
    common["touchRecalDelay"] = String(at42qt2120.touchRecalDelay.value);
  }

  if (at42qt2120SocketBuffer.driftHoldTime.value != at42qt2120.driftHoldTime.value) {
    common["driftHoldTime"] = String(at42qt2120.driftHoldTime.value);
  }

  if (at42qt2120SocketBuffer.sliderOptions.value != at42qt2120.sliderOptions.value) {
    common["sliderOptEnable"] = String(at42qt2120.sliderOptions.en);
    common["sliderOptWheel"] = String(at42qt2120.sliderOptions.wheel);
  }

  if (at42qt2120SocketBuffer.chargeTime.value != at42qt2120.chargeTime.value) {
    common["chargeTime"] = String(at42qt2120.chargeTime.time);
  }

  for (int i = 0; i < AT42QT2120_KEYS; i++) {
    const JsonObject key = keys.createNestedObject();

    if (i < 8) {
      if (((at42qt2120SocketBuffer.keyStatus.first >> i) & 1) != ((at42qt2120.keyStatus.first >> i) & 1)) {
        key["keyStatus"] = String((at42qt2120.keyStatus.first >> i) & 1);
      }
    } else {
      if (((at42qt2120SocketBuffer.keyStatus.second >> i - 8) & 1) != ((at42qt2120.keyStatus.second >> i - 8) & 1)) {
        key["keyStatus"] = String((at42qt2120.keyStatus.second >> i - 8) & 1);
      }
    }

    if (at42qt2120SocketBuffer.detectThreshold[i].value != at42qt2120.detectThreshold[i].value) {
      key["keyThreshold"] = String(at42qt2120.detectThreshold[i].value);
    }

    if (at42qt2120SocketBuffer.keyControl[i].value != at42qt2120.keyControl[i].value) {
      key["keyControlEn"] = String(at42qt2120.keyControl[i].en);
      key["keyControlAks"] = String(at42qt2120.keyControl[i].aks);
      key["keyControlGpo"] = String(at42qt2120.keyControl[i].gpo);
      key["keyControlGuard"] = String(at42qt2120.keyControl[i].guard);
    }

    if (at42qt2120SocketBuffer.keyPulseScale[i].value != at42qt2120.keyPulseScale[i].value) {
      key["keyPulse"] = String(at42qt2120.keyPulseScale[i].pulse);
      key["keyScale"] = String(at42qt2120.keyPulseScale[i].scale);
    }

    if (at42qt2120SocketBuffer.referenceData[i].value != at42qt2120.referenceData[i].value) {
      key["keyReference"] = String(at42qt2120.referenceData[i].value);
    }

    if (at42qt2120SocketBuffer.keySignal[i].value > at42qt2120.keySignal[i].value + 2 ||
        at42qt2120SocketBuffer.keySignal[i].value < at42qt2120.keySignal[i].value - 2) {
      key["keySignal"] = String(at42qt2120.keySignal[i].value);
    }
  }

  if (data->memoryUsage() != 224) {
    at42qt2120SocketBuffer = at42qt2120;
    return true;
  } else {
    return false;
  }
}
#endif