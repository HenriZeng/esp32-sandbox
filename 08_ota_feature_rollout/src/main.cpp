#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <PubSubClient.h>

// ============================================================================
// 🚀 DEPLOYMENT VERSION MANAGER
// ============================================================================
const char* CURRENT_VERSION = "1.0.0"; 

// Dynamic Rhythm Profiles
unsigned long rhythmInterval = (strcmp(CURRENT_VERSION, "1.0.0") == 0) ? 2000 : 400;
unsigned long beepDuration   = (strcmp(CURRENT_VERSION, "1.0.0") == 0) ? 300  : 150;

// ============================================================================
// 🎛️ HARDWARE PERIPHERAL CONFIGURATION
// ============================================================================
const int STATUS_LED  = 2;  
const int BUZZER_PIN  = 18; 
const int BOOT_BUTTON = 0;  

// ============================================================================
// 🌐 NETWORK & CLOUD GATEWAY PARAMETERS
// ============================================================================
const char* ssid         = "CU_9cdu"; 
const char* password     = "wzmbm5cf";
const char* mqtt_broker  = "broker.emqx.io";
const int   mqtt_port    = 1883;
const char* ota_cmd_topic = "henri/ota/cmd";

const String MANUAL_OTA_URL = "http://192.168.1.4:8000/firmware.bin?v=button_click";

WiFiClient espClient;
PubSubClient client(espClient);

// ============================================================================
// ⏳ COOPERATIVE MULTI-TASKING TIMING REGISTERS
// ============================================================================
bool isUpdating = false;             
bool buzzerState = false;            
unsigned long lastStateChange = 0;   

// ============================================================================
// 📡 LOCAL NETWORK INFRASTRUCTURE (Wi-Fi)
// ============================================================================
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("[WiFi] Linking up with network: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        // ✨【修正】：连接网络期间，强制输出 LOW 保持静音
        digitalWrite(BUZZER_PIN, LOW); 
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("[WiFi] Network interface established.");
    Serial.print("[WiFi] IP Lease: ");
    Serial.println(WiFi.localIP());
}

// ============================================================================
// 📦 MQTT INTERFACE & BACKDOOR OTA TRIGGER
// ============================================================================
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("[MQTT] Message arrived on topic [");
    Serial.print(topic);
    Serial.print("] ");
    
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);

    if (String(topic) == ota_cmd_topic && !isUpdating) {
        Serial.println("[MQTT] Remote OTA Command verified. Route URL parsing...");
        isUpdating = true;
        
        // ✨【修正】：升级前强制输出 LOW 闭嘴
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(STATUS_LED, LOW);
    }
}

void reconnect_mqtt() {
    while (!client.connected()) {
        Serial.print("[MQTT] Polling cloud broker connection...");
        String clientId = "ESP32Client-AlertEngine-";
        clientId += String(random(0, 0xffff), HEX);
        
        if (client.connect(clientId.c_str())) {
            Serial.println("connected.");
            client.subscribe(ota_cmd_topic);
            Serial.printf("[MQTT] Telemetry hook armed on target topic: %s\n", ota_cmd_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" | Retry executing in 5 seconds...");
            
            unsigned long startWait = millis();
            while(millis() - startWait < 5000) {
                // ✨【修正】：断网等待期间维持 LOW 静音
                digitalWrite(BUZZER_PIN, LOW);
            }
        }
    }
}

// ============================================================================
// ⚡ HTTP AUTOMATED OTA RUNTIME CORE
// ============================================================================
void perform_http_ota(String url) {
    HTTPClient http;
    Serial.printf("[OTA] Initiating binary stream fetch from: %s\n", url.c_str());
    
    // ✨【修正】：下载前强制输出 LOW 闭嘴
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(STATUS_LED, LOW);
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        Serial.printf("[OTA] Payload validated. Size: %d bytes. Allocation flash sector...\n", contentLength);
        
        bool canBegin = Update.begin(contentLength);
        if (canBegin) {
            WiFiClient* stream = http.getStreamPtr();
            size_t written = Update.writeStream(*stream);
            
            if (written == contentLength) {
                Serial.printf("[OTA] Flash written completed: %d bytes successfully.\n", written);
            } else {
                Serial.printf("[OTA] Flash write mismatch error. Only: %d/%d\n", written, contentLength);
            }
            
            if (Update.end()) {
                Serial.println("[OTA] Update fully structural parsed!");
                if (Update.isFinished()) {
                    Serial.println("[System] Success! Executing software warm reset in 1 second...");
                    delay(1000);
                    ESP.restart();
                } else {
                    Serial.println("[OTA] Critical Error: Update flagged not finished.");
                }
            } else {
                Serial.printf("[OTA] Error Occurred. Error #: %s\n", Update.errorString());
            }
        } else {
            Serial.println("[OTA] Error: Insufficient space inside localized partition.");
        }
    } else {
        Serial.printf("[OTA] HTTP execution channel failed, error code: %d\n", httpCode);
    }
    http.end();
    isUpdating = false; 
}

// ============================================================================
// 🛠️ SYSTEM KERNEL ENTRYPOINT
// ============================================================================
void setup() {
    Serial.begin(115200);

    pinMode(STATUS_LED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BOOT_BUTTON, INPUT_PULLUP); 

    // ✨【修正】：开机初状态立即锁定为 LOW（实现真正的初始静音）
    digitalWrite(STATUS_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW); 

    Serial.println("\n=================================================");
    Serial.printf("  ESP32 Adjusted Alert Engine - Profile: v%s\n", CURRENT_VERSION);
    Serial.println("=================================================");

    setup_wifi();
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(mqtt_callback);
}

// ============================================================================
// 🔁 CORE NON-BLOCKING COOPERATIVE LOOP
// ============================================================================
void loop() {
    if (!isUpdating) {
        if (!client.connected()) {
            reconnect_mqtt();
        }
        client.loop(); 
    }

    unsigned long currentMillis = millis(); 

    if (!isUpdating) { 
        
        // --- A. 强力按键扫描 ---
        if (digitalRead(BOOT_BUTTON) == LOW) {
            Serial.println("\n[Hardware] BOOT Low Level Caught! Breaking loop for OTA...");
            isUpdating = true; 
            
            // ✨【修正】：强制闭嘴
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(STATUS_LED, LOW);
            
            perform_http_ota(MANUAL_OTA_URL);
        }

        // --- B. 纯异步双状态解耦蜂鸣器驱动（逻辑反转版） ---
        if (!buzzerState) {
            // 【状态 A：目前是安静的】 检查是否已经安静够了时间？
            if (currentMillis - lastStateChange >= rhythmInterval) {
                digitalWrite(STATUS_LED, HIGH);  
                digitalWrite(BUZZER_PIN, HIGH);  // ✨【修正】：高电平触发，给 HIGH 开始响！
                
                lastStateChange = currentMillis; 
                buzzerState = true;              
                
                Serial.printf("[Machine] Switch to BEEPING. Version: v%s\n", CURRENT_VERSION);
            }
        } 
        else {
            // 【状态 B：目前正在鸣叫】 检查是否已经叫够了规定的短促时间？
            if (currentMillis - lastStateChange >= beepDuration) {
                digitalWrite(STATUS_LED, LOW);   
                digitalWrite(BUZZER_PIN, LOW);   // ✨【修正】：高电平触发，给 LOW 彻底闭嘴！
                
                lastStateChange = currentMillis; 
                buzzerState = false;             
                
                Serial.println("[Machine] Switch to SILENT.");
            }
        }
    }
}