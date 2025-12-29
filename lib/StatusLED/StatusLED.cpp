#include "StatusLED.h"

StatusLED::StatusLED(uint8_t pin, bool activeLow)
    : _pin(pin), _activeLow(activeLow) {}

void StatusLED::begin() {
  pinMode(_pin, OUTPUT);
  setLed(false);
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
  int interval = 1000;

  if (_pattern == LED_BLINK_FAST)
    interval = 200;
  else if (_pattern == LED_BLINK_OTA)
    interval = 50;
  else if (_pattern == LED_BLINK_SLOW)
    interval = 1000;

  if (now - _lastToggle >= interval) {
    _lastToggle = now;
    setLed(!_state);
  }
}
