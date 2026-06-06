# Embedded Systems Portfolio

## Portfolio Summary
I am an embedded systems engineer who has progressed from foundational MCU bring-up to industrial-grade edge firmware. My development path reflects a steady move from basic peripheral control to disciplined system architecture, non-blocking runtime design, connectivity integration, and resilience engineering for deployments that must remain predictable under real-world operating conditions.

## Project Portfolio

| Project | Core Technical Focus | Status |
| --- | --- | --- |
| 01 - Hello World | MCU bring-up, serial I/O, basic firmware workflow | Completed |
| 02 - Wi-Fi Scanner | Wi-Fi scanning, serial diagnostics, network discovery | Completed |
| 03 - BLE Advertiser | Bluetooth Low Energy advertising, payload configuration | Completed |
| 04 - Smart Light | GPIO control, device logic, embedded application structure | Completed |
| 05 - Cloud Hub | Cloud connectivity, telemetry pipeline, networked firmware | Completed |
| 06 - Telemetry Hub | Structured data publishing, sensor aggregation, connectivity handling | Completed |
| 07 - OTA Update | Over-the-air update flow, firmware maintenance, deployment safety | Completed |
| 08 - OTA Feature Rollout | Feature delivery strategy, staged firmware evolution, OTA validation | Completed |
| 09 - OTA Active Buzzer Rollout | OTA-driven actuation control, alerting logic, release management | Completed |
| 10 - Industrial Communication Node | Robust device communication, field integration, industrial messaging | Completed |
| 11 - Industrial Fail-Safe Relay Controller | Fail-safe initialization, watchdog supervision, non-blocking relay sequencing | Completed |
| 12 - Industrial Resilient Environment Sensor | Resilience engineering, non-blocking sensing, MQTT telemetry, watchdog recovery | Completed |

## Flagship Spotlight

### 12 - Industrial Resilient Environment Sensor
This is the flagship project in the portfolio because it best demonstrates the depth expected from an industrial embedded engineer: resilience engineering plus end-to-end system integration.

On the resilience engineering side, the firmware is built around deterministic startup, bounded reconnection behavior, watchdog-supervised recovery, and a deliberate fault-injection path for validation. These are not cosmetic design choices; they are the operating principles required for unattended edge devices where fault tolerance, safe recovery, and predictable behavior matter more than feature count.

On the end-to-end integration side, the project completes the full path from sensing to downstream consumption: DHT11 acquisition, local state-machine scheduling, Wi-Fi connectivity, MQTT transport, and Home Assistant discovery support. The result is a production-style telemetry node that can be discussed credibly with MNC interviewers as a complete embedded system rather than a disconnected hardware demo.

## Repository Structure

- `01_hello_world`
- `02_wifi_scanner`
- `03_ble_advertiser`
- `04_smart_light`
- `05_cloud_hub`
- `06_telemetry_hub`
- `07_ota_update`
- `08_ota_feature_rollout`
- `09_ota_active_buzzer_rollout`
- `10_industrial_communication_node`
- `11_industrial_relay_controller`
- `12_industrial_resilient_env_sensor`

## Positioning
This repository is intended to present an embedded engineer whose growth path spans from microcontroller fundamentals to industrial-grade systems engineering. The portfolio highlights practical competence in firmware architecture, reliability, connectivity, and lifecycle-aware device behavior.