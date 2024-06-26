#ifndef WEB_H
#define WEB_H

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer webServer;
extern DNSServer dnsServer;

static const String WEB_ELEMENT_DISPLAY_NONE = "style=\"display:none\"";
static const String WEB_ELEMENT_DISPLAY_BLOCK = "style=\"display:block\"";

void requestHandlerNotFound(AsyncWebServerRequest *request);
void requestHandlerIndexHtml(AsyncWebServerRequest *request);
void requestHandlerIndexCss(AsyncWebServerRequest *request);
void requestHandlerIndexJs(AsyncWebServerRequest *request);
void requestHandlerFavicon(AsyncWebServerRequest *request);

#endif