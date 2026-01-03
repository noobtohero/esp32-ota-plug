#include "ESP32WebManager.h"

ESP32WebManager::ESP32WebManager(AsyncWebServer &server, DNSServer &dns)
    : _server(server), wifi(server, dns), ota(server) {}

bool ESP32WebManager::begin(const char *version) {
  _version = version;

  // Start WiFi first
  wifi.begin();

  // Then OTA
  ota.begin(version);

  // Start server
  _server.begin();

  // Create background task for loop (only in AP mode)
  if (wifi.isAPMode()) {
    xTaskCreatePinnedToCore(loopTask,     // Task function
                            "WebMgrTask", // Name
                            4096,         // Stack size
                            this,         // Parameter
                            1,            // Priority
                            &_taskHandle, // Handle
                            0             // Core 0
    );
    return false; // AP Mode
  }

  return true; // WiFi Connected
}

// Static task function
void ESP32WebManager::loopTask(void *param) {
  ESP32WebManager *mgr = static_cast<ESP32WebManager *>(param);
  while (true) {
    mgr->wifi.loop();
    vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms interval
  }
}

void ESP32WebManager::loop() { wifi.loop(); }

// Configuration shortcuts
void ESP32WebManager::setHostname(const char *hostname) {
  wifi.setHostname(hostname);
}

void ESP32WebManager::setVersion(const char *version) { _version = version; }

void ESP32WebManager::setOTAAuth(const char *user, const char *pass) {
  ota.setAuth(user, pass);
}

// WiFi Callbacks
void ESP32WebManager::onWiFiConnect(std::function<void()> fn) {
  wifi.onConnect(fn);
}

void ESP32WebManager::onAPMode(std::function<void()> fn) { wifi.onAPMode(fn); }

// OTA Callbacks
void ESP32WebManager::onOTAStart(std::function<void()> fn) { ota.onStart(fn); }

void ESP32WebManager::onOTAEnd(std::function<void()> fn) { ota.onEnd(fn); }

void ESP32WebManager::onOTAProgress(std::function<void(int)> fn) {
  ota.onProgress(fn);
}

void ESP32WebManager::onOTAError(std::function<void(String)> fn) {
  ota.onError(fn);
}

// Status
bool ESP32WebManager::isAPMode() { return wifi.isAPMode(); }

String ESP32WebManager::getIP() { return wifi.getIP(); }

String ESP32WebManager::getVersion() { return _version; }

WiFiState ESP32WebManager::getWiFiState() { return wifi.getState(); }
