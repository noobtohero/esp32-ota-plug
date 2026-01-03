#pragma once

// ===== AP Mode Settings =====
#ifndef WEBMGR_AP_NAME
#define WEBMGR_AP_NAME "ESP32-Config"
#endif

#ifndef WEBMGR_AP_PASSWORD
#define WEBMGR_AP_PASSWORD "12345678"
#endif

#ifndef WEBMGR_AP_TIMEOUT_MS
#define WEBMGR_AP_TIMEOUT_MS 600000 // 10 minutes
#endif

// ===== WiFi Settings =====
#ifndef WEBMGR_DEFAULT_HOSTNAME
#define WEBMGR_DEFAULT_HOSTNAME "esp32"
#endif

#ifndef WEBMGR_CONN_TIMEOUT_MS
#define WEBMGR_CONN_TIMEOUT_MS 10000
#endif

#ifndef WEBMGR_PREFS_KEY
#define WEBMGR_PREFS_KEY "wifi_conf"
#endif

// ===== OTA Settings =====
#ifndef WEBMGR_OTA_USER
#define WEBMGR_OTA_USER "admin"
#endif

#ifndef WEBMGR_OTA_PASS
#define WEBMGR_OTA_PASS "admin"
#endif
