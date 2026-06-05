# ESP32 Sandbox

## Overview
This repository contains eleven ESP32 projects covering basic bring-up, wireless scanning, BLE advertising, local relay control, MQTT relay control, MQTT telemetry output, remote firmware maintenance through OTA, a feature-rollout OTA demo, a dual-speaker OTA and alerting module, an autonomous edge lifecycle management agent, and an industrial fail-safe relay controller. The projects are organized as small, self-contained examples for bench testing, integration validation, and field-oriented proof-of-concept work.

## Project Index

| Project | One-line description |
| --- | --- |
| `01_hello_world` | Minimal ESP32 bring-up example with on-board LED status blinking. |
| `02_wifi_scanner` | Wi-Fi scan utility for discovering nearby access points and validating radio behavior. |
| `03_ble_advertiser` | BLE advertising example for exercising the ESP32 Bluetooth radio in peripheral mode. |
| `04_smart_light` | Four-channel relay control demo for local switching and simple automation validation. |
| `05_cloud_hub` | MQTT-connected relay hub for remote cloud-side switching and transport verification. |
| `06_telemetry_hub` | MQTT telemetry node that streams ultrasonic distance data for edge sensing workflows. |
| `07_ota_update` | Baseline remote firmware maintenance example focused on OTA update execution. |
| `08_ota_feature_rollout` | MQTT-triggered OTA rollout demo with HTTP transport and manual recovery support. |
| `09_ota_active_buzzer_rollout` | Advanced OTA rollout module with dual-speaker alerting and separated passive/active buzzer roles. |
| `10_edge_lifecycle_agent` | Industrial edge lifecycle agent with autonomous OTA supervision, resilient network recovery, and hardware-backed operational signaling. |
| `11_industrial_relay_controller` | Industrial reliability relay controller with fail-safe startup sequencing, watchdog supervision, and non-blocking state management. |

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
| 07_ota_update | GPIO 2 | On-board LED | Status heartbeat and OTA progress indicator |
| 08_ota_feature_rollout | GPIO 2 | Status LED | OTA progress and runtime state indicator |
| 08_ota_feature_rollout | GPIO 18 | BUZZER | Audible output / alert channel |
| 08_ota_feature_rollout | GPIO 0 | BOOT button | Manual OTA trigger |
| 09_ota_active_buzzer_rollout | GPIO 18 | Passive buzzer | Shaped melody playback |
| 09_ota_active_buzzer_rollout | GPIO 19 | Active buzzer | Fault-grade alert signaling |
| 09_ota_active_buzzer_rollout | GPIO 0 | BOOT button | Manual OTA trigger |
| 09_ota_active_buzzer_rollout | GPIO 2 | Status LED | Runtime and progress indicator |
| 10_edge_lifecycle_agent | GPIO 2 | Status LED | Network and runtime state feedback |
| 10_edge_lifecycle_agent | GPIO 19 | Active buzzer | Operational alerts and OTA fault signaling |
| 10_edge_lifecycle_agent | GPIO 0 | BOOT button | Local recovery and OTA fallback trigger |
| 11_industrial_relay_controller | GPIO 18 | IN1 | Relay channel 0 |
| 11_industrial_relay_controller | GPIO 19 | IN2 | Relay channel 1 |
| 11_industrial_relay_controller | GPIO 21 | IN3 | Relay channel 2 |
| 11_industrial_relay_controller | GPIO 22 | IN4 | Relay channel 3 |

Relay outputs in `04_smart_light` and `05_cloud_hub` are configured for HIGH-level trigger mode.

## Firmware Architecture & Dependencies
All projects use the Arduino framework on PlatformIO with the ESP32 core package. `05_cloud_hub`, `06_telemetry_hub`, `07_ota_update`, and `08_ota_feature_rollout` use `PubSubClient` (`knolleary/PubSubClient @ ^2.8`) for MQTT transport. `10_edge_lifecycle_agent` also uses `PubSubClient` and adds `ArduinoJson` for metadata-driven lifecycle control. The networked projects avoid long blocking delays in the main loop where keepalive, telemetry, or update supervision must remain active.

## Deployment Guide
Each project can be built and flashed with PlatformIO CLI:

```bash
cd <project_folder>
pio run
pio run --target upload
pio device monitor
```

Update the Wi-Fi, broker, GPIO, and OTA settings in each project before flashing.

## Engineering Notes & Edge Cases
- WiFi/MQTT jitter: the MQTT-based projects keep `client.loop()` serviced continuously so broker keepalive does not fail during normal network jitter.
- Network port ingress: port `1883` may be blocked on corporate or campus networks. A 2.4 GHz cellular hotspot is a common fallback for broker access.
- Payload constraints vs formats: the telemetry, control, and OTA command paths use plain primitive payloads instead of JSON to keep heap use low and match simple client-side parsing requirements.
- Hardware polarity: relay boards must match the configured trigger level. If the jumper setting or module logic is different, outputs will invert.
- OTA partitioning: `07_ota_update`, `08_ota_feature_rollout`, `09_ota_active_buzzer_rollout`, and `10_edge_lifecycle_agent` depend on a partition table that leaves adequate application space for dual-image update workflows.
- Lifecycle supervision: `10_edge_lifecycle_agent` is designed to keep checking remote metadata so the node can promote itself to a newer release without requiring a one-shot operator command.
