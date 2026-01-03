/**
 * StatusLED - Simple LED Status Indicator for ESP32/Arduino
 *
 * See README.md for usage documentation.
 *
 * Patterns: LED_OFF, LED_ON, LED_BLINK_SLOW, LED_BLINK_FAST,
 * LED_BLINK_VERY_FAST Configuration: Edit LED_*_MS macros in StatusLED.h
 */

#include "StatusLED.h"

StatusLED::StatusLED(uint8_t pin, bool activeLow)
    : _pin(pin), _activeLow(activeLow) {}

void StatusLED::begin() {
  pinMode(_pin, OUTPUT);
  setLed(false);

  // Create background task for auto-blink
  xTaskCreatePinnedToCore(ledTask,      // Task function
                          "LEDTask",    // Name
                          2048,         // Stack size
                          this,         // Parameter
                          1,            // Priority
                          &_taskHandle, // Handle
                          0             // Core 0
  );
}

// Static task function
void StatusLED::ledTask(void *param) {
  StatusLED *led = static_cast<StatusLED *>(param);
  while (true) {
    led->loop();
    vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms interval
  }
}

void StatusLED::setLed(bool on) {
  _state = on;
  digitalWrite(_pin, _activeLow ? !on : on);
}

void StatusLED::setPattern(LEDPattern pattern) {
  if (_pattern != pattern) {
    _pattern = pattern;
    _lastToggle = 0; // Reset timer on change
  }

  if (pattern == LED_OFF)
    setLed(false);
  else if (pattern == LED_ON)
    setLed(true);
}

void StatusLED::loop() {
  if (_pattern == LED_OFF || _pattern == LED_ON)
    return;

  unsigned long now = millis();
  int currentInterval = _state ? getOnTime() : getOffTime();

  if (now - _lastToggle >= currentInterval) {
    _lastToggle = now;
    setLed(!_state);
  }
}

int StatusLED::getOnTime() {
  return LED_ON_TIME_MS; // Always 500ms ON
}

int StatusLED::getOffTime() {
  switch (_pattern) {
  case LED_BLINK_SLOW:
    return LED_SLOW_INTERVAL_MS; // 5000ms OFF
  case LED_BLINK_FAST:
    return LED_FAST_INTERVAL_MS; // 500ms OFF
  case LED_BLINK_VERY_FAST:
    return LED_VERY_FAST_MS; // 50ms OFF
  default:
    return LED_FAST_INTERVAL_MS;
  }
}
