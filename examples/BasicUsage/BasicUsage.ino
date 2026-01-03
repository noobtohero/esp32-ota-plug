/**
 * ESP32WebManager - Basic Usage Example
 *
 * This is the simplest way to use ESP32WebManager.
 * It provides WiFi configuration portal and OTA updates out of the box.
 *
 * Features:
 * - Captive Portal for WiFi setup
 * - OTA firmware updates via web browser
 * - mDNS support (http://esp32.local)
 *
 * Hardware: Any ESP32 board
 *
 * Instructions:
 * 1. Upload this sketch
 * 2. Connect to "ESP32-Config" WiFi network (password: 12345678)
 * 3. Open browser → you'll be redirected to config page
 * 4. Select your WiFi and enter password
 * 5. Device will restart and connect to your network
 */

#include <ESP32WebManager.h>

// Create server and DNS instances
AsyncWebServer server(80);
DNSServer dns;

// Create manager instance
ESP32WebManager manager(server, dns);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  Serial.println("\n=== ESP32WebManager Basic Example ===");

  // Optional: Set custom hostname (default: "esp32")
  // Access via http://mydevice.local
  manager.setHostname("mydevice");

  // Optional: Set OTA authentication (default: admin/admin)
  manager.setOTAAuth("admin", "secret123");

  // Callback: WiFi connected successfully
  manager.onWiFiConnect([]() {
    Serial.println("✓ WiFi Connected!");
    Serial.print("  IP Address: ");
    Serial.println(manager.getIP());
    Serial.print("  Access: http://");
    Serial.print(manager.wifi.getHostname());
    Serial.println(".local");
  });

  // Callback: Entered AP mode (no saved WiFi or connection failed)
  manager.onAPMode([]() {
    Serial.println("⚠ AP Mode Active");
    Serial.println("  Connect to: ESP32-Config");
    Serial.println("  Password: 12345678");
    Serial.print("  Config URL: http://");
    Serial.println(manager.getIP());
  });

  // Callback: OTA update started
  manager.onOTAStart([]() { Serial.println("📦 OTA Update Starting..."); });

  // Callback: OTA progress
  manager.onOTAProgress(
      [](int percent) { Serial.printf("   Progress: %d%%\n", percent); });

  // Callback: OTA completed
  manager.onOTAEnd(
      []() { Serial.println("✓ OTA Update Complete! Restarting..."); });

  // Callback: OTA error
  manager.onOTAError([](String error) {
    Serial.print("✗ OTA Error: ");
    Serial.println(error);
  });

  // Start the manager with firmware version
  manager.begin("1.0.0");

  Serial.println("\n=== System Ready ===");
  Serial.print("Firmware Version: ");
  Serial.println(manager.getVersion());
}

void loop() {
  // No need to call manager.loop() - it runs automatically!

  // Your application code here...
  delay(1000);
}
