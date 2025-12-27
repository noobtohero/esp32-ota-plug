# ESP32 OTA Plug v0.0.1 🚀

A modern, secure, and beautiful Web OTA (Over-The-Air) firmware update solution for ESP32.

## ✨ Features (v0.0.1)

*   **🎨 Modern UI/UX**:
    *   Sleek Dark Theme with glassmorphism elements.
    *   Responsive design (Mobile & Desktop friendly).
    *   Smooth animations and transition effects.
    *   **Dashboard Grid** for quick status monitoring (Version, Uptime).
*   **🔒 Enhanced Security**:
    *   **Secure Login System**: specific UI for authentication.
    *   Protected API Endpoints (`/update`, `/status`, `/ota-progress`).
    *   Auto-logout on unauthorized access.
*   **⚡ Smart Interactions**:
    *   **Real-time Progress Bar**: Visual feedback during upload.
    *   **UI Lock-out**: Prevents user interference during the update process.
    *   **Input Validation**: Checks file types (.bin) and sizes before upload.
*   **🛠️ Developer Friendly**:
    *   Easy integration with existing ESP32 projects.
    *   Configuration via `ESP32WebOTAConfig.h`.

## 📦 Improvements in v0.0.1

This initial release focuses on revamping the traditional OTA experience:

1.  **Grid Dashboard**: Remade the status card into a uniform grid layout for better readability.
2.  **Fixed Layouts**: Solved layout shifts when switching between "Manual Upload" and "URL Update" tabs.
3.  **Visual Polish**: Added custom styling for file inputs, buttons, and status badges.
4.  **Code Refactoring**: Cleaned up `app.js` and `style.css` for better maintainability.

## 🚀 Getting Started

1.  **Dependencies**: Ensure your `platformio.ini` is set up for ESP32.
2.  **Upload Filesystem**: You **MUST** upload the files in `data/` to SPIFFS/LittleFS for the web UI to work.
    ```bash
    pio run --target uploadfs
    ```
3.  **Include & Initialize**:
    ```cpp
    #include <ESP32WebOTA.h>

    void setup() {
      // ... wifi setup ...
      ota.begin(); 
    }

    }
    ```

    (No `ota.handle()` required in `loop()` as it runs asynchronously)

## 📸 Screenshots

*(Add screenshots of the Login screen and Dashboard here)*

## 📄 License

MIT License
