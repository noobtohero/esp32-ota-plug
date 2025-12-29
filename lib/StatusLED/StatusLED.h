#pragma once
#include <Arduino.h>

enum LEDPattern {
  LED_OFF,
  LED_ON,
  LED_BLINK_SLOW, // Connecting (1000ms)
  LED_BLINK_FAST, // AP Mode (200ms)
  LED_BLINK_OTA   // OTA (50ms)
};

class StatusLED {
public:
  StatusLED(uint8_t pin, bool activeLow = false);
  void begin();
  void loop();
  void setPattern(LEDPattern pattern);

private:
  uint8_t _pin;
  bool _activeLow;
  LEDPattern _pattern = LED_OFF;
  unsigned long _lastToggle = 0;
  bool _state = false;
  int _interval = 1000;

  void setLed(bool on);
};
