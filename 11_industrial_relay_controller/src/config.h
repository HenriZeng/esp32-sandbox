#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Hardware Topology ---
// Ensure these match your actual wiring
static const uint8_t RELAY_PINS[4] = {18, 19, 21, 22};
#define CHANNEL_COUNT 4

// --- Timing Parameters (ms) ---
#define STEP_INTERVAL_MS 500
#define WDT_TIMEOUT_SECONDS 5

// --- System States ---
enum class SystemState { 
    BOOT_WAIT, 
    SEQUENCING, 
    OPERATIONAL, 
    EMERGENCY_SHUTDOWN 
};

#endif