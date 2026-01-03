#include "StatusLED.h"
#include <ESP32WebManager.h>

AsyncWebServer server(80);
DNSServer dns;
StatusLED led(2);

ESP32WebManager manager(server, dns);

void setup() {
  Serial.begin(115200);
  led.begin();
  led.setPattern(LED_BLINK_FAST); // Connecting to WiFi

  // Configuration
  manager.setHostname("esp32");
  manager.setOTAAuth("admin", "admin");

  // WiFi Callbacks
  manager.onWiFiConnect([]() {
    Serial.print("WiFi Connected: ");
    Serial.println(manager.getIP());
    led.setPattern(LED_BLINK_SLOW); // Connected = slow blink
  });

  manager.onAPMode([]() {
    Serial.println("AP Mode Active - Connect to ESP32-Config");
    Serial.print("AP IP: ");
    Serial.println(manager.getIP());
    led.setPattern(LED_ON); // AP Mode = solid on
  });

  // OTA Callbacks
  manager.onOTAStart([]() {
    Serial.println("OTA Update Starting...");
    led.setPattern(LED_BLINK_VERY_FAST); // OTA = very fast blink
  });

  manager.onOTAEnd([]() {
    Serial.println("OTA Update Complete!");
    led.setPattern(LED_ON);
  });

  manager.onOTAProgress(
      [](int percent) { Serial.printf("OTA Progress: %d%%\n", percent); });

  manager.onOTAError([](String error) {
    Serial.println("OTA Error: " + error);
    led.setPattern(LED_BLINK_FAST);
  });

  // Start everything
  manager.begin("1.0.0");

  Serial.println("System Ready!");
  Serial.print("Version: ");
  Serial.println(manager.getVersion());
}

void loop() {
  // ไม่ต้องเรียก manager.loop() หรือ led.loop() แล้ว
  // ทำงานอัตโนมัติผ่าน FreeRTOS Task
}
