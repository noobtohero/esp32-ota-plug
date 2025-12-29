#include "StatusLED.h"
#include <ESP32WebOTA.h>
#include <ESP32WebWiFi.h>
#include <ESPAsyncWebServer.h>


AsyncWebServer server(80);
DNSServer dns;
StatusLED led(2); // Builtin LED

ESP32WebWiFi wifi(server, dns);
ESP32WebOTA ota(server);

void setup() {
  Serial.begin(115200);
  led.begin();
  led.setPattern(LED_BLINK_SLOW); // Default: Connecting

  // Configuration & Callbacks
  wifi.setHostname("esp32-ota-plug");
  wifi.onConnect([]() {
    Serial.println("WiFi Connected!");
    led.setPattern(LED_ON);
  });
  wifi.onAPMode([]() {
    Serial.println("WiFi Config Portal Active");
    led.setPattern(LED_BLINK_FAST);
  });

  ota.onStart([]() {
    Serial.println("OTA Started");
    led.setPattern(LED_BLINK_OTA);
  });
  ota.onEnd([]() {
    Serial.println("OTA Finished");
    led.setPattern(LED_ON); // Back to normal
  });
  ota.onError([](String e) {
    Serial.println(e);
    led.setPattern(LED_BLINK_FAST); // Error indication
  });

  // Initialize
  wifi.begin();
  ota.begin("0.2.0");

  Serial.println("System Ready");
}

void loop() {
  wifi.loop();
  led.loop();
}
