#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <functional>

class ESP32WebOTA {
public:
  ESP32WebOTA(AsyncWebServer &server);
  void begin(const char *currentVersion);
  String getVersion();
  void setVersion(const String &v);

  // Callbacks
  void onStart(std::function<void()> fn);
  void onEnd(std::function<void()> fn);
  void onProgress(std::function<void(int)> fn);
  void onError(std::function<void(String)> fn);

private:
  AsyncWebServer &_server;
  void boot();

  std::function<void()> _cbStart = nullptr;
  std::function<void()> _cbEnd = nullptr;
  std::function<void(int)> _cbProgress = nullptr;
  std::function<void(String)> _cbError = nullptr;
};
