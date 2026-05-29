/**
 * Project: 05_cloud_hub
 * Description: 4-Channel Relay IoT Controller via MQTT Protocol
 * Author: Henri
 */

 #include <Arduino.h>
 #include <WiFi.h>
 #include <PubSubClient.h> // 导入刚刚配置的 MQTT 库
 
// ==========================================
// 【用户配置区】已填入你提供的真实 Wi-Fi 信息
const char* ssid     = "CU_9cdu";
const char* password = "wzmbm5cf";

// 使用公共免签名的免费 MQTT 服务器（由中科院/EMQX提供，非常适合实验）
 const char* mqtt_server = "broker.emqx.io";
 const int mqtt_port     = 1883;
 // ==========================================
 
 // 锁定最稳定的 HIGH 触发模式
 #define RELAY_TRIGGER_MODE    HIGH 
 
 #if (RELAY_TRIGGER_MODE == LOW)
     const int RELAY_ON  = LOW;
     const int RELAY_OFF = HIGH;
 #else
     const int RELAY_ON  = HIGH;
     const int RELAY_OFF = LOW;
 #endif
 
 // 硬件引脚
 const int relayPins[4] = {2, 4, 16, 17};
 
 WiFiClient espClient;
 PubSubClient client(espClient);
 
 // 定义 MQTT 主题（Topic）
 // 为了防止在公共服务器上和别人冲突，我们加上你的名字 Henri 作为前缀
 const char* topic_sub = "henri/hub/control"; // 接收控制指令的主题
 const char* topic_pub = "henri/hub/status";  // 上报状态的主题
 
 // 联网初始化
 void setup_wifi() {
     delay(10);
     Serial.println();
     Serial.print("Connecting to ");
     Serial.println(ssid);
 
     WiFi.begin(ssid, password);
 
     while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial.print(".");
     }
 
     Serial.println("");
     Serial.println("WiFi connected");
     Serial.print("IP address: ");
     Serial.println(WiFi.localIP());
 }
 
 // 核心：收到外部 MQTT 消息时的回调函数（相当于局域网版的路由解析）
 void callback(char* topic, byte* payload, unsigned int length) {
     Serial.print("Message arrived [");
     Serial.print(topic);
     Serial.print("] ");
     
     String message = "";
     for (unsigned int i = 0; i < length; i++) {
         message += (char)payload[i];
     }
     Serial.println(message);
 
     // 期待收到的消息格式如: "0:ON", "1:OFF", "2:ON"
     if (message.length() >= 4 && message.charAt(1) == ':') {
         int ch = message.substring(0, 1).toInt(); // 提取通道号 (0-3)
         String action = message.substring(2);     // 提取动作 "ON" 或 "OFF"
         
         if (ch >= 0 && ch < 4) {
             if (action == "ON") {
                 digitalWrite(relayPins[ch], RELAY_ON);
                 Serial.printf("Channel %d -> TURN ON\n", ch);
                 client.publish(topic_pub, ("Channel " + String(ch) + " is ON").c_str());
             } else if (action == "OFF") {
                 digitalWrite(relayPins[ch], RELAY_OFF);
                 Serial.printf("Channel %d -> TURN OFF\n", ch);
                 client.publish(topic_pub, ("Channel " + String(ch) + " is OFF").c_str());
             }
         }
     }
 }
 
 // 核心：断线重连机制
 void reconnect() {
     while (!client.connected()) {
         Serial.print("Attempting MQTT connection...");
         // 随机生成一个客户端ID防止冲突
         String clientId = "ESP32Client-Henri-" + String(random(0, 0xffff), HEX);
         
         if (client.connect(clientId.c_str())) {
             Serial.println("connected");
             // 连接成功后，立刻重新订阅控制主题
             client.subscribe(topic_sub);
             // 顺便往云端报个平安
             client.publish(topic_pub, "Henri Hub Online!");
         } else {
             Serial.print("failed, rc=");
             Serial.print(client.state());
             Serial.println(" try again in 5 seconds");
             delay(5000);
         }
     }
 }
 
 void setup() {
     Serial.begin(115200);
     
     // 初始化引脚
     for (int i = 0; i < 4; i++) {
         pinMode(relayPins[i], OUTPUT);
         digitalWrite(relayPins[i], RELAY_OFF);
     }
 
     setup_wifi();
     
     // 配置 MQTT 服务器和回调
     client.setServer(mqtt_server, mqtt_port);
     client.setCallback(callback);
 }
 
 void loop() {
     // 保持 MQTT 心跳，如果没有连接则自动重连
     if (!client.connected()) {
         reconnect();
     }
     client.loop(); // 让底层库去处理保活和接收缓冲
 }