# ESP32 Sandbox

## Overview
This repository contains six ESP32 projects for basic bring-up, wireless scanning, BLE advertising, local relay control, MQTT relay control, and MQTT telemetry output. The projects are organized as small, self-contained examples for bench testing and field validation.

## Hardware Configuration & Pinout

| Project | ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- | --- |
| 01_hello_world | GPIO 2 | On-board LED | Status blink output |
| 02_wifi_scanner | None | None | Wi-Fi radio only |
| 03_ble_advertiser | None | None | BLE radio only |
| 04_smart_light | GPIO 2 | IN1 | Relay channel 0 |
| 04_smart_light | GPIO 4 | IN2 | Relay channel 1 |
| 04_smart_light | GPIO 16 | IN3 | Relay channel 2 |
| 04_smart_light | GPIO 17 | IN4 | Relay channel 3 |
| 05_cloud_hub | GPIO 2 | IN1 | Relay channel 0 |
| 05_cloud_hub | GPIO 4 | IN2 | Relay channel 1 |
| 05_cloud_hub | GPIO 16 | IN3 | Relay channel 2 |
| 05_cloud_hub | GPIO 17 | IN4 | Relay channel 3 |
| 06_telemetry_hub | GPIO 18 | TRIG | Ultrasonic trigger output |
| 06_telemetry_hub | GPIO 19 | ECHO | Ultrasonic echo input |

Relay outputs in `04_smart_light` and `05_cloud_hub` are configured for HIGH-level trigger mode.

## Firmware Architecture & Dependencies
All projects use the Arduino framework on PlatformIO with the ESP32 core package. `05_cloud_hub` and `06_telemetry_hub` use `PubSubClient` (`knolleary/PubSubClient @ ^2.8`) for MQTT transport. The networked projects avoid long blocking delays in the main loop where keepalive or periodic telemetry must remain active.

## Deployment Guide
Each project can be built and flashed with PlatformIO CLI:

```bash
cd <project_folder>
pio run
pio run --target upload
pio device monitor
```

Update the Wi-Fi, broker, and GPIO settings in each project before flashing.

## Engineering Notes & Edge Cases
- WiFi/MQTT jitter: The MQTT-based projects keep `client.loop()` serviced continuously so broker keepalive does not fail during normal network jitter.
- Network port ingress: Port `1883` may be blocked on corporate or campus networks. A 2.4 GHz cellular hotspot is a common fallback for broker access.
- Payload constraints vs formats: The telemetry and control paths use plain primitive payloads instead of JSON to keep heap use low and match simple client-side parsing requirements.
- Hardware polarity: Relay boards must match the configured trigger level. If the jumper setting or module logic is different, outputs will invert.
