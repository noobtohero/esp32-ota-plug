#pragma once
#include "../ESP32WebManagerConfig.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <functional>


class OTAModule {
public:
  OTAModule(AsyncWebServer &server);

  void begin(const char *version);
  String getVersion();

  // Callbacks
  void onStart(std::function<void()> fn);
  void onEnd(std::function<void()> fn);
  void onProgress(std::function<void(int)> fn);
  void onError(std::function<void(String)> fn);

  // Auth
  void setAuth(const char *user, const char *pass);

private:
  AsyncWebServer &_server;
  String _version;
  String _authUser = WEBMGR_OTA_USER;
  String _authPass = WEBMGR_OTA_PASS;

  int _otaProgress = 0;

  std::function<void()> _cbStart = nullptr;
  std::function<void()> _cbEnd = nullptr;
  std::function<void(int)> _cbProgress = nullptr;
  std::function<void(String)> _cbError = nullptr;

  void setupRoutes();
  bool checkAuth(AsyncWebServerRequest *req);
};
