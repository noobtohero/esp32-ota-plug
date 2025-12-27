#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class ESP32WebOTA {
public:
  ESP32WebOTA(AsyncWebServer &server);
  void begin(const char *currentVersion);
  String getVersion();
  void setVersion(const String &v);

private:
  AsyncWebServer &_server;
  void boot();
};
