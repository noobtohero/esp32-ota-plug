#include "WiFiModule.h"
#include <SPIFFS.h>

WiFiModule::WiFiModule(AsyncWebServer &server, DNSServer &dns)
    : _server(server), _dns(dns) {}

void WiFiModule::begin() {
  _prefs.begin(WEBMGR_PREFS_KEY, false);
  String ssid = _prefs.getString("ssid", "");
  String pass = _prefs.getString("pass", "");
  _prefs.end();

  _state = WIFI_STATE_CONNECTING;

  if (ssid.length() > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < WEBMGR_CONN_TIMEOUT_MS) {
      delay(500);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    _state = WIFI_STATE_CONNECTED;
    _apMode = false;
    setupMDNS();
    if (_cbConnect)
      _cbConnect();
  } else {
    _state = WIFI_STATE_AP_MODE;
    startAPMode();
  }

  setupRoutes();
}

void WiFiModule::startAPMode() {
  _apMode = true;
  _apStartTime = millis();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(WEBMGR_AP_NAME, WEBMGR_AP_PASSWORD);
  setupMDNS();
  if (_cbAPMode)
    _cbAPMode();

  _dns.start(53, "*", WiFi.softAPIP());
}

void WiFiModule::setupRoutes() {
  // CORS & Cache Headers
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Cache-Control",
                                       "no-cache, no-store, must-revalidate");

  // API: Start Scan (Async)
  _server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *req) {
    WiFi.scanNetworks(true);
    req->send(200, "application/json", "{\"status\":\"scanning\"}");
  });

  // API: Get Scan Result
  _server.on("/api/wifi/scanresult", HTTP_GET, [](AsyncWebServerRequest *req) {
    int n = WiFi.scanComplete();

    if (n == WIFI_SCAN_RUNNING) {
      req->send(200, "application/json", "{\"status\":\"scanning\"}");
      return;
    }
    if (n == WIFI_SCAN_FAILED || n < 0) {
      req->send(200, "application/json", "{\"status\":\"failed\"}");
      WiFi.scanDelete();
      return;
    }

    String json = "[";
    for (int i = 0; i < n; ++i) {
      if (i)
        json += ",";
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json +=
          "\"auth\":" +
          String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true") +
          "}";
    }
    json += "]";
    WiFi.scanDelete();
    req->send(200, "application/json", json);
  });

  // API: Connect
  _server.on("/api/wifi/connect", HTTP_POST,
             [this](AsyncWebServerRequest *req) {
               if (req->hasParam("ssid", true) && req->hasParam("pass", true)) {
                 String ssid = req->getParam("ssid", true)->value();
                 String pass = req->getParam("pass", true)->value();

                 _prefs.begin(WEBMGR_PREFS_KEY, false);
                 _prefs.putString("ssid", ssid);
                 _prefs.putString("pass", pass);
                 _prefs.end();

                 req->send(200, "text/plain", "Saved. Rebooting...");
                 delay(1000);
                 ESP.restart();
               } else {
                 req->send(400, "text/plain", "Missing params");
               }
             });

  // API: Reset
  _server.on("/api/wifi/reset", HTTP_POST, [this](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "Resetting settings...");
    delay(500);
    resetSettings();
  });

  // Portal UI
  if (_apMode) {
    _server.serveStatic("/", SPIFFS, "/wifi/").setDefaultFile("index.html");
    _server.onNotFound([](AsyncWebServerRequest *req) { req->redirect("/"); });
  } else {
    _server.serveStatic("/wifi", SPIFFS, "/wifi/").setDefaultFile("index.html");
  }
}

void WiFiModule::loop() {
  if (_apMode) {
    _dns.processNextRequest();
    if (WEBMGR_AP_TIMEOUT_MS > 0 &&
        millis() - _apStartTime > WEBMGR_AP_TIMEOUT_MS) {
      ESP.restart();
    }
  }
}

void WiFiModule::setupMDNS() {
  if (MDNS.begin(_hostname.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }
}

void WiFiModule::resetSettings() {
  _prefs.begin(WEBMGR_PREFS_KEY, false);
  _prefs.clear();
  _prefs.end();
  WiFi.disconnect(true, true);
  _state = WIFI_STATE_AP_MODE;
  startAPMode();
}

// Getters & Setters
void WiFiModule::setHostname(const char *hostname) { _hostname = hostname; }
void WiFiModule::onConnect(std::function<void()> fn) { _cbConnect = fn; }
void WiFiModule::onAPMode(std::function<void()> fn) { _cbAPMode = fn; }

WiFiState WiFiModule::getState() { return _state; }
bool WiFiModule::isAPMode() { return _apMode; }
String WiFiModule::getIP() {
  return _apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}
String WiFiModule::getSSID() { return WiFi.SSID(); }
String WiFiModule::getHostname() { return _hostname; }
