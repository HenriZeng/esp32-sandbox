/**
 * ============================================================================
 * @file        main.cpp
 * @brief       Enterprise IoT Edge Agent with Autonomous Lifecycle Management
 * and Dual-Speaker Telemetry Support.
 * @note        Designed for high-resilience environments with zero-downtime
 * declarative OTA scheduling.
 * ============================================================================
 */

 #include <Arduino.h>
 #include <WiFi.h>
 #include <PubSubClient.h>
 #include <HTTPClient.h>
 #include <Update.h>
 
 // ============================================================================
 // 1. HARDWARE TOPOLOGY & FIRMWARE VERSION CONTROL
 // ============================================================================
 #define FIRMWARE_VERSION      2       // Local Semantic Version: V1
 #define STATUS_LED            2       // Built-in status LED for visual telemetry
 #define BOOT_BUTTON           0       // GPIO0 mapped to physical BOOT switch
 #define ACTIVE_BUZZER_PIN    19       // Direct GPIO drive for Active Buzzer
 
 // ============================================================================
 // 2. NETWORK INGRESS/EGRESS CONFIGURATION
 // ============================================================================
 const char* ssid     = "CU_9cdu";     // Target WLAN Service Set Identifier
 const char* password = "wzmbm5cf";       // WLAN Pre-Shared Key (PSK)
 
 // Local Orchestration Infrastructure (Mac Host)
 const char* mqtt_server      = "192.168.1.4"; 
 const int   mqtt_port        = 1883;
 const char* default_ota_url  = "http://192.168.1.4:8000/firmware.bin";
 const char* metadata_json_url = "http://192.168.1.4:8000/version.json";
 
 // ============================================================================
 // 3. GLOBAL OBJECT INSTANTIATION & SCHEDULER TIMERS
 // ============================================================================
 WiFiClient espClient;
 PubSubClient client(espClient);
 
 unsigned long lastNetCheck     = 0;
 unsigned long lastUpdateCheck  = 0;
 const unsigned long cronInterval = 60000; // Autonomous polling window: 60 seconds
 
 // ============================================================================
 // 4. FUNCTION PROTOTYPES
 // ============================================================================
 void tryNetworkConnect();
 void mqttCallback(char* topic, byte* payload, unsigned int length);
 void executeOTA(const char* url);
 void checkForUpdates();
 void triggerSystemBeep(int count, int duration_ms);
 
 // ============================================================================
 // 5. FIRMWARE DOWNSTREAM TRANSPORT ENGINE (OTA)
 // ============================================================================
 void executeOTA(const char* url) {
     Serial.printf("[OTA] Initiating transport handshake via URL: %s\n", url);
     triggerSystemBeep(2, 100); // Pre-deployment warning chime
 
     HTTPClient http;
     http.begin(url);
     int httpCode = http.GET();
 
     if (httpCode == HTTP_CODE_OK) {
         int contentLength = http.getSize();
         bool canBegin = Update.begin(contentLength);
 
         if (canBegin) {
             Serial.println("[OTA] Stream verified. Writing binary payload to flash partition...");
             WiFiClient* stream = http.getStreamPtr();
             size_t written = Update.writeStream(*stream);
 
             if (written == contentLength) {
                 Serial.printf("[OTA] Successfully flashed %d bytes.\n", written);
             } else {
                 Serial.printf("[OTA] Flash mismatch error. Wrote %d/%d bytes.\n", written, contentLength);
             }
 
             if (Update.end()) {
                 if (Update.isFinished()) {
                     Serial.println("[OTA] Integrity verification passed. Executing warm reboot...");
                     triggerSystemBeep(5, 50); // Deployment success chime
                     delay(1000);
                     ESP.restart();
                 } else {
                     Serial.println("[OTA] Critical Error: Update declared finished but not verified.");
                 }
             } else {
                 Serial.printf("[OTA] Partition finalization failed. Error Code: %s\n", Update.errorString());
             }
         } else {
             Serial.println("[OTA] Insufficient memory allocation or unsafe partition layouts.");
         }
     } else {
         Serial.printf("[OTA] Transport abstraction layer failed. HTTP Code: %d\n", httpCode);
     }
     http.end();
 }
 
 // ============================================================================
 // 6. AUTONOMOUS METADATA POLLING ENGINE (VERSION VERIFICATION)
 // ============================================================================
 void checkForUpdates() {
     if (WiFi.status() != WL_CONNECTED) return;
 
     HTTPClient http;
     Serial.println("[UPDATE] Querying remote orchestration server for metadata...");
     http.begin(metadata_json_url);
     int httpCode = http.GET();
 
     if (httpCode == HTTP_CODE_OK) {
         String payload = http.getString();
         Serial.printf("[UPDATE] Ingested Metadata Payload: %s\n", payload.c_str());
 
         // Lightweight declarative JSON parsing via substring matching
         int versionIndex = payload.indexOf("\"version\":");
         if (versionIndex != -1) {
             int startPos = versionIndex + 10; 
             while(payload.charAt(startPos) == ' ' || payload.charAt(startPos) == ':') startPos++;
             int endPos = payload.indexOf(",", startPos);
             if (endPos == -1) endPos = payload.indexOf("}", startPos);
             
             String serverVerStr = payload.substring(startPos, endPos);
             serverVerStr.trim();
             int serverVersion = serverVerStr.toInt();
 
             Serial.printf("[UPDATE] Topology Mapping -> Edge Node: V%d | Target Registry: V%d\n", FIRMWARE_VERSION, serverVersion);
 
             // Version Divergence Resolution
             if (serverVersion > FIRMWARE_VERSION) {
                 Serial.println("[UPDATE] Version divergence detected. Spawning background update worker...");
                 
                 int urlIndex = payload.indexOf("\"url\":");
                 int urlStart = payload.indexOf("\"", urlIndex + 6) + 1;
                 int urlEnd = payload.indexOf("\"", urlStart);
                 String downloadUrl = payload.substring(urlStart, urlEnd);
 
                 executeOTA(downloadUrl.c_str());
             } else {
                 Serial.println("[UPDATE] Semantic consensus reached. Edge node is up-to-date.");
             }
         }
     } else {
         Serial.printf("[UPDATE] Polling cycle aborted. Network transport error: %s\n", http.errorToString(httpCode).c_str());
     }
     http.end();
 }
 
 // ============================================================================
 // 7. CONTROL PLANE TELEMETRY INTERFACE (MQTT)
 // ============================================================================
 void mqttCallback(char* topic, byte* payload, unsigned int length) {
     char urlBuffer[256];
     if (length >= sizeof(urlBuffer)) length = sizeof(urlBuffer) - 1;
     memcpy(urlBuffer, payload, length);
     urlBuffer[length] = '\0';
 
     Serial.printf("[MQTT] Inbound message ingested on topic [%s]\n", topic);
 
     // Explicit Route Validation
     if (strncmp(urlBuffer, "http", 4) == 0) {
         executeOTA(urlBuffer);
     }
 }
 
