/**
 * Project: 01_hello_world
 * Description: Basic Serial communication and LED blink test for ESP32.
 * Author: Henri
 */

 #include <Arduino.h>

 // Most ESP32 dev boards have a built-in LED on GPIO 2
 #define LED_PIN 2
 
 void setup() {
     // Initialize serial communication at 115200 baud rate
     Serial.begin(115200);
     delay(1000);
     
     Serial.println("\n--- ESP32 Initialized Successfully ---");
     pinMode(LED_PIN, OUTPUT);
 }
 
 void loop() {
     Serial.println("Hello World from ESP32-E!");
     
     // Blink the built-in LED
     digitalWrite(LED_PIN, HIGH);
     delay(1000);
     digitalWrite(LED_PIN, LOW);
     delay(1000);
 }