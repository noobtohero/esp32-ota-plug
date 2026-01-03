# ESP32WebManager

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)](https://platformio.org/)

**All-in-one WiFi Manager + OTA Update library for ESP32**

A modern, async, and FreeRTOS-compatible library that provides:
- 📶 **WiFi Manager** - Captive Portal for easy WiFi configuration
- 🔄 **OTA Updates** - Web-based firmware updates with progress tracking
- 🌐 **mDNS Support** - Access your device via `http://esp32.local`
- 🔒 **Authentication** - Password-protected OTA updates
- 💾 **Persistent Storage** - WiFi credentials saved in NVR

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| **Captive Portal** | Auto-redirect to config page when in AP mode |
| **Async WiFi Scan** | Non-blocking network scanning |
| **AP Timeout** | Auto-restart if no config after 10 minutes |
| **Password Protected AP** | Secure your configuration portal |
| **OTA with Progress** | Real-time upload progress via callbacks |
| **Factory Reset** | Clear WiFi credentials via web UI |
| **LED Status** | Built-in status indication support |
| **FreeRTOS Safe** | No blocking calls, works with tasks |

---

## 📦 Installation

### PlatformIO (Recommended)

Add to your `platformio.ini`:

```ini
lib_deps =
    me-no-dev/AsyncTCP
    me-no-dev/ESPAsyncWebServer
```

Copy `/lib/ESP32WebManager` to your project's `lib/` folder.

---

## 🚀 Quick Start

```cpp
#include <ESP32WebManager.h>

AsyncWebServer server(80);
DNSServer dns;
ESP32WebManager manager(server, dns);

void setup() {
    Serial.begin(115200);
    
    // Configuration
    manager.setHostname("my-device");
    manager.setOTAAuth("admin", "password");
    
    // Callbacks
    manager.onWiFiConnect([]() {
        Serial.println("WiFi Connected!");
        Serial.println(manager.getIP());
    });
    
    manager.onAPMode([]() {
        Serial.println("Config Portal Active");
    });
    
    manager.onOTAProgress([](int percent) {
        Serial.printf("OTA: %d%%\n", percent);
    });
    
    // Start
    manager.begin("1.0.0");
}

void loop() {
    manager.loop();
}
```

---

## 📡 API Reference

### Manager Methods

| Method | Description |
|--------|-------------|
| `begin(version)` | Initialize WiFi, OTA, and start server |
| `loop()` | Process DNS and timeouts (call in loop) |
| `setHostname(name)` | Set mDNS hostname |
| `setOTAAuth(user, pass)` | Set OTA authentication |
| `isAPMode()` | Check if in Access Point mode |
| `getIP()` | Get current IP address |
| `getVersion()` | Get firmware version |

### Callbacks

| Callback | Parameters | Description |
|----------|------------|-------------|
| `onWiFiConnect(fn)` | `void()` | Called when WiFi connects |
| `onAPMode(fn)` | `void()` | Called when AP mode starts |
| `onOTAStart(fn)` | `void()` | Called when OTA begins |
| `onOTAEnd(fn)` | `void()` | Called when OTA completes |
| `onOTAProgress(fn)` | `void(int)` | Called with progress % |
| `onOTAError(fn)` | `void(String)` | Called on OTA error |

---

## 🌐 Web Endpoints

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/wifi/scan` | Start WiFi scan |
| GET | `/api/wifi/scanresult` | Get scan results |
| POST | `/api/wifi/connect` | Save WiFi credentials |
| POST | `/api/wifi/reset` | Factory reset WiFi |
| GET | `/api/ota/version` | Get firmware version |
| POST | `/api/ota/update` | Upload firmware |
| GET | `/api/ota/progress` | Get OTA progress |
| GET | `/api/ping` | Health check |

---

## ⚙️ Configuration

Override defaults in your code or create `ESP32WebManagerConfig.h`:

```c
// AP Settings
#define WEBMGR_AP_NAME "MyDevice-Setup"
#define WEBMGR_AP_PASSWORD "12345678"
#define WEBMGR_AP_TIMEOUT_MS 600000  // 10 minutes

// WiFi Settings
#define WEBMGR_DEFAULT_HOSTNAME "mydevice"
#define WEBMGR_CONN_TIMEOUT_MS 10000

// OTA Settings
#define WEBMGR_OTA_USER "admin"
#define WEBMGR_OTA_PASS "admin"
```

---

## 📂 Project Structure

```
lib/ESP32WebManager/
├── ESP32WebManager.h           # Main header
├── ESP32WebManager.cpp         # Main implementation
├── ESP32WebManagerConfig.h     # Configuration
└── modules/
    ├── WiFiModule.h/cpp        # WiFi logic
    └── OTAModule.h/cpp         # OTA logic

data/
├── wifi/                       # WiFi config portal UI
│   ├── index.html
│   ├── style.css
│   └── app.js
└── ota/                        # OTA update UI
    ├── index.html
    ├── style.css
    └── app.js
```

---

## 📱 Screenshots

### WiFi Configuration Portal
Connect to `ESP32-Config` AP, open any browser → redirects to config page.

### OTA Update Page
Navigate to `http://[device-ip]/ota` → Enter credentials → Upload firmware.

---

## 🔧 Dependencies

- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

---

## 📄 License

MIT License - See [LICENSE](LICENSE) file.

---

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📝 Changelog

### v1.0.0 (2024-12-29)
- 🎉 Initial release
- ✨ Unified WiFi Manager + OTA library
- 🔄 Async WiFi scanning (non-blocking)
- 📶 Captive Portal with auto-redirect
- 🔒 Password-protected AP and OTA
- ⏱️ AP timeout with auto-restart
- 📡 mDNS support
- 💾 Persistent WiFi credentials
- 🎨 Modern responsive Web UI
- 🛡️ FreeRTOS compatible
