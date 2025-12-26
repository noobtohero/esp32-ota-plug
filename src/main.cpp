#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESP32WebOTA.h>

AsyncWebServer server(80);

void setup() {
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(200);

  SPIFFS.begin(true);

  ESP32WebOTA::boot();
  ESP32WebOTA(server).begin();

  server.begin();
}

void loop() {}
