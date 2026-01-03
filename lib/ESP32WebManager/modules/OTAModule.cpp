#include "OTAModule.h"

OTAModule::OTAModule(AsyncWebServer &server) : _server(server) {}

void OTAModule::begin(const char *version) {
  _version = version;

  if (!SPIFFS.begin(true)) {
    // SPIFFS mount failed
  }

  setupRoutes();
}

void OTAModule::setupRoutes() {
  // API: Version
  _server.on("/api/ota/version", HTTP_GET, [this](AsyncWebServerRequest *req) {
    req->send(200, "application/json", "{\"version\":\"" + _version + "\"}");
  });

  // API: Progress
  _server.on("/api/ota/progress", HTTP_GET, [this](AsyncWebServerRequest *req) {
    req->send(200, "application/json",
              "{\"progress\":" + String(_otaProgress) + "}");
  });

  // OTA Page (Protected)
  _server.on("/ota", HTTP_GET, [this](AsyncWebServerRequest *req) {
    if (!checkAuth(req))
      return;
    req->send(SPIFFS, "/ota/index.html", "text/html");
  });

  // OTA Assets
  _server.serveStatic("/ota/", SPIFFS, "/ota/");

  // OTA Upload Handler
  _server.on(
      "/api/ota/update", HTTP_POST,
      [this](AsyncWebServerRequest *req) {
        if (!checkAuth(req))
          return;
        bool success = !Update.hasError();
        req->send(200, "text/plain", success ? "OK" : "FAIL");
        if (success) {
          delay(1000);
          ESP.restart();
        }
      },
      [this](AsyncWebServerRequest *req, String filename, size_t index,
             uint8_t *data, size_t len, bool final) {
        if (!checkAuth(req))
          return;

        if (!index) {
          // Start
          size_t contentLen = req->contentLength();
          int cmd = filename.endsWith(".bin") ? U_FLASH : U_SPIFFS;

          if (_cbStart)
            _cbStart();

          if (!Update.begin(contentLen, cmd)) {
            if (_cbError)
              _cbError("Update begin failed");
          }
        }

        // Write
        if (Update.write(data, len) != len) {
          if (_cbError)
            _cbError("Write error");
        }

        // Progress
        if (Update.size() > 0) {
          _otaProgress = (Update.progress() * 100) / Update.size();
          if (_cbProgress)
            _cbProgress(_otaProgress);
        }

        // Final
        if (final) {
          if (Update.end(true)) {
            if (_cbEnd)
              _cbEnd();
          } else {
            if (_cbError)
              _cbError("Update end failed");
          }
        }
      });

  // Ping for health check
  _server.on("/api/ping", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "pong");
  });
}

bool OTAModule::checkAuth(AsyncWebServerRequest *req) {
  if (!req->authenticate(_authUser.c_str(), _authPass.c_str())) {
    req->requestAuthentication();
    return false;
  }
  return true;
}

// Setters
void OTAModule::setAuth(const char *user, const char *pass) {
  _authUser = user;
  _authPass = pass;
}

void OTAModule::onStart(std::function<void()> fn) { _cbStart = fn; }
void OTAModule::onEnd(std::function<void()> fn) { _cbEnd = fn; }
void OTAModule::onProgress(std::function<void(int)> fn) { _cbProgress = fn; }
void OTAModule::onError(std::function<void(String)> fn) { _cbError = fn; }

String OTAModule::getVersion() { return _version; }
