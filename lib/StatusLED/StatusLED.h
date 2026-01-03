#pragma once
#include <Arduino.h>

// ===== LED Timing Configuration =====
#define LED_ON_TIME_MS 200        // ON duration (constant for all patterns)
#define LED_SLOW_INTERVAL_MS 5000 // OFF duration for slow blink
#define LED_FAST_INTERVAL_MS 500  // OFF duration for fast blink
#define LED_VERY_FAST_MS 50       // OFF duration for very fast blink

enum LEDPattern {
  LED_OFF,
  LED_ON,
  LED_BLINK_SLOW,     // Normal / connected
  LED_BLINK_FAST,     // Warning / connecting
  LED_BLINK_VERY_FAST // Critical / OTA / urgent
};

class StatusLED {
public:
  StatusLED(uint8_t pin, bool activeLow = false);
  void begin(); // Auto-starts background task
  void loop();  // Optional: call manually if needed
  void setPattern(LEDPattern pattern);

private:
  uint8_t _pin;
  bool _activeLow;
  LEDPattern _pattern = LED_OFF;
  unsigned long _lastToggle = 0;
  bool _state = false;
  TaskHandle_t _taskHandle = NULL;

  void setLed(bool on);
  int getOnTime();
  int getOffTime();
  static void ledTask(void *param);
};
