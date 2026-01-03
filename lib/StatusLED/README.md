# StatusLED

Simple LED Status Indicator for ESP32/Arduino with **auto-blink** via FreeRTOS Task.

## Quick Start

Copy these files to your project:
- `StatusLED.h`
- `StatusLED.cpp`

```cpp
#include "StatusLED.h"

StatusLED led(2);          // GPIO 2 (built-in LED on most ESP32)
// StatusLED led(2, true); // Active LOW (LED on when pin is LOW)

void setup() {
    led.begin();                         // Auto-starts background task
    led.setPattern(LED_BLINK_SLOW);      // Set pattern
}

void loop() {
    // No need to call led.loop()! It runs automatically.
}
```

## Available Patterns

| Pattern             | ON Time | OFF Time | Use Case              |
|---------------------|---------|----------|-----------------------|
| `LED_OFF`           | -       | -        | LED off               |
| `LED_ON`            | -       | -        | LED solid on          |
| `LED_BLINK_SLOW`    | 500ms   | 5000ms   | Normal / connected    |
| `LED_BLINK_FAST`    | 500ms   | 500ms    | Connecting / warning  |
| `LED_BLINK_VERY_FAST` | 500ms | 50ms     | Critical / OTA        |

## Configuration

Edit timing in `StatusLED.h`:

```cpp
#define LED_ON_TIME_MS         500   // ON duration (same for all patterns)
#define LED_SLOW_INTERVAL_MS   5000  // OFF duration for slow blink
#define LED_FAST_INTERVAL_MS   500   // OFF duration for fast blink
#define LED_VERY_FAST_MS       50    // OFF duration for very fast blink
```

## Example: WiFi Status Indicator

```cpp
void onWiFiConnecting()  { led.setPattern(LED_BLINK_FAST); }
void onWiFiConnected()   { led.setPattern(LED_BLINK_SLOW); }
void onWiFiDisconnected(){ led.setPattern(LED_BLINK_FAST); }
void onAPMode()          { led.setPattern(LED_ON); }
void onOTAStart()        { led.setPattern(LED_BLINK_VERY_FAST); }
void onOTAEnd()          { led.setPattern(LED_BLINK_SLOW); }
void onError()           { led.setPattern(LED_OFF); }
```

## Example: Change Pattern Anytime

```cpp
void loop() {
    if (someCondition) {
        led.setPattern(LED_BLINK_FAST);
    } else {
        led.setPattern(LED_BLINK_SLOW);
    }
    // No led.loop() needed!
}
```

## How It Works

- `led.begin()` creates a FreeRTOS background task
- The task automatically handles blinking at 10ms intervals
- `setPattern()` only updates when pattern changes (no flicker)
- Works with both active-high and active-low LEDs

## Notes

- **ESP32 only** (uses FreeRTOS)
- Uses ~2KB stack for LED task
- `loop()` method still available for manual use if needed

## License

MIT
