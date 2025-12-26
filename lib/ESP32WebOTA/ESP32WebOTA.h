#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class ESP32WebOTA {
public:
  ESP32WebOTA(AsyncWebServer& server);
  void begin();
  static void boot();
  String getVersion();
  void setVersion(const String& v);
private:
  AsyncWebServer& _server;
};
