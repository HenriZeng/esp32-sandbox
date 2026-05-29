/**
 * Project: 04_smart_light (Minimalist Dashboard)
 * Description: 4-Channel Relay Controller with Premium Minimalist UI
 * Author: Henri
 */

 #include <Arduino.h>
 #include <WiFi.h>
 
 #define RELAY_TRIGGER_MODE    HIGH 
 
 #if (RELAY_TRIGGER_MODE == LOW)
     const int RELAY_ON  = LOW;
     const int RELAY_OFF = HIGH;
 #else
     const int RELAY_ON  = HIGH;
     const int RELAY_OFF = LOW;
 #endif
 
 const char *ssid = "Henri-Smart-Hub";
 const char *password = "12345678";
 
 WiFiServer server(80);
 String header;
 
 const int relayPins[4] = {2, 4, 16, 17};
 String relayStates[4] = {"off", "off", "off", "off"};
 String deviceNames[4] = {"Light 1", "Fan 2", "Device 3", "Device 4"};
 
 void setup() {
     Serial.begin(115200);
     for (int i = 0; i < 4; i++) {
         pinMode(relayPins[i], OUTPUT);
         digitalWrite(relayPins[i], RELAY_OFF);
     }
     WiFi.softAP(ssid, password);
     server.begin();
     Serial.println("\n--- Minimalist Smart Hub Ready ---");
 }
 
 void loop() {
     WiFiClient client = server.available();
     if (client) {
         String currentLine = "";
         while (client.connected()) {
             if (client.available()) {
                 char c = client.read();
                 header += c;
                 if (c == '\n') {
                     if (currentLine.length() == 0) {
                         client.println("HTTP/1.1 200 OK");
                         client.println("Content-type:text/html; charset=utf-8");
                         client.println("Connection: close");
                         client.println();
                         
                         // 解析动作
                         for (int i = 0; i < 4; i++) {
                             if (header.indexOf("GET /" + String(i) + "/toggle") >= 0) {
                                 if (relayStates[i] == "off") {
                                     relayStates[i] = "on";
                                     digitalWrite(relayPins[i], RELAY_ON);
                                 } else {
                                     relayStates[i] = "off";
                                     digitalWrite(relayPins[i], RELAY_OFF);
                                 }
                             }
                         }
                         
                         // 发送极简 UI 网页
                         client.println("<!DOCTYPE html><html><head>");
                         client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
                         client.println("<style>");
                         client.println("body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #f4f6f9; margin: 0; display: flex; justify-content: center; align-items: center; min-height: 100vh; }");
                         client.println(".panel { width: 100%; max-width: 360px; padding: 24px; box-sizing: border-box; }");
                         client.println("h2 { color: #1d1d1f; font-size: 24px; font-weight: 600; margin-bottom: 24px; text-align: left; }");
                         client.println(".grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 16px; }");
                         client.println(".btn { background: #ffffff; border: none; border-radius: 16px; padding: 24px 16px; text-align: left; text-decoration: none; box-shadow: 0 4px 12px rgba(0,0,0,0.05); display: flex; flex-direction: column; justify-content: space-between; height: 110px; box-sizing: border-box; transition: all 0.2s ease; -webkit-tap-highlight-color: transparent; }");
                         client.println(".btn:active { transform: scale(0.96); box-shadow: 0 2px 6px rgba(0,0,0,0.05); }");
                         client.println(".btn-on { background: #34c759; }");
                         client.println(".name { font-size: 16px; font-weight: 600; color: #8e8e93; }");
                         client.println(".btn-on .name { color: rgba(255,255,255,0.9); }");
                         client.println(".state { font-size: 14px; font-weight: 500; color: #1d1d1f; margin-top: auto; }");
                         client.println(".btn-on .state { color: #ffffff; }");
                         client.println("</style></head><body>");
                         
                         client.println("<div class=\"panel\">");
                         client.println("<h2>控制中心</h2>");
                         client.println("<div class=\"grid\">");
                         
                         // 循环生成 $2\times2$ 极简网格按钮
                         for (int i = 0; i < 4; i++) {
                             bool isOn = (relayStates[i] == "on");
                             client.println("<a href=\"/" + String(i) + "/toggle\" class=\"btn " + String(isOn ? "btn-on" : "") + "\">");
                             client.println("  <span class=\"name\">" + deviceNames[i] + "</span>");
                             client.println("  <span class=\"state\">" + String(isOn ? "已开启" : "已关闭") + "</span>");
                             client.println("</a>");
                         }
                         
                         client.println("</div></div></body></html>");
                         client.println();
                         break;
                     } else { currentLine = ""; }
                 } else if (c != '\r') { currentLine += c; }
             }
         }
         header = "";
         client.stop();
     }
 }