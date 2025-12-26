#include "ESP32WebOTA.h"
#include "ESP32WebOTAConfig.h"

#include <Preferences.h>
#include <Update.h>
#include <SPIFFS.h>
#include <esp_ota_ops.h>

void ESP32WebOTA::boot() {
  Preferences prefs;
  prefs.begin("ota", false);

  esp_ota_img_states_t state;
  const esp_partition_t* running =
    esp_ota_get_running_partition();

  if (esp_ota_get_state_partition(running, &state) == ESP_OK) {
    if (state == ESP_OTA_IMG_PENDING_VERIFY) {
      esp_ota_mark_app_valid_cancel_rollback();
      prefs.putUInt("retry", 0);
    }
  }
  prefs.end();
}

ESP32WebOTA::ESP32WebOTA(AsyncWebServer& server)
  : _server(server) {}

void ESP32WebOTA::begin() {

#if OTA_ENABLE_MANUAL
  _server.on("/ota", HTTP_GET, [](AsyncWebServerRequest* req) {
    if (!req->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
      return req->requestAuthentication();
    }
    req->send(SPIFFS, "/ota/index.html", "text/html");
  });
#endif

#if OTA_ENABLE_MANUAL
  _server.on("/update", HTTP_POST,
    [](AsyncWebServerRequest* req) {
      req->send(200, "text/plain", "OK");
      ESP.restart();
    },
    [](AsyncWebServerRequest*, String, size_t idx,
       uint8_t* data, size_t len, bool fin) {

      if (idx == 0) Update.begin(UPDATE_SIZE_UNKNOWN);
      Update.write(data, len);
      if (fin) Update.end(true);
    });
#endif
}
