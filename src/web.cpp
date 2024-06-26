#include "web.h"

#include <SPIFFS.h>
#include <config.h>
#include <gpio.h>
#include <network.h>
#include <socket.h>
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

AsyncWebServer webServer(80);
DNSServer dnsServer;

void requestHandlerIndexHtml(AsyncWebServerRequest *request) {
  // template processor html
  request->send(SPIFFS, "/index.html", String(), false, [](const String &var) {
    if (var == "module") return MODULE_NAME;
    if (var == "hwv") return MODULE_HWV;
    if (var == "swv") return MODULE_SWV;
#ifdef ESP_CU
    if (var == "touchModule") return WEB_ELEMENT_DISPLAY_NONE;
    if (var == "inputModule") return WEB_ELEMENT_DISPLAY_BLOCK;
    if (var == "outputModule") return WEB_ELEMENT_DISPLAY_BLOCK;
    if (var == "anemometerVelocity") return String(anemometerVelocity);
    if (var == "hydreonStatus") return String(hydreonStatus);
#endif
#ifdef ESP_SU
    if (var == "touchModule") return WEB_ELEMENT_DISPLAY_NONE;
    if (var == "inputModule") return WEB_ELEMENT_DISPLAY_NONE;
    if (var == "outputModule") return WEB_WEB_ELEMENT_DISPLAY_NONE;
    if (var == "battery") return String(batteryVoltage);
    if (var == "anemometerVelocity") return String(anemometerVelocity);
    if (var == "hydreonStatus") return String(hydreonStatus);
    if (var == "hydreonIntensity") return String(hydreonIntensity);
#endif
#ifdef ESP_OU
    if (var == "touchModule") return WEB_ELEMENT_DISPLAY_BLOCK;
    if (var == "inputModule") return WEB_ELEMENT_DISPLAY_NONE;
    if (var == "outputModule") return WEB_ELEMENT_DISPLAY_NONE;
    if (var == "battery") return String(batteryVoltage);
    if (var == "chipId") return String(at42qt2120.chipID.value);
    if (var == "firmwareMajor") return String(at42qt2120.firmwareVersion.majorVersion);
    if (var == "firmwareMinor") return String(at42qt2120.firmwareVersion.minorVersion);
    if (var == "detectCalibrate") return String(at42qt2120.detectionStatus.calibrate);
    if (var == "detectOverflow") return String(at42qt2120.detectionStatus.overflow);
    if (var == "detectSdet") return String(at42qt2120.detectionStatus.sdet);
    if (var == "detectTdet") return String(at42qt2120.detectionStatus.tdet);
    if (var == "sliderStatus") return String(at42qt2120.sliderStatus.value);
    if (var == "calibrate") return String(at42qt2120.calibrate.value);
    if (var == "reset") return String(at42qt2120.reset.value);
    if (var == "lowPowerMode") return String(at42qt2120.lowPowerMode.value);
    if (var == "towardTouchDrift") return String(at42qt2120.towardsTouchDrift.value);
    if (var == "awayTouchDrift") return String(at42qt2120.awayFromTouchDrift.value);
    if (var == "detectionIntegration") return String(at42qt2120.detectionIntegration.value);
    if (var == "touchRecalDelay") return String(at42qt2120.touchRecalDelay.value);
    if (var == "driftHoldTime") return String(at42qt2120.driftHoldTime.value);
    if (var == "sliderOptEnable") return String(at42qt2120.sliderOptions.en);
    if (var == "sliderOptWheel") return String(at42qt2120.sliderOptions.wheel);
    if (var == "chargeTime") return String(at42qt2120.chargeTime.time);

    for (int i = 0; i < AT42QT2120_KEYS; i++) {
      String key = String(i);
      if (i <= 7 && var == "keyStatus" + key) return String((at42qt2120.keyStatus.first >> i) & 1);
      if (i >= 8 && var == "keyStatus" + key) return String((at42qt2120.keyStatus.second >> i - 8) & 1);
      if (var == "keyThreshold" + key) return String(at42qt2120.detectThreshold[i].value);
      if (var == "keyControlEn" + key) return String(at42qt2120.keyControl[i].en);
      if (var == "keyControlAks" + key) return String(at42qt2120.keyControl[i].aks);
      if (var == "keyControlGpo" + key) return String(at42qt2120.keyControl[i].gpo);
      if (var == "keyControlGuard" + key) return String(at42qt2120.keyControl[i].guard);
      if (var == "keyPulse" + key) return String(at42qt2120.keyPulseScale[i].pulse);
      if (var == "keyScale" + key) return String(at42qt2120.keyPulseScale[i].scale);
      if (var == "keySignal" + key) return String(at42qt2120.keySignal[i].value);
      if (var == "keyReference" + key) return String(at42qt2120.referenceData[i].value);
    }
#endif
    return String();
  });
}

void requestHandlerIndexJs(AsyncWebServerRequest *request) {
  // template processor javsacript
  request->send(SPIFFS, "/index.js", String(), false, [](const String &var) {
    if (var == "address") return wifiAddress.toString();
    if (var == "socket") return SOCKET_URL;
    if (var == "ping") return String(SOCKET_TRANSMIT_INTERVAL_SLOW);
    return String();
  });
}

void requestHandlerIndexCss(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.css");
}

void requestHandlerFavicon(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/favicon.ico");
}

void requestHandlerNotFound(AsyncWebServerRequest *request) {
  requestHandlerIndexHtml(request);
}
