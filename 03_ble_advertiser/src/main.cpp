/**
 * Project: 03_ble_advertiser
 * Description: Initializes the ESP32 BLE stack and starts broadcasting packets.
 * Author: Henri
 */

 #include <Arduino.h>
 #include <BLEDevice.h>
 #include <BLEUtils.h>
 #include <BLEServer.h>
 
 // Every BLE service needs a unique 128-bit ID (UUID). 
 // You can generate custom ones at uuidgenerator.net
 #define SERVICE_UUID        "4fa4c201-1fb2-45e5-8a89-19a6d4567f5d"
 
 void setup() {
     Serial.begin(115200);
     delay(1000);
     Serial.println("\n--- ESP32 BLE Advertiser Initializing ---");
 
     // 1. Initialize the BLE Device with a professional device name
     BLEDevice::init("Henri-ESP32-Sandbox");
 
     // 2. Create a BLE Server (acting as a peripheral device)
     BLEServer *pServer = BLEDevice::createServer();
 
     // 3. Create a BLE Service using our unique UUID
     BLEService *pService = pServer->createService(SERVICE_UUID);
 
     // 4. Start the service
     pService->start();
 
     // 5. Configure Advertising settings
     BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
     pAdvertising->addServiceUUID(SERVICE_UUID);
     pAdvertising->setScanResponse(true);
     pAdvertising->setMinPreferred(0x06);  // Helps with iPhone connection issues
     pAdvertising->setMinPreferred(0x12);
 
     // 6. Start advertising data over the air
     BLEDevice::startAdvertising();
     Serial.println("BLE Advertising started successfully!");
     Serial.println("Device Name: Henri-ESP32-Sandbox");
 }
 
 void loop() {
     // BLE advertising runs completely in the background via hardware/FreeRTOS.
     // The loop can stay idle or perform other periodic monitoring tasks.
     Serial.println("BLE is broadcasting... open your phone to scan.");
     delay(5000);
 }