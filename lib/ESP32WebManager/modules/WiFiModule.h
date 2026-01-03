#pragma once
#include "../ESP32WebManagerConfig.h"
#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <functional>


enum WiFiState {
  WIFI_STATE_IDLE,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_AP_MODE,
  WIFI_STATE_ERROR
};

class WiFiModule {
public:
  WiFiModule(AsyncWebServer &server, DNSServer &dns);

  void begin();
  void loop();

  // Configuration
  void setHostname(const char *hostname);
  void resetSettings();

  // Status
  WiFiState getState();
  bool isAPMode();
  String getIP();
  String getSSID();
  String getHostname();

  // Callbacks
  void onConnect(std::function<void()> fn);
  void onAPMode(std::function<void()> fn);

private:
  AsyncWebServer &_server;
  DNSServer &_dns;
  Preferences _prefs;

  WiFiState _state = WIFI_STATE_IDLE;
  bool _apMode = false;
  unsigned long _apStartTime = 0;
  String _hostname = WEBMGR_DEFAULT_HOSTNAME;

  std::function<void()> _cbConnect = nullptr;
  std::function<void()> _cbAPMode = nullptr;

  void startAPMode();
  void setupRoutes();
  void setupMDNS();
};
