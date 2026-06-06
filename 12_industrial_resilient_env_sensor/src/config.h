/**
 * @file config.h
 * @brief Configuration constants for the industrial sensor node.
 */
 #ifndef CONFIG_H
 #define CONFIG_H
 
 // Network Credentials
 const char* WIFI_SSID     = "CMCC-yxU3";
 const char* WIFI_PASSWORD = "x7f5f3dc";
 
 // System Constants
 const int WDT_TIMEOUT_SECONDS = 5; // Watchdog reset time
 const char* MQTT_BROKER_IP    = "192.168.1.15";
 
 #endif