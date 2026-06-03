#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Update.h>

// ============================================================================
// 1. VERSION CONTROL & HARDWARE TOPOLOGY DEFINITIONS
// ============================================================================
#define FIRMWARE_VERSION    3      // Release Branch: 1=Birthday, 2=TwoTigers, 3=OdeToJoy
#define STATUS_LED          2      // Built-in status LED for visual telemetry
#define BOOT_BUTTON         0      // GPIO0 mapped to the physical BOOT switch
#define PASSIVE_BUZZER_PIN   18    // PWM Channel output for the Active-Low Passive Buzzer
#define PASSIVE_LEDC_CH      0     // LEDC PWM timer channel assignment

#define ACTIVE_BUZZER_PIN    19    // Direct GPIO drive for the Active Buzzer (Hardware Alert)

// Network Layer Credentials
const char* ssid     = "CU_9cdu"; 
const char* password = "wzmbm5cf";

// MQTT Control Plane Telemetry Configurations
const char* mqtt_broker    = "broker.emqx.io";
const int   mqtt_port      = 1883;
const char* ota_cmd_topic  = "henri/ota/cmd";
const char* default_ota_url = "http://192.168.1.4:8000/firmware.bin";

WiFiClient espClient;
PubSubClient client(espClient);

// ============================================================================
// 2. AUDIO ENGINE & ACOUSTIC ACOUSTIC DEFINITIONS (PASSIVE BUZZER CALIBRATION)
// ============================================================================
// Transposed Musical Pitch Frequencies (Optimized Golden Frequency Band to eliminate high-pitch harshness)
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

// Temporal Controls (Tempo-mapped millisecond durations for snappy, upbeat playback)
#define BEAT_4      380   // Quarter Note (1 Beat)
#define BEAT_8      190   // Eighth Note  (1/2 Beat)
#define BEAT_2      760   // Half Note    (2 Beats)

/**
 * @brief Triggers a hardware-level alarm using the standalone Active Buzzer.
 * @note This uses raw DC driving since the active buzzer has an internal oscillator.
 * @param count Number of distinct beep pulses.
 * @param duration_ms Active high hold time per pulse.
 */
void triggerActiveAlarm(int count, int duration_ms) {
    for (int i = 0; i < count; i++) {
        digitalWrite(ACTIVE_BUZZER_PIN, HIGH); // Assert DC rail to trigger internal oscillator
        delay(duration_ms);
        digitalWrite(ACTIVE_BUZZER_PIN, LOW);  // Collapse DC rail to mute
        if (count > 1) delay(duration_ms);     // Inter-pulse squelch delay
    }
}

/**
 * @brief Enforces an absolute physical hardware lockdown on the passive buzzer.
 * @details Detaches the PWM peripheral matrix and forces the GPIO pin to a steady HIGH state.
 * This cuts off the PNP transistor driver bias, completely eliminating idling white noise.
 */
void hardPassiveMute() {
    ledcDetachPin(PASSIVE_BUZZER_PIN);          
    pinMode(PASSIVE_BUZZER_PIN, OUTPUT);        
    digitalWrite(PASSIVE_BUZZER_PIN, HIGH);     // Drive HIGH to isolate PNP base and force cut-off
    digitalWrite(STATUS_LED, LOW);
}

/**
 * @brief Advanced Acoustic Smoothing Audio Engine for the Passive Buzzer.
 * @details Implements a Soft-Attack algorithm paired with a smooth linear decay envelope 
 * to transform harsh square waves into pleasant, music-box-like tones.
 * @param frequency Target pitch frequency in Hz.
 * @param duration Active sounding duration in milliseconds.
 */
void playSoftTone(int frequency, int duration) {
    if (frequency == 0) {
        hardPassiveMute();
        delay(duration);
        return;
    }

    digitalWrite(STATUS_LED, HIGH);
    ledcAttachPin(PASSIVE_BUZZER_PIN, PASSIVE_LEDC_CH);
    
    // Phase 1: Soft Attack Mechanism (Suppresses initial transient harshness)
    ledcWriteTone(PASSIVE_LEDC_CH, frequency - 15); // Slight frequency offset for pitch-glide buffering
    ledcWrite(PASSIVE_LEDC_CH, 210);                // Muted initial duty cycle (Low-Level Active)
    delay(10);
    
    ledcWriteTone(PASSIVE_LEDC_CH, frequency);      // Snap back to normalized center frequency
    
    // Phase 2: Dynamic Envelope Profiling
    int startDuty = 195;                            // Peak acoustic energy point (~23% square wave density)
    int endDuty = 252;                              // Target decay noise-floor threshold
    int attackTime = duration * 0.25; 
    int decayTime  = duration * 0.75; 
    
    ledcWrite(PASSIVE_LEDC_CH, startDuty); 
    delay(attackTime);
    
    // Phase 3: Linear Decay Slew Rate Interpolation
    int steps = 10; 
    int stepTime = decayTime / steps;
    for (int i = 0; i < steps; i++) {
        // Stepwise increments towards 255 effectively dampens the PNP active drive current
        int duty = startDuty + ((endDuty - startDuty) * i / steps);
        ledcWrite(PASSIVE_LEDC_CH, duty);
        delay(stepTime);
    }
    
    // Phase 4: Staccato Inter-note Release
    ledcWrite(PASSIVE_LEDC_CH, 255);                // Hard clamp to VCC for clean separation
    delay(15); 
    
    digitalWrite(STATUS_LED, LOW);
    delay(40);                                      // Minimal acoustic breathing window for snappy rhythm
}

