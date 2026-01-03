/**
 * ESP32WebManager - Custom Routes Example
 *
 * This example shows how to add your own API endpoints
 * alongside the built-in WiFi and OTA routes.
 *
 * Custom Endpoints Added:
 * - GET  /api/sensor     → Returns sensor data as JSON
 * - POST /api/led        → Control an LED (on/off)
 * - GET  /api/status     → Returns device status
 *
 * Note: You have direct access to the AsyncWebServer instance
 * to add any routes you need!
 */

#include <ESP32WebManager.h>

// ===== Configuration =====
#define LED_PIN 2
#define SENSOR_PIN 34 // Analog input

// ===== Global Variables =====
AsyncWebServer server(80);
DNSServer dns;
ESP32WebManager manager(server, dns);

bool ledState = false;
unsigned long startTime = 0;

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32WebManager + Custom Routes ===");

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  startTime = millis();

  // Configure manager
  manager.setHostname("esp32-api");
  manager.setOTAAuth("admin", "admin");

  manager.onWiFiConnect(
      []() { Serial.println("✓ WiFi Connected: " + manager.getIP()); });

  manager.onAPMode([]() { Serial.println("⚠ AP Mode Active"); });

  // ================================================
  // ADD YOUR CUSTOM ROUTES BEFORE manager.begin()!
  // ================================================

  // Custom API: Get sensor data
  server.on("/api/sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
    int rawValue = analogRead(SENSOR_PIN);
    float voltage = (rawValue / 4095.0) * 3.3;

    String json = "{";
    json += "\"raw\":" + String(rawValue) + ",";
    json += "\"voltage\":" + String(voltage, 2) + ",";
    json += "\"unit\":\"V\"";
    json += "}";

    request->send(200, "application/json", json);
  });

  // Custom API: Control LED
  server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();

      if (state == "on" || state == "1") {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
        request->send(200, "application/json", "{\"led\":\"on\"}");
      } else if (state == "off" || state == "0") {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
        request->send(200, "application/json", "{\"led\":\"off\"}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid state\"}");
      }
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing 'state' parameter\"}");
    }
  });

  // Custom API: Device status
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    unsigned long uptime = (millis() - startTime) / 1000;

    String json = "{";
    json += "\"version\":\"" + manager.getVersion() + "\",";
    json += "\"ip\":\"" + manager.getIP() + "\",";
    json += "\"hostname\":\"" + manager.wifi.getHostname() + "\",";
    json += "\"ssid\":\"" + manager.wifi.getSSID() + "\",";
    json += "\"uptime\":" + String(uptime) + ",";
    json += "\"led\":" + String(ledState ? "true" : "false") + ",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap());
    json += "}";

    request->send(200, "application/json", json);
  });

  // ================================================
  // Start manager (this also starts the server)
  // ================================================
  manager.begin("1.0.0");

  Serial.println("\n=== Custom API Endpoints ===");
  Serial.println("GET  /api/sensor  → Read analog sensor");
  Serial.println("POST /api/led     → Control LED (state=on/off)");
  Serial.println("GET  /api/status  → Device status");
  Serial.println("\n=== Built-in Endpoints ===");
  Serial.println("GET  /api/wifi/scan      → Scan WiFi networks");
  Serial.println("POST /api/ota/update     → Upload firmware");
  Serial.println("GET  /wifi               → WiFi config page");
  Serial.println("GET  /ota                → OTA update page");
}

// ===== Main Loop =====
void loop() {
  // Your sensor reading, control logic, etc.
  delay(100);
}
