/**
 * @file NetworkManager.h
 * @brief Handles Wi-Fi and MQTT connectivity with non-blocking reconnection.
 */
 #ifndef NETWORK_MANAGER_H
 #define NETWORK_MANAGER_H
 
 #include <WiFi.h>
 #include <PubSubClient.h>
 
 class NetworkManager {
 public:
     NetworkManager(const char* ssid, const char* password, const char* mqttServer)
         : _ssid(ssid), _password(password), _mqttServer(mqttServer) {}
 
     void initialize();
     void update(); // Must be called in main loop
     bool publish(const char* topic, const char* payload);
     void sendDiscoveryMessages(); 

 private:
     const char* _ssid;
     const char* _password;
     const char* _mqttServer;
     WiFiClient _espClient;
     PubSubClient _mqttClient;
     
     unsigned long _lastReconnectAttempt = 0;
     void _reconnect();
 };
 
 #endif