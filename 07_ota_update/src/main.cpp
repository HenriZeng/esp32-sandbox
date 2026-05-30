/**
 * Project: 07_ota_update
 * Description: Secure HTTP OTA Firmware Update via MQTT Trigger
 * Environment: PlatformIO + ESP32 Arduino Core
 * Author: Henri
 */

 #include <WiFi.h>
 #include <PubSubClient.h>
 #include <HTTPClient.h>
 #include <Update.h>
 
 // Firmware version tracking for deployment validation
 const char* CURRENT_VERSION = "4.0.0";
 
 // Hardware Pin Configuration: Reuse on-board LED for status and business logic
 const int STATUS_LED = 2;
 
 // 1. Network & Protocol Configurations
 const char* ssid     = "Henri's iPhone";
 const char* password = "123456789";
 
 // Public MQTT broker configuration (EMQX public sandbox)
 const char* mqtt_broker   = "broker.emqx.io"; 
 const int   mqtt_port     = 1883;
 const char* ota_cmd_topic = "henri/ota/cmd";   // Isolated topic to prevent public collision
 
 WiFiClient espClient;
 PubSubClient client(espClient);
 
 // Timing variables for non-blocking business loop execution
 unsigned long lastBlinkTime = 0;   
 const long blinkInterval = 500;   // Toggle LED interval in milliseconds
 bool isUpdating = false;           // Semaphore flag to lock GPIO access during flash write
 
 // 2. Wi-Fi Station Initialization
 void setup_wifi() {
     delay(10);
     Serial.println("\n[WiFi] Initializing connection to SSID: CU_9cdu...");
     WiFi.begin(ssid, password);
 
     // Toggle LED during handshake phase to indicate connection progress
     while (WiFi.status() != WL_CONNECTED) {
         digitalWrite(STATUS_LED, !digitalRead(STATUS_LED)); 
         delay(500);
         Serial.print(".");
     }
     
     digitalWrite(STATUS_LED, LOW); 
     Serial.println("\n[WiFi] Network connection established successfully.");
     Serial.print("[WiFi] Assigned Local IP Address: ");
     Serial.println(WiFi.localIP());
 }
 
 // 3. Core HTTP OTA Engine (Robust chunk-based binary flashing)
 void perform_http_ota(const String& url) {
     isUpdating = true; // Acquire GPIO semaphore lock to intercept business logic
     Serial.println("\n[OTA] Interrupted by remote trigger. Executing OTA update sequence...");
     Serial.print("[OTA] Target Firmware URL: ");
     Serial.println(url);
 
     HTTPClient http;
     http.begin(espClient, url); 
     
     int httpCode = http.GET();
     if (httpCode != HTTP_CODE_OK) {
         Serial.printf("[OTA] CRITICAL ERROR: HTTP GET request failed. Status Code: %d\n", httpCode);
         http.end();
         isUpdating = false;
         return;
     }
 
     int contentLength = http.getSize();
     Serial.printf("[OTA] Inbound firmware metadata resolved. Payload Size: %d bytes\n", contentLength);
 
     if (contentLength <= 0) {
         Serial.println("[OTA] CRITICAL ERROR: Invalid payload boundary condition.");
         http.end();
         isUpdating = false;
         return;
     }
 
     // Allocate flash execution partitions and parse application boundaries
     bool canBegin = Update.begin(contentLength);
     if (!canBegin) {
         Serial.println("[OTA] CRITICAL ERROR: Storage allocation failure. Insufficient partition space.");
         Update.printError(Serial);
         http.end();
         isUpdating = false;
         return;
     }
 
     Serial.println("[OTA] Partition maps initialized. Processing binary stream...");
     digitalWrite(STATUS_LED, HIGH); // Hold LED high to serve as a hardware write indicator
 
     // Robust chunk-based stream production pipeline
     WiFiClient* stream = http.getStreamPtr();
     size_t written = 0;
     const size_t bufferSize = 1024;  // Stable 1KB data block memory window
     uint8_t buffer[bufferSize];
 
     // Stream ingestion: explicitly manage packet windows to prevent socket starvation
     while (http.connected() && (written < (size_t)contentLength)) {
         size_t availableSize = stream->available();
         if (availableSize > 0) {
             // Read either the maximum buffer capacity or remaining slice of payload
             size_t bytesToRead = min(availableSize, bufferSize);
             bytesToRead = min(bytesToRead, (size_t)(contentLength - written));
             
             int bytesRead = stream->readBytes(buffer, bytesToRead);
             if (bytesRead > 0) {
                 // Pipe the explicitly buffered chunk straight into storage blocks
                 Update.write(buffer, bytesRead);
                 written += bytesRead;
             }
         }
         delay(1); // Yield execution execution back to RTOS idle task to feed system watchdogs
     }
 
     if (written == (size_t)contentLength) {
         Serial.printf("[OTA] Stream ingestion complete. Flashed %zu bytes to sectors.\n", written);
     } else {
         Serial.printf("[OTA] RUNTIME ERROR: Byte count verification failed (%zu/%d).\n", written, contentLength);
     }
 
     // Conclude download transaction and perform checksum/header validation
     if (Update.end()) {
         if (Update.isFinished()) {
             Serial.println("[OTA] SUCCESS: Firmware signature verified. Performing hardware reset...");
             delay(1000);
             ESP.restart(); // Software fallback to load new boot vector from alternate app slot
         } else {
             Serial.println("[OTA] CRITICAL ERROR: Post-download integrity verification failed.");
         }
     } else {
         Serial.print("[OTA] CRITICAL ERROR: Post-write transaction termination failed. Code: ");
         Update.printError(Serial);
     }
 
     digitalWrite(STATUS_LED, LOW);
     http.end();
     isUpdating = false; // Release semaphore on transaction failure
 }
 
 // 4. MQTT Inbound Message Pipeline Handler
 void mqtt_callback(char* topic, byte* payload, unsigned int length) {
     Serial.print("\n[MQTT] Event dispatched on ingest topic: ");
     Serial.println(topic);
 
     // Reconstruct raw packet segment into structured primitive string URL
     String ota_url = "";
     for (unsigned int i = 0; i < length; i++) {
         ota_url += (char)payload[i];
     }
     ota_url.trim();
 
     // Sanitize input vector for strict HTTP protocol prefixes
     if (ota_url.startsWith("http")) {
         perform_http_ota(ota_url);
     } else {
         Serial.println("[MQTT] Validation rejected. Payload does not conform to valid URI schema.");
     }
 }
 
 // 5. MQTT Session Broker Reconnection State Machine
 void reconnect_mqtt() {
     while (!client.connected()) {
         Serial.print("[MQTT] Polling broker connection status...");
         
         // Dynamic client identification sequence to isolate public sandboxes
         String clientId = "ESP32Client-Henri-" + String(random(0, 0xffff), HEX);
         
         if (client.connect(clientId.c_str())) {
             Serial.println("connected.");
             client.subscribe(ota_cmd_topic);
             Serial.printf("[MQTT] Subscribed to telemetry target channel: %s\n", ota_cmd_topic);
         } else {
             Serial.print("rejected, state code=");
             Serial.print(client.state());
             Serial.println(" -> Retrying state connection loop in 5000ms");
             delay(5000);
         }
     }
 }
 
 void setup() {
     Serial.begin(115200);
     pinMode(STATUS_LED, OUTPUT);
     digitalWrite(STATUS_LED, LOW);
 
     Serial.printf("\n================================================\n");
     Serial.printf("  ESP32 Firmware Sandbox Architecture - v%s\n", CURRENT_VERSION);
     Serial.printf("================================================\n");
 
     setup_wifi();
     client.setServer(mqtt_broker, mqtt_port);
     client.setCallback(mqtt_callback);
 }
 
 void loop() {
     // Keepalive management and background protocol stack servicing
     if (!client.connected()) {
         reconnect_mqtt();
     }
     client.loop(); 
 
     // ----------------- Non-Blocking Business Logic -----------------
     // Execute default application tasks only when flash pipeline is unlocked
     if (!isUpdating) { 
         unsigned long currentMillis = millis(); 
         
         // Evaluate periodic window boundary conditions
         if (currentMillis - lastBlinkTime >= blinkInterval) {
             lastBlinkTime = currentMillis; 
             
             // Toggle IO pin latch state
             int ledState = digitalRead(STATUS_LED);
             digitalWrite(STATUS_LED, !ledState);
             
             Serial.printf("[Business] Periodic beacon toggled. Current Active Image: %s\n", CURRENT_VERSION);
         }
     }
     // -----------------------------------------------------------------
 }