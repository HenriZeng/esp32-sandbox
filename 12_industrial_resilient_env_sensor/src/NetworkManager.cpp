#include "NetworkManager.h"

void NetworkManager::initialize() {
    _mqttClient.setClient(_espClient);
    _mqttClient.setServer(_mqttServer, 1883);
    WiFi.begin(_ssid, _password);
}

void NetworkManager::update() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    if (!_mqttClient.connected()) {
        // Attempt reconnection only every 5 seconds to prevent network flood
        unsigned long currentMillis = millis();
        if (currentMillis - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = currentMillis;
            _reconnect();
        }
    } else {
        _mqttClient.loop();
    }
}

void NetworkManager::_reconnect() {
    if (_mqttClient.connect("ESP32_Industrial_Node_01")) {
        Serial.println("[NETWORK] MQTT connected. Sending discovery packets...");
        sendDiscoveryMessages(); // <--- Add this call
    }
}

bool NetworkManager::publish(const char* topic, const char* payload) {
    return _mqttClient.connected() ? _mqttClient.publish(topic, payload) : false;
}

void NetworkManager::sendDiscoveryMessages() {
    // Discovery topic for Temperature
    const char* tempTopic = "homeassistant/sensor/esp32_node/temp/config";
    String tempPayload = "{"
        "\"name\": \"Industrial Temp\","
        "\"state_topic\": \"sensors/industrial/env\","
        "\"unit_of_measurement\": \"°C\","
        "\"value_template\": \"{{ value_json.temperature }}\","
        "\"unique_id\": \"esp32_temp_001\""
    "}";
    
    // Use true for the 'retained' flag so HA receives the config even after a reboot
    _mqttClient.publish(tempTopic, tempPayload.c_str(), true);
}