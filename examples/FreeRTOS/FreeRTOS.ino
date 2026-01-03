/**
 * ESP32WebManager - FreeRTOS Multi-Task Example
 *
 * This example demonstrates using ESP32WebManager with
 * multiple FreeRTOS tasks for complex applications.
 *
 * Tasks:
 * - Task 1: Sensor reading every 500ms
 * - Task 2: Data processing every 1s
 * - Task 3: Serial logging every 2s
 * - WebMgrTask: Handled automatically by library
 *
 * Note: ESP32WebManager creates its own task internally,
 * so you don't need to call manager.loop() manually!
 */

#include <ESP32WebManager.h>

// ===== Configuration =====
#define LED_PIN 2
#define SENSOR_PIN 34

// ===== Task Handles =====
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t processTaskHandle = NULL;
TaskHandle_t logTaskHandle = NULL;

// ===== Shared Data (with mutex) =====
SemaphoreHandle_t dataMutex;
volatile int sensorValue = 0;
volatile float processedValue = 0.0;

// ===== Global Objects =====
AsyncWebServer server(80);
DNSServer dns;
ESP32WebManager manager(server, dns);

// ===== Task 1: Sensor Reading =====
void sensorTask(void *pvParameters) {
  Serial.println("[SensorTask] Started on Core " + String(xPortGetCoreID()));

  while (true) {
    // Read sensor
    int raw = analogRead(SENSOR_PIN);

    // Update shared data with mutex
    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      sensorValue = raw;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(500 / portTICK_PERIOD_MS); // 500ms
  }
}

// ===== Task 2: Data Processing =====
void processTask(void *pvParameters) {
  Serial.println("[ProcessTask] Started on Core " + String(xPortGetCoreID()));

  while (true) {
    int localSensor = 0;

    // Read shared data with mutex
    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      localSensor = sensorValue;
      xSemaphoreGive(dataMutex);
    }

    // Process data (example: convert to voltage)
    float voltage = (localSensor / 4095.0) * 3.3;

    // Update processed value
    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      processedValue = voltage;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second
  }
}

// ===== Task 3: Logging =====
void logTask(void *pvParameters) {
  Serial.println("[LogTask] Started on Core " + String(xPortGetCoreID()));

  while (true) {
    int localRaw = 0;
    float localVoltage = 0.0;

    // Read shared data
    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      localRaw = sensorValue;
      localVoltage = processedValue;
      xSemaphoreGive(dataMutex);
    }

    Serial.printf("[Log] Sensor: %d, Voltage: %.2fV, Heap: %d\n", localRaw,
                  localVoltage, ESP.getFreeHeap());

    vTaskDelay(2000 / portTICK_PERIOD_MS); // 2 seconds
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32WebManager + FreeRTOS Example ===");

  // Initialize hardware
  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  // Create mutex for shared data
  dataMutex = xSemaphoreCreateMutex();

  // Configure WebManager
  manager.setHostname("esp32-rtos");
  manager.setOTAAuth("admin", "admin");

  manager.onWiFiConnect([]() {
    Serial.println("✓ WiFi Connected: " + manager.getIP());
    digitalWrite(LED_PIN, HIGH);
  });

  manager.onAPMode([]() {
    Serial.println("⚠ AP Mode Active");
    digitalWrite(LED_PIN, LOW);
  });

  // Add custom API endpoint for sensor data
  server.on("/api/sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
    int raw = 0;
    float voltage = 0.0;

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      raw = sensorValue;
      voltage = processedValue;
      xSemaphoreGive(dataMutex);
    }

    String json =
        "{\"raw\":" + String(raw) + ",\"voltage\":" + String(voltage, 2) + "}";
    request->send(200, "application/json", json);
  });

  // Start WebManager (creates its own task internally)
  manager.begin("1.0.0");

  // ===== Create User Tasks =====

  // Sensor task on Core 0
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 2048, NULL, 1,
                          &sensorTaskHandle,
                          0 // Core 0
  );

  // Process task on Core 1
  xTaskCreatePinnedToCore(processTask, "ProcessTask", 2048, NULL, 1,
                          &processTaskHandle,
                          1 // Core 1
  );

  // Log task on Core 1
  xTaskCreatePinnedToCore(logTask, "LogTask", 2048, NULL, 1, &logTaskHandle,
                          1 // Core 1
  );

  Serial.println("\n=== Tasks Created ===");
  Serial.println("- SensorTask  (Core 0)");
  Serial.println("- ProcessTask (Core 1)");
  Serial.println("- LogTask     (Core 1)");
  Serial.println("- WebMgrTask  (Core 0) [Auto]");
  Serial.println("\nSystem Ready!");
}

// ===== Main Loop =====
void loop() {
  // Main loop is free for other work
  // or can be empty if all work is in tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
