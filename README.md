# ESP32 Sandbox

A dedicated workspace for hands-on experimentation, prototyping, and mastering firmware development on the **ESP32-E (N4)** platform.

## Project Structure

This repository uses a monorepo approach to organize progressive learning modules:

* **`01_hello_world/`**: Initial hardware verification. Focuses on basic GPIO manipulation (LED blinking) and high-baud UART serial communication.
* **`02_wifi_scanner/`**: Exploring the Espressif Wi-Fi stack. Scanning local access points, handling RSSI signal metrics, and state transitions.
* **`03_ble_advertiser/`**: Prototyping Bluetooth Low Energy (BLE) beacon capabilities and custom GATT server profiles.

## Hardware & Environment

* **Microcontroller**: ESP32-WROOM-32E (4MB Flash / Integrated Wi-Fi & BT)
* **Framework**: Arduino Core / ESP-IDF via PlatformIO
* **IDE Tools**: Cursor (AI-assisted engineering environment)
* **Target Baud Rate**: `115200`