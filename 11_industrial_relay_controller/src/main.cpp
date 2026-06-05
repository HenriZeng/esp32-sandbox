#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "RelayManager.h"

// Global Controller Instance
RelayManager relayController;
SystemState systemState = SystemState::BOOT_WAIT;
unsigned long lastUpdate = 0;

void setup() {
    Serial.begin(115200);
    
    // Hardware-first initialization
    relayController.initialize();

    // Configure and start Watchdog
    esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL); 

    systemState = SystemState::SEQUENCING;
    Serial.println("[SYSTEM] Industrial node initialized.");
}

void loop() {
    // Keep-alive: Reset watchdog to prevent system hang under normal operation
    esp_task_wdt_reset();

    // --- Diagnostic Routine: Fault Injection ---
    // If the user types 'f' in the serial terminal, we simulate a fatal deadlock.
    if (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'f') {
            Serial.println("[DIAGNOSTIC] Injecting fatal deadlock...");
            while (true) {
                // System is now unresponsive, WDT will trigger a reset in 5 seconds
                delay(10); 
            }
        }
    }

    switch (systemState) {
        case SystemState::SEQUENCING:
            if (millis() - lastUpdate >= STEP_INTERVAL_MS) {
                relayController.activateNext();
                lastUpdate = millis();
                
                if (relayController.allOperational()) {
                    systemState = SystemState::OPERATIONAL;
                    Serial.println("[SYSTEM] Startup sequence completed.");
                }
            }
            break;

        case SystemState::OPERATIONAL:
            // Standard operational monitoring logic
            break;

        default:
            break;
    }
}