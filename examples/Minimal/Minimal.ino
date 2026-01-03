/**
 * ESP32WebManager - Minimal Example
 * The shortest way to get WiFi + OTA working!
 */

#include <ESP32WebManager.h>

AsyncWebServer server(80);
DNSServer dns;
ESP32WebManager manager(server, dns);

//edit version here for OTA update e.g. "1.0.0"
void setup() { manager.begin("1.0.0"); }

void loop() {
  // Nothing needed here!
}
