#include <ESP32WebOTA.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
ESP32WebOTA ota(server);

void setup() {
  Serial.begin(115200);
  WiFi.begin("Papa_wifi_2.4G", "9999900000");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nเชื่อมต่อ WiFi สำเร็จ");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  ota.begin("0.2.0");

  Serial.println("System Ready");
}

void loop() {  }
