#include "ESP32WebOTA.h"
#include "ESP32WebOTAConfig.h"

#include <Preferences.h>
#include <SPIFFS.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static volatile int _ota_progress = 0;

static void ota_restart_task(void *pv) {
  vTaskDelay(pdMS_TO_TICKS(3000));
  ESP.restart();
  vTaskDelete(NULL);
}

String ESP32WebOTA::getVersion() {
  Preferences prefs;
  prefs.begin("ota", false);
  String v = prefs.getString("version", "0.0.0");
  prefs.end();
  return v;
}

void ESP32WebOTA::setVersion(const String &v) {
  Preferences prefs;
  prefs.begin("ota", false);
  prefs.putString("version", v);
  prefs.end();
}

void ESP32WebOTA::boot() {
  Preferences prefs;
  prefs.begin("ota", false);

  esp_ota_img_states_t state;
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (esp_ota_get_state_partition(running, &state) == ESP_OK) {
    if (state == ESP_OTA_IMG_PENDING_VERIFY) {
      esp_ota_mark_app_valid_cancel_rollback();
      prefs.putUInt("retry", 0);
    }
  }
  prefs.end();
}

ESP32WebOTA::ESP32WebOTA(AsyncWebServer &server) : _server(server) {}

void ESP32WebOTA::begin(const char *currentVersion) {
  // Ensure SPIFFS is mounted
  if (!SPIFFS.begin(true)) {
    // OTA: SPIFFS Mount Failed
  }

  // Run boot checks
  this->boot();

  // Set version
  this->setVersion(String(currentVersion));

#if 1
  // Serve static files under /ota from SPIFFS (index.html, app.js, style.css)
  _server.serveStatic("/panel", SPIFFS, "/panel/");

  // Avoid 500 on missing favicon requests
  _server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(204, "text/plain", "");
  });
#endif

  // /status: return JSON with version and uptime (seconds)
  // Requires authentication for security
  ESP32WebOTA *_self = this;
  _server.on("/status", HTTP_GET, [_self](AsyncWebServerRequest *req) {
    // Check authentication
    if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
      return req->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    }

    String ver = _self->getVersion();
    unsigned long up = millis() / 1000;
    String body = "{";
    body += "\"version\":\"" + ver + "\",";
    body += "\"uptime\":" + String(up);
    body += "}";
    req->send(200, "application/json", body);
  });

  // /version: return version only (no auth required for login page)
  _server.on("/version", HTTP_GET, [_self](AsyncWebServerRequest *req) {
    String ver = _self->getVersion();
    String body = "{\"version\":\"" + ver + "\"}";
    req->send(200, "application/json", body);
  });

#if OTA_ENABLE_MANUAL
  // Serve OTA page without server-side auth (uses custom login form)
  // Serve OTA page without server-side auth (uses custom login form)
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(SPIFFS, "/panel/index.html", "text/html");
  });
#endif

#if OTA_ENABLE_MANUAL
  _server.on(
      "/update", HTTP_POST,
      [](AsyncWebServerRequest *req) {
        if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
          return req->send(401, "text/plain", "Unauthorized");
        }
        req->send(200, "text/plain", "OK");
        // schedule reboot after short delay so client receives response
        xTaskCreate(ota_restart_task, "ota_reboot", 4096, NULL, 1, NULL);
      },
      [this](AsyncWebServerRequest *req, String filename, size_t idx,
             uint8_t *data, size_t len, bool fin) {
        // Check auth BEFORE doing anything
        if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
          return;
        }

        static size_t totalSize = 0;

        if (idx == 0) {
          _ota_progress = 0;
          totalSize = req->contentLength();
          if (_cbStart)
            _cbStart();
          // OTA Update Start
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            if (_cbError)
              _cbError("Update begin failed");
          }
        }

        if (Update.write(data, len) != len) {
          // Write Error
        }

        // Calculate real progress
        if (totalSize > 0) {
          _ota_progress = ((idx + len) * 100) / totalSize;
          if (_ota_progress > 99)
            _ota_progress = 99; // Cap at 99 until complete
          if (_cbProgress)
            _cbProgress(_ota_progress);
        }

        if (fin) {
          if (Update.end(true)) {
            _ota_progress = 100;
            if (_cbEnd)
              _cbEnd();
          } else {
            _ota_progress = 0;
            if (_cbError)
              _cbError(Update.errorString());
          }
        }
      });

  // /update-url endpoint for OTA from URL
  _server.on("/update-url", HTTP_POST, [](AsyncWebServerRequest *req) {
    // Check authentication
    if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
      return req->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    }

    // Check if URL parameter exists
    if (!req->hasParam("url", true)) {
      return req->send(400, "application/json",
                       "{\"error\":\"Missing URL parameter\"}");
    }

    String url = req->getParam("url", true)->value();

    // Basic URL validation
    if (url.length() == 0 ||
        (!url.startsWith("http://") && !url.startsWith("https://"))) {
      return req->send(400, "application/json", "{\"error\":\"Invalid URL\"}");
    }

    // Note: Actual URL-based OTA implementation would require HTTPClient
    // For now, we'll just acknowledge the request
    // You would need to implement the actual download and update logic

    req->send(501, "application/json",
              "{\"error\":\"URL-based OTA not implemented yet. Please use "
              "manual upload.\"}");
  });
#endif

  // ota-progress endpoint for UI polling
  _server.on("/ota-progress", HTTP_GET, [](AsyncWebServerRequest *req) {
    // Check authentication
    if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
      return req->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    }

    // Debug log
    // (Disabled for FreeRTOS compatibility)
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "{\"progress\":%d}", _ota_progress);
    if (n < 0) {
      req->send(500, "text/plain", "json_error");
      return;
    }
    req->send(200, "application/json", buf);
  });

  _server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "pong");
  });

  // Start server automatically
  _server.begin();
}

void ESP32WebOTA::onStart(std::function<void()> fn) { _cbStart = fn; }
void ESP32WebOTA::onEnd(std::function<void()> fn) { _cbEnd = fn; }
void ESP32WebOTA::onProgress(std::function<void(int)> fn) { _cbProgress = fn; }
void ESP32WebOTA::onError(std::function<void(String)> fn) { _cbError = fn; }
