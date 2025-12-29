#pragma once

// Default Access Point Name when connection fails
#ifndef ESP32WEBWIFI_AP_NAME
#define ESP32WEBWIFI_AP_NAME "ESP32-Config"
#endif

// Default Access Point Password (min 8 chars, or empty "" for open)
#ifndef ESP32WEBWIFI_AP_PASSWORD
#define ESP32WEBWIFI_AP_PASSWORD "12345678"
#endif

// Default mDNS hostname (http://hostname.local)
#ifndef ESP32WEBWIFI_DEFAULT_HOSTNAME
#define ESP32WEBWIFI_DEFAULT_HOSTNAME "esp32"
#endif

// Preferences Namespace Key (Max 15 chars)
#ifndef ESP32WEBWIFI_PREFS_KEY
#define ESP32WEBWIFI_PREFS_KEY "wifi_conf"
#endif

// WiFi Connection Timeout in Milliseconds
#ifndef ESP32WEBWIFI_CONN_TIMEOUT_MS
#define ESP32WEBWIFI_CONN_TIMEOUT_MS 10000
#endif

// AP Mode Timeout in Milliseconds (0 = disable, default 10min)
// System will restart if no user connects within this time.
#ifndef ESP32WEBWIFI_AP_TIMEOUT_MS
#define ESP32WEBWIFI_AP_TIMEOUT_MS 600000
#endif
