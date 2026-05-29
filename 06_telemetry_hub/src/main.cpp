/**
 * @file main.cpp
 * @brief Real-time ultrasonic telemetry hub for industrial level monitoring.
 * @author Henri
 * @date 2026-05-29
 */

 #include <Arduino.h>
 #include <WiFi.h>
 #include <PubSubClient.h>
 
 // --- Hardware Pin Configurations ---
 // Utilizing distinct GPIO pins on the safe side of the breadboard to prevent channel interference
 #define PIN_TRIG 18
 #define PIN_ECHO 19
 
 // --- Network & Gateway Infrastructures ---
 const char* ssid     = "CU_9cdu";
 const char* password = "wzmbm5cf";
 const char* mqtt_broker  = "broker.emqx.io";
 const int   mqtt_port    = 1883;
 
 // --- MQTT Telemetry Pipelines ---
 const char* topic_telemetry = "henri/hub/telemetry";
 
 WiFiClient espClient;
 PubSubClient client(espClient);
 
 // --- Asynchronous Execution Controls ---
 unsigned long lastTelemetryTime = 0;
 const unsigned long telemetryInterval = 2000; // Sample and transmit data every 2000 milliseconds
 
 /**
  * @brief Captures the time-of-flight of the acoustic wave and converts it to physical distance.
  * @return float Calculated distance in centimeters (cm). Returns -1.0 on signal timeout.
  */
 float getUltrasonicDistance() {
     // Generate a precise 10-microsecond active-HIGH trigger pulse to initiate transducer burst
     digitalWrite(PIN_TRIG, LOW);
     delayMicroseconds(2);
     digitalWrite(PIN_TRIG, HIGH);
     delayMicroseconds(10);
     digitalWrite(PIN_TRIG, LOW);
 
     // Measure the duration of the HIGH pulse returned on the Echo pin (max 30ms block timeout)
     long duration = pulseIn(PIN_ECHO, HIGH, 30000); 
 
     // Error Handling: Hardware sensor fault or distance out of physical boundary
     if (duration == 0) {
         return -1.0; 
     }
 
     // Mathematical Formulation: Distance = (Time * Acoustic Velocity [~340m/s]) / 2 (round-trip factor)
     // Simplified scaling constant: Distance (cm) = Duration (us) / 58.2
     return (float)duration / 58.2;
 }
 
 /**
  * @brief Blocks and attempts connection/reconnection to the cloud MQTT Broker gateway.
  */
 void reconnect() {
     while (!client.connected()) {
         Serial.print("Attempting cloud MQTT handshake...");
         
         // Dynamic client ID allocation to avoid broker identity collisions
         String clientId = "ESP32-Telemetry-Client-";
         clientId += String(random(0, 0xffff), HEX);
         
         if (client.connect(clientId.c_str())) {
             Serial.println("Connected to cloud broker successfully.");
         } else {
             Serial.print("Handshake failed, state return code = ");
             Serial.print(client.state());
             Serial.println(" - Retrying execution loop in 5 seconds...");
             delay(5000);
         }
     }
 }
 
 void setup() {
     // Initialize high-speed UART hardware communication for local diagnostics
     Serial.begin(115200);
     
     // Register GPIO directional constraints
     pinMode(PIN_TRIG, OUTPUT);
     pinMode(PIN_ECHO, INPUT);
 
     // Bootstrap network handshake with the local wireless infrastructure
     Serial.print("Connecting to local Network: ");
     WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial.print(".");
     }
     Serial.println("\nWi-Fi interface up and running.");
 
     // Assign broker destinations to the underlying network transport layer
     client.setServer(mqtt_broker, mqtt_port);
 }
 
 void loop() {
     // Structural state invariant preservation: ensure connection integrity before polling
     if (!client.connected()) {
         reconnect();
     }
     client.loop();
 
     // Non-blocking asynchronous telemetry task scheduling via system uptime polling
     unsigned long currentMillis = millis();
     if (currentMillis - lastTelemetryTime >= telemetryInterval) {
         lastTelemetryTime = currentMillis;
 
         float distance = getUltrasonicDistance();
         
         if (distance >= 0) {
             // Local runtime instrumentation for desktop diagnostic tracking
             Serial.printf("[Debug Log] Metric Calculated: %.1f cm\n", distance);
 
             // Serialization Strategy: Downgrade object structure to a raw numeric string
             // to fulfill strict primitive telemetry parsing constraints on low-tier mobile UIs
             String pureNumberPayload = String(distance, 1); 
 
             // Dispatch serialized primitive telemetry load over the network pipeline
             Serial.printf("Transmitting telemetry uplink to [%s]: %s\n", topic_telemetry, pureNumberPayload.c_str());
             client.publish(topic_telemetry, pureNumberPayload.c_str());
         } else {
             Serial.println("Hardware Warning: Ultrasonic echo timeout. Discarding telemetry epoch.");
         }
     }
 }