// ============================================================================
// 3. MELODY SUB-ROUTINES (SCORE ARRAYS)
// ============================================================================
void playHappyBirthdayMelody() {
    Serial.println("[AUDIO] Executing Playback: Happy Birthday (Upbeat Soft)...");
    playSoftTone(NOTE_G4, BEAT_8); playSoftTone(NOTE_G4, BEAT_8);
    playSoftTone(NOTE_A4, BEAT_4); playSoftTone(NOTE_G4, BEAT_4);
    playSoftTone(NOTE_C5, BEAT_4); playSoftTone(NOTE_B4, BEAT_2);
    delay(100); 
    playSoftTone(NOTE_G4, BEAT_8); playSoftTone(NOTE_G4, BEAT_8);
    playSoftTone(NOTE_A4, BEAT_4); playSoftTone(NOTE_G4, BEAT_4);
    playSoftTone(NOTE_D5, BEAT_4); playSoftTone(NOTE_C5, BEAT_2);
    delay(100);
    hardPassiveMute();
}

void playTwoTigersMelody() {
    Serial.println("[AUDIO] Executing Playback: Two Tigers...");
    for(int i=0; i<2; i++) {
        playSoftTone(NOTE_G4, BEAT_4); playSoftTone(NOTE_A4, BEAT_4);
        playSoftTone(NOTE_B4, BEAT_4); playSoftTone(NOTE_G4, BEAT_4);
    }
    delay(100);
    for(int i=0; i<2; i++) {
        playSoftTone(NOTE_B4, BEAT_4); playSoftTone(NOTE_C5, BEAT_4);
        playSoftTone(NOTE_D5, BEAT_2);
    }
    delay(100);
    hardPassiveMute();
}

void playOdeToJoyMelody() {
    Serial.println("[AUDIO] Executing Playback: Beethoven's Ode to Joy (Premium Master Edition)...");
    
    // Phrase 1
    playSoftTone(NOTE_B4, BEAT_4); playSoftTone(NOTE_B4, BEAT_4);
    playSoftTone(NOTE_C5, BEAT_4); playSoftTone(NOTE_D5, BEAT_4);
    playSoftTone(NOTE_D5, BEAT_4); playSoftTone(NOTE_C5, BEAT_4);
    playSoftTone(NOTE_B4, BEAT_4); playSoftTone(NOTE_A4, BEAT_4);
    
    // Phrase 2
    playSoftTone(NOTE_G4, BEAT_4); playSoftTone(NOTE_G4, BEAT_4);
    playSoftTone(NOTE_A4, BEAT_4); playSoftTone(NOTE_B4, BEAT_4);
    playSoftTone(NOTE_B4, (BEAT_4 + BEAT_8)); playSoftTone(NOTE_A4, BEAT_8); // Dotted note
    playSoftTone(NOTE_A4, BEAT_2); delay(80); 
    
    // Phrase 3
    playSoftTone(NOTE_B4, BEAT_4); playSoftTone(NOTE_B4, BEAT_4);
    playSoftTone(NOTE_C5, BEAT_4); playSoftTone(NOTE_D5, BEAT_4);
    playSoftTone(NOTE_D5, BEAT_4); playSoftTone(NOTE_C5, BEAT_4);
    playSoftTone(NOTE_B4, BEAT_4); playSoftTone(NOTE_A4, BEAT_4);
    
    // Phrase 4
    playSoftTone(NOTE_G4, BEAT_4); playSoftTone(NOTE_G4, BEAT_4);
    playSoftTone(NOTE_A4, BEAT_4); playSoftTone(NOTE_B4, BEAT_4);
    playSoftTone(NOTE_A4, (BEAT_4 + BEAT_8)); playSoftTone(NOTE_G4, BEAT_8); 
    playSoftTone(NOTE_G4, BEAT_2);
    
    hardPassiveMute();
}

void triggerSystemBeep(int freq, int duration_ms, int count) {
    for (int i = 0; i < count; i++) {
        playSoftTone(freq, duration_ms);
    }
    hardPassiveMute(); 
}