// ============================================================================
// 8. INFRASTRUCTURE KEEP-ALIVE STATE MACHINE
// ============================================================================
void tryNetworkConnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("\n[NETWORK] Associating with SSID: %s ", ssid);
        
        // --- 工业级稳健性抗干扰加固开始 ---
        WiFi.disconnect(true); // 1. 强力清除 Flash 中可能受损的旧连接缓存
        delay(500);
        WiFi.mode(WIFI_STA);   // 2. 锁定客户端模式，彻底关闭隐式 AP 广播
        WiFi.setSleep(false);  // 3. 关键：关闭射频休眠（Modem-sleep），防止丢包导致的握手超时
        // ---------------------------------
        
        WiFi.begin(ssid, password);
        
        int retryCounter = 0;
        while (WiFi.status() != WL_CONNECTED && retryCounter < 60) {
            delay(500);
            Serial.print(".");
            retryCounter++;
            
            // 进阶防护：如果 5 秒内死锁在一半的状态，主动 reset 射频栈重试
            if (retryCounter == 30) {
                Serial.print("[Radio Reset]");
                WiFi.disconnect();
                delay(200);
                WiFi.begin(ssid, password);
            }
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\n[NETWORK] Lease acquired. IPv4 Address: %s\n", WiFi.localIP().toString().c_str());
            digitalWrite(STATUS_LED, HIGH);
        } else {
            Serial.println("\n[NETWORK] Link-layer negotiation timed out.");
            return;
        }
    }

    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
        Serial.print("[MQTT] Establishing session boundary with Mosquitto Broker... ");
        String clientId = "Villas_Edge_Node_" + String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str())) {
            Serial.println("connected.");
            client.subscribe("henri/ota/cmd");
            Serial.println("[MQTT] Subscription matrix initialized. Channel active.");
        } else {
            Serial.printf("failed, rc=%d. Retrying during next execution window.\n", client.state());
        }
    }
}
 
 // ============================================================================
 // 9. HARDWARE ACOUSTIC BLOCK (NON-BLOCKING ALERTS)
 // ============================================================================
 void triggerSystemBeep(int count, int duration_ms) {
     for (int i = 0; i < count; i++) {
         digitalWrite(ACTIVE_BUZZER_PIN, HIGH);
         delay(duration_ms);
         digitalWrite(ACTIVE_BUZZER_PIN, LOW);
         if (i < count - 1) delay(duration_ms);
     }
 }
 
 // ============================================================================
 // 10. KERNEL INITIALIZATION & EXECUTIVE LOOP
 // ============================================================================
 void setup() {
     Serial.begin(115200);
     pinMode(STATUS_LED, OUTPUT);
     pinMode(ACTIVE_BUZZER_PIN, OUTPUT);
     pinMode(BOOT_BUTTON, INPUT_PULLUP);
     
     digitalWrite(STATUS_LED, LOW);
     digitalWrite(ACTIVE_BUZZER_PIN, LOW);
 
     Serial.printf("\n========================================================\n");
     Serial.printf("[SYSTEM] Initialization Complete. Current Version: V%d\n", FIRMWARE_VERSION);
     Serial.printf("========================================================\n");
 
     client.setServer(mqtt_server, mqtt_port);
     client.setCallback(mqttCallback);
 
     // Establish early-stage communication boundary
     tryNetworkConnect();
     
     // Boot-phase deployment verification
     checkForUpdates();
 }
 
 void loop() {
     lastNetCheck = millis();
 
     // Critical Path: Guard control plane connectivity
     if (!client.connected()) {
         tryNetworkConnect();
     }
     client.loop(); // Delegate CPU cycles to internal MQTT buffer slicing
 
     // Cron Task: Asynchronous scheduling for server pulling
     if (millis() - lastUpdateCheck > cronInterval) {
         lastUpdateCheck = millis();
         checkForUpdates();
     }
 
     // Hardware Interface: Local mechanical interrupt override
     if (digitalRead(BOOT_BUTTON) == LOW) {
         delay(50); // Anti-rebound debounce window
         if (digitalRead(BOOT_BUTTON) == LOW) {
             Serial.println("[HARDWARE] Local panic button triggered. Enforcing hardware-fallback update...");
             executeOTA(default_ota_url);
         }
     }
 
     delay(10); // Context-switching padding to preserve RTOS idling profiles
 }