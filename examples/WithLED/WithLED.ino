/**
 * ESP32WebManager - With LED Status Indicator
 *
 * This example adds visual feedback using the built-in LED.
 * LED patterns indicate the current device state.
 *
 * LED Patterns:
 * - Slow blink (1s): Connecting to WiFi
 * - Fast blink (0.2s): AP Mode active
 * - Solid ON: WiFi connected
 * - Very fast blink (0.1s): OTA update in progress
 *
 * Hardware: ESP32 with built-in LED (usually GPIO 2)
 */

#include <ESP32WebManager.h>

// ===== Configuration =====
#define LED_PIN 2 // Built-in LED on most ESP32 boards

// ===== LED Pattern Definitions =====
enum LEDPattern {
  LED_OFF,
  LED_ON,
  LED_BLINK_SLOW, // Connecting
  LED_BLINK_FAST, // AP Mode
  LED_BLINK_OTA   // OTA Update
};

// ===== Global Variables =====
AsyncWebServer server(80);
DNSServer dns;
ESP32WebManager manager(server, dns);

LEDPattern currentPattern = LED_OFF;
unsigned long lastBlink = 0;
bool ledState = false;

// ===== LED Control Functions =====
void setLEDPattern(LEDPattern pattern) { currentPattern = pattern; }

void updateLED() {
  unsigned long now = millis();
  int interval = 0;

  switch (currentPattern) {
  case LED_OFF:
    digitalWrite(LED_PIN, LOW);
    return;

  case LED_ON:
    digitalWrite(LED_PIN, HIGH);
    return;

  case LED_BLINK_SLOW:
    interval = 1000; // 1 second
    break;

  case LED_BLINK_FAST:
    interval = 200; // 0.2 seconds
    break;

  case LED_BLINK_OTA:
    interval = 100; // 0.1 seconds
    break;
  }

  if (now - lastBlink >= interval) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  setLEDPattern(LED_BLINK_SLOW); // Start with "connecting" pattern

  Serial.println("\n=== ESP32WebManager + LED Example ===");

  // Configure manager
  manager.setHostname("esp32-led");
  manager.setOTAAuth("admin", "admin");

  // WiFi Connected → Solid LED
  manager.onWiFiConnect([]() {
    Serial.println("✓ WiFi Connected!");
    Serial.println("  IP: " + manager.getIP());
    setLEDPattern(LED_ON);
  });

  // AP Mode → Fast blink
  manager.onAPMode([]() {
    Serial.println("⚠ AP Mode - Connect to ESP32-Config");
    setLEDPattern(LED_BLINK_FAST);
  });

  // OTA Start → Very fast blink
  manager.onOTAStart([]() {
    Serial.println("📦 OTA Starting...");
    setLEDPattern(LED_BLINK_OTA);
  });

  // OTA End → Solid LED
  manager.onOTAEnd([]() {
    Serial.println("✓ OTA Complete!");
    setLEDPattern(LED_ON);
  });

  // OTA Error → Fast blink (like AP mode)
  manager.onOTAError([](String error) {
    Serial.println("✗ OTA Error: " + error);
    setLEDPattern(LED_BLINK_FAST);
  });

  // Start manager
  manager.begin("1.0.0");

  Serial.println("System Ready!");
}

// ===== Main Loop =====
void loop() {
  // Update LED based on current pattern
  updateLED();

  // Your application code here...
}
