#pragma once

#define OTA_CURRENT_VERSION "0.0.1"

#define OTA_AUTO_ENABLED true
#define OTA_AUTO_VERSION_URL "https://server/version.txt"
#define OTA_AUTO_FW_URL "https://server/firmware.bin"

#define OTA_AUTO_MAX_RETRY 3

// ⚠️ SECURITY WARNING: Change these credentials before deployment!
// Default credentials are for development only
#define OTA_AUTH_USER "admin"
#define OTA_AUTH_PASS "admin1234"

#define OTA_ENABLE_MANUAL true
#define OTA_ENABLE_PROGRESS true
