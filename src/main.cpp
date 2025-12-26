#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESP32WebOTA.h>

AsyncWebServer server(80);

void setup()
{
  Serial.begin(115200);
  WiFi.begin("Papa_wifi_2.4G", "9999900000");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected IP address: ");
  Serial.println(WiFi.localIP());
  
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  ESP32WebOTA::boot();
  ESP32WebOTA(server).begin();

  server.begin();
}

void loop() {}
