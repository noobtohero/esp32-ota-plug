#pragma once
#include "ESP32WebManagerConfig.h"
#include "modules/OTAModule.h"
#include "modules/WiFiModule.h"

class ESP32WebManager {
public:
  ESP32WebManager(AsyncWebServer &server, DNSServer &dns);

  // Initialize everything - returns true if WiFi connected, false if AP mode
  bool begin(const char *version = "1.0.0");
  void loop();

  // Configuration shortcuts
  void setHostname(const char *hostname);
  void setVersion(const char *version);
  void setOTAAuth(const char *user, const char *pass);

  // WiFi Callbacks
  void onWiFiConnect(std::function<void()> fn);
  void onAPMode(std::function<void()> fn);

  // OTA Callbacks
  void onOTAStart(std::function<void()> fn);
  void onOTAEnd(std::function<void()> fn);
  void onOTAProgress(std::function<void(int)> fn);
  void onOTAError(std::function<void(String)> fn);

  // Status
  bool isAPMode();
  String getIP();
  String getVersion();
  WiFiState getWiFiState();

  // Direct module access (advanced usage)
  WiFiModule wifi;
  OTAModule ota;

private:
  AsyncWebServer &_server;
  String _version = "1.0.0";
  TaskHandle_t _taskHandle = NULL;

  static void loopTask(void *param);
};