// ============================================================================
// 4. CORE HTTP STREAMING OVER-THE-AIR (OTA) ENGINE
// ============================================================================
void executeOTA(const char* url) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[OTA] [CRITICAL] Aborted: No valid Wi-Fi context.");
        triggerActiveAlarm(3, 300); // Hardware Fault Alert: 3 sharp pulses
        return;
    }
    Serial.printf("[OTA] Initiating transport handshake via URL: %s\n", url);
    triggerSystemBeep(880, 400, 1); // Informative acoustic chime

    HTTPClient http;
    if (!http.begin(url)) {
        triggerActiveAlarm(1, 1000); // Connection Timeout Alert: 1 long pulse
        return;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        triggerActiveAlarm(1, 1000); 
        return;
    }

    int contentLength = http.getSize();
    if (!Update.begin(contentLength, U_FLASH)) {
        http.end();
        triggerActiveAlarm(1, 1000); // Partition allocation failure alert
        return;
    }

    // Stream-processing the binary data chunk-by-chunk to prevent memory fragmentation
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[1024];
    size_t totalBytesWritten = 0;

    while (http.connected() && (totalBytesWritten < contentLength)) {
        size_t availableBytes = stream->available();
        if (availableBytes > 0) {
            int bytesRead = stream->readBytes(buffer, min(availableBytes, sizeof(buffer)));
            size_t bytesWritten = Update.write(buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                Update.abort();
                http.end();
                triggerActiveAlarm(1, 1000); // Write block validation fault
                return;
            }
            totalBytesWritten += bytesWritten;
        }
        delay(1); // Task yielding to prevent watchdog timer (WDT) trips
    }
    http.end();

    if (Update.end(true)) {
        if (Update.isFinished()) {
            Serial.println("[OTA] Integrity verification passed. Executing warm reboot...");
            triggerSystemBeep(1046, 60, 5); // Success chime
            delay(2000);
            ESP.restart();
        }
    } else {
        triggerActiveAlarm(2, 500); // Post-write MD5 checksum validation failure
    }
}

// ============================================================================
// 5. NON-BLOCKING ASYNCHRONOUS NETWORK MONITORING LAYER
// ============================================================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char urlBuffer[256];
    if (length >= sizeof(urlBuffer)) length = sizeof(urlBuffer) - 1;
    memcpy(urlBuffer, payload, length);
    urlBuffer[length] = '\0';

    // Packet parsing: Check if payload contains a valid URL scheme
    if (strncmp(urlBuffer, "http", 4) == 0) {
        executeOTA(urlBuffer);
    }
}

void tryNetworkConnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("\n[NETWORK] Associating with SSID: %s ", ssid);
        WiFi.begin(ssid, password);
        
        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 20) { 
            delay(500);
            Serial.print(".");
            timeout++;
        }
        
        if (WiFi.status() != WL_CONNECTED) {
            triggerActiveAlarm(2, 100); // Link-layer association timeout alert
            return;
        }
    }

    // Process MQTT broker reconnection loop if internet gateway is alive
    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
        client.setServer(mqtt_broker, mqtt_port);
        client.setCallback(mqttCallback);

        String clientId = "ESP32-Henri-DualSpeakers-" + String(random(0, 0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            client.subscribe(ota_cmd_topic);
            Serial.println("\n[MQTT] Session established. Inbound telemetry channel active.");
        } else {
            triggerActiveAlarm(1, 200); // Broker connection handshake failure alert
        }
    }
}

// ============================================================================
// 6. FIRMWARE MAIN ENTRY POINTS
// ============================================================================
unsigned long lastSongTime = 0;
const unsigned long songInterval = 10000; // Recurrence delay between background melodies (10s)

void setup() {
    Serial.begin(115200);
    
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    pinMode(BOOT_BUTTON, INPUT_PULLUP);

    // Hard-init the telemetry peripherals to secure safe power states
    pinMode(ACTIVE_BUZZER_PIN, OUTPUT);
    digitalWrite(ACTIVE_BUZZER_PIN, LOW); 

    ledcSetup(PASSIVE_LEDC_CH, 2000, 8);
    hardPassiveMute(); 

    Serial.println("\n=============================================");
    Serial.printf("[SYSTEM] Version V%d - Dual-Speaker Telemetry Engine Active\n", FIRMWARE_VERSION);
    Serial.println("=============================================");
    
    triggerSystemBeep(523, 60, 3); // POST (Power-On Self Test) audit tone

    tryNetworkConnect();
    lastSongTime = millis();
}

void loop() {
    // Non-blocking Link Layer Keep-Alive Interrogator (Polling interval: 15s)
    static unsigned long lastNetCheck = 0;
    if (millis() - lastNetCheck > 15000) { 
        tryNetworkConnect();
        lastNetCheck = millis();
    }
    
    if (client.connected()) {
        client.loop(); // Yield execution time slice to MQTT background processing
    }

    // Finite State Scheduler for automated acoustic playback
    if (millis() - lastSongTime > songInterval) {
        if (FIRMWARE_VERSION == 1)      playHappyBirthdayMelody(); 
        else if (FIRMWARE_VERSION == 2) playTwoTigersMelody();
        else if (FIRMWARE_VERSION == 3) playOdeToJoyMelody(); 
        lastSongTime = millis();
    }

    // Hardware polling loop for manual local override/OTA trigger
    if (digitalRead(BOOT_BUTTON) == LOW) {
        delay(50); // Software debounce window
        if (digitalRead(BOOT_BUTTON) == LOW) {
            triggerSystemBeep(440, 200, 2); // Explicit execution confirmation chime
            executeOTA(default_ota_url);
        }
    }
    delay(10); // Context switching yield for freeRTOS task balancing
}