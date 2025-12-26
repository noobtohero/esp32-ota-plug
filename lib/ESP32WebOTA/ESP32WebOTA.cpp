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
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP.restart();
  vTaskDelete(NULL);
}

String ESP32WebOTA::getVersion() {
  Preferences prefs;
  prefs.begin("ota", false);
  String v = prefs.getString("version", OTA_CURRENT_VERSION);
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

void ESP32WebOTA::begin() {
#if 1
  // Serve static files under /ota from SPIFFS (index.html, app.js, style.css)
  _server.serveStatic("/ota", SPIFFS, "/ota/");

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
  _server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
      return req->requestAuthentication();
    }
    req->send(SPIFFS, "/ota/index.html", "text/html");
  });
#endif

#if OTA_ENABLE_MANUAL
  _server.on(
      "/update", HTTP_POST,
      [](AsyncWebServerRequest *req) {
        req->send(200, "text/plain", "OK");
        // schedule reboot after short delay so client receives response
        xTaskCreate(ota_restart_task, "ota_reboot", 4096, NULL, 1, NULL);
      },
      [](AsyncWebServerRequest *req, String, size_t idx, uint8_t *data,
         size_t len, bool fin) {
        if (idx == 0) {
          _ota_progress = 0;
          Update.begin(UPDATE_SIZE_UNKNOWN);
        }
        Update.write(data, len);
        if (!fin) {
          if (_ota_progress < 99)
            _ota_progress += 5;
        } else {
          if (Update.end(true)) {
            _ota_progress = 100;
          } else {
            _ota_progress = 0;
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
    Serial.print("OTA from URL requested: ");
    Serial.println(url);

    req->send(501, "application/json",
              "{\"error\":\"URL-based OTA not implemented yet. Please use "
              "manual upload.\"}");
  });
#endif

  // ota-progress endpoint for UI polling
  _server.on("/ota-progress", HTTP_GET, [](AsyncWebServerRequest *req) {
    // Debug log
    Serial.print("/ota-progress requested, progress=");
    Serial.println(_ota_progress);
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
}
