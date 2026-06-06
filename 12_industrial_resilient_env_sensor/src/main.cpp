/**
 * @file main.cpp
 * @brief Industrial-grade environmental monitoring node.
 * * Architecture:
 * - Deterministic Startup: Configures hardware before enabling peripherals.
 * - Non-blocking Execution: State-machine based sensor and network polling.
 * - Watchdog Supervision: Hardware-backed recovery from system deadlocks.
 */

 #include <Arduino.h>
 #include <esp_task_wdt.h>
 #include "config.h"
 #include "SensorManager.h"
 #include "NetworkManager.h"
 
 // Initialize Managers
 SensorManager sensorNode;
 NetworkManager network(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER_IP);
 
 // System state tracking for diagnostic logging
 enum class SystemState { BOOT_WAIT, INITIALIZING, OPERATIONAL, FAULT };
 SystemState currentSystemState = SystemState::BOOT_WAIT;
 
 void setup() {
     Serial.begin(115200);
     Serial.println("\n[SYSTEM] Booting Industrial Node...");
 
     // 1. Initialize Peripherals
     currentSystemState = SystemState::INITIALIZING;
     sensorNode.initialize(25); // GPIO 25 for DHT11
     network.initialize();
 
     // 2. Configure Task Watchdog Timer
     // Reset system if loop hangs for more than WDT_TIMEOUT_SECONDS
     esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
     esp_task_wdt_add(NULL); 
 
     currentSystemState = SystemState::OPERATIONAL;
     Serial.println("[SYSTEM] Node operational.");
 }
 
 void loop() {
     // 1. Service the Watchdog
     esp_task_wdt_reset();
 
     // 2. Non-blocking Task Updates
     sensorNode.update();
     network.update();
 
     // 3. Periodic Telemetry Transmission (5s interval)
     static unsigned long lastTelemetry = 0;
     if (millis() - lastTelemetry >= 5000) {
         lastTelemetry = millis();
         
         // Prepare JSON payload for Home Assistant
         String payload = "{\"temperature\":" + String(sensorNode.getTemperature()) + 
                          ", \"humidity\":" + String(sensorNode.getHumidity()) + "}";
         
         if (network.publish("sensors/industrial/env", payload.c_str())) {
             Serial.println("[TELEMETRY] Data published successfully.");
         }
     }
 
     // 4. Diagnostic Fault Injection
     // Usage: Send 'f' via Serial Monitor to test Watchdog recovery
     if (Serial.available() > 0) {
         if (Serial.read() == 'f') {
             Serial.println("[DIAGNOSTIC] Simulating fatal deadlock...");
             while (true) { /* Force WDT trigger */ }
         }
     }
 }