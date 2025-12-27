#include <ESP32WebOTA.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>

AsyncWebServer server(80);

// Set current version of the firmware
#define CURRENT_VERSION "0.1.0"

void setup() {
  Serial.begin(115200);
  WiFi.begin("Papa_wifi_2.4G", "9999900000");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected IP address: ");
  Serial.println(WiFi.localIP());

  static ESP32WebOTA ota(server);
  ota.begin(CURRENT_VERSION);
  Serial.print("OTA version: ");
  Serial.println(ota.getVersion());
}

void loop() {}
