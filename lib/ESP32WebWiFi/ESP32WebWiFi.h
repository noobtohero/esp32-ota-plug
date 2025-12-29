#pragma once
#include "ESP32WebWiFiConfig.h"
#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <functional>

enum WebWiFiState {
  WEBWIFI_IDLE,
  WEBWIFI_CONNECTING,
  WEBWIFI_CONNECTED,
  WEBWIFI_AP_MODE,
  WEBWIFI_ERROR
};

class ESP32WebWiFi {
public:
  ESP32WebWiFi(AsyncWebServer &server, DNSServer &dns);

  // Configuration
  void setHostname(const char *hostname);
  void resetSettings(); // Clear creds & Force AP Mode

  // Callbacks
  void onConnect(std::function<void()> fn);
  void onAPMode(std::function<void()> fn);

  void begin(); // Start logic
  void loop();
  WebWiFiState getStatus();

  // Status helpers
  bool isAPMode();
  String getIP();
  String getSSID();

private:
  AsyncWebServer &_server;
  DNSServer &_dns;
  Preferences _prefs;

  WebWiFiState _state = WEBWIFI_IDLE;
  bool _apMode = false;
  unsigned long _apStartTime = 0;
  String _hostname = ESP32WEBWIFI_DEFAULT_HOSTNAME; // Default

  // Callbacks storage
  std::function<void()> _cbConnect = nullptr;
  std::function<void()> _cbAPMode = nullptr;

  void setupRoutes();
  void startAPMode();
  void setupMDNS();
};
