#pragma once
#include <ESPAsyncWebServer.h>

class ESP32WebOTA {
public:
  ESP32WebOTA(AsyncWebServer& server);
  void begin();
  static void boot();
private:
  AsyncWebServer& _server;
};
