/**
 * Project: 02_wifi_scanner
 * Description: Scans for nearby Wi-Fi networks and prints details in English.
 * Author: Henri
 */

 #include <Arduino.h>
 #include <WiFi.h>
 
 // Helper function to translate encryption type to a readable string
 const char* getEncryptionType(wifi_auth_mode_t encryptionType) {
     switch (encryptionType) {
         case WIFI_AUTH_OPEN:            return "Open";
         case WIFI_AUTH_WEP:             return "WEP";
         case WIFI_AUTH_WPA_PSK:         return "WPA_PSK";
         case WIFI_AUTH_WPA2_PSK:        return "WPA2_PSK";
         case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA_WPA2_PSK";
         case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
         default:                        return "Unknown";
     }
 }
 
 void setup() {
     Serial.begin(115200);
     delay(1000);
 
     Serial.println("\n--- ESP32 WiFi Scanner Initialized ---");
     
     // Set WiFi to Station Mode and ensure it is disconnected
     WiFi.mode(WIFI_STA);
     WiFi.disconnect();
     delay(100);
 
     Serial.println("Setup complete. Ready to scan.");
 }
 
 void loop() {
     Serial.println("\nScanning for networks...");
 
     // scanNetworks returns the number of networks found
     int n = WiFi.scanNetworks();
     
     if (n == 0) {
         Serial.println("No networks found.");
     } else {
         Serial.printf("%d networks found:\n", n);
         for (int i = 0; i < n; ++i) {
             // Print SSID, RSSI, and the English encryption name
             Serial.printf("%d: %s | RSSI: %d dBm | Security: %s\n", 
                           i + 1, 
                           WiFi.SSID(i).c_str(), 
                           WiFi.RSSI(i), 
                           getEncryptionType(WiFi.encryptionType(i)));
             delay(10);
         }
     }
 
     // Delete scan result from memory to avoid leakage
     WiFi.scanDelete();
 
     Serial.println("Waiting 10 seconds before next scan...");
     delay(10000);
 }