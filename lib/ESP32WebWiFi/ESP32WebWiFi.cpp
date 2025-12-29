#include "ESP32WebWiFi.h"
#include <SPIFFS.h>

ESP32WebWiFi::ESP32WebWiFi(AsyncWebServer &server, DNSServer &dns)
    : _server(server), _dns(dns) {}

void ESP32WebWiFi::begin() {
  _prefs.begin(ESP32WEBWIFI_PREFS_KEY, false);
  String ssid = _prefs.getString("ssid", "");
  String pass = _prefs.getString("pass", "");
  _prefs.end();

  _state = WEBWIFI_CONNECTING;

  if (ssid.length() > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    // Wait up to 10 seconds for connection
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < ESP32WEBWIFI_CONN_TIMEOUT_MS) {
      delay(500);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    _state = WEBWIFI_CONNECTED;
    _apMode = false;
    setupMDNS();
    if (_cbConnect)
      _cbConnect();
  } else {
    _state = WEBWIFI_AP_MODE;
    startAPMode();
  }

  setupRoutes();
}

void ESP32WebWiFi::startAPMode() {
  _apMode = true;
  _apStartTime = millis();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ESP32WEBWIFI_AP_NAME, ESP32WEBWIFI_AP_PASSWORD);
  setupMDNS();
  if (_cbAPMode)
    _cbAPMode();

  // Redirect all DNS requests to this IP (Captive Portal)
  _dns.start(53, "*", WiFi.softAPIP());
}

void ESP32WebWiFi::setupRoutes() {
  // Fix CORS for Captive Portal (allows fetch from google.com etc. to work)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  // Prevent browser caching (ensures latest CSS/JS is always loaded)
  DefaultHeaders::Instance().addHeader("Cache-Control",
                                       "no-cache, no-store, must-revalidate");

  // Serve WiFi Portal Static Files
  // If in AP Mode, redirect root to /wifi/index.html or serve it directly

  // API: Start Scan (Async - non-blocking)
  _server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *req) {
    WiFi.scanNetworks(true); // true = async, returns immediately
    req->send(200, "application/json", "{\"status\":\"scanning\"}");
  });

  // API: Get Scan Result (Poll this until result is ready)
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

    // Build Result JSON
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

                 _prefs.begin(ESP32WEBWIFI_PREFS_KEY, false);
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
    req->send(200, "text/plain", "Resetting settings and restarting...");
    delay(500);
    resetSettings(); // Calls startAPMode
    // Actually resetSettings() just clears prefs and switches to AP.
    // If we want full factory reset feel, maybe restart?
    // But forcing AP mode is enough for now.
    // Wait, resetSettings() calls startAPMode() which sets _state =
    // WEBWIFI_AP_MODE
  });

  // Portal UI
  if (_apMode) {
    // In AP mode, serve wifi page at root
    _server.serveStatic("/", SPIFFS, "/wifi/").setDefaultFile("index.html");

    // Captive Portal Redirect
    _server.onNotFound([](AsyncWebServerRequest *req) { req->redirect("/"); });
  } else {
    // In STA mode, mapped to /wifi
    _server.serveStatic("/wifi", SPIFFS, "/wifi/").setDefaultFile("index.html");
  }
}

bool ESP32WebWiFi::isAPMode() { return _apMode; }

String ESP32WebWiFi::getIP() {
  return _apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}

String ESP32WebWiFi::getSSID() { return WiFi.SSID(); }

void ESP32WebWiFi::loop() {
  if (_apMode) {
    _dns.processNextRequest();
    if (ESP32WEBWIFI_AP_TIMEOUT_MS > 0 &&
        millis() - _apStartTime > ESP32WEBWIFI_AP_TIMEOUT_MS) {
      ESP.restart();
    }
  }
}

WebWiFiState ESP32WebWiFi::getStatus() { return _state; }

void ESP32WebWiFi::setHostname(const char *hostname) { _hostname = hostname; }
void ESP32WebWiFi::onConnect(std::function<void()> fn) { _cbConnect = fn; }
void ESP32WebWiFi::onAPMode(std::function<void()> fn) { _cbAPMode = fn; }

void ESP32WebWiFi::setupMDNS() {
  if (!MDNS.begin(_hostname.c_str())) {
    // Fail
  } else {
    MDNS.addService("http", "tcp", 80);
  }
}

void ESP32WebWiFi::resetSettings() {
  _prefs.begin(ESP32WEBWIFI_PREFS_KEY, false);
  _prefs.clear();
  _prefs.end();

  WiFi.disconnect(true, true);

  _state = WEBWIFI_AP_MODE;
  startAPMode();
}
