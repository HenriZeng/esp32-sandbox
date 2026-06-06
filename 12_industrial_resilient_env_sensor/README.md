# Industrial Resilient Environment Sensor

## Overview
`12_industrial_resilient_env_sensor` is an ESP32-based environmental monitoring node designed to demonstrate industrial-grade resilience engineering. The firmware combines deterministic startup, non-blocking sensing, controlled Wi-Fi/MQTT connectivity, and watchdog-supervised recovery so the device can operate predictably in unattended edge deployments.

## Key Engineering Features

### Deterministic Startup
Hardware is initialized in a controlled order before the system enters steady-state operation. This reduces undefined behavior during boot and supports repeatable bring-up across power cycles and field resets.

### Non-Blocking Sensor Scheduling
The DHT11 acquisition flow is implemented as a state-driven update path rather than a long blocking routine. This keeps the main loop responsive, preserves watchdog servicing, and allows network tasks to continue while telemetry is being prepared.

### Network Resilience
`NetworkManager` separates Wi-Fi bring-up, MQTT connection handling, and publish operations. Reconnect attempts are rate-limited to avoid flooding the network during outages, and MQTT-based Home Assistant discovery is emitted after a successful connection for easier ecosystem integration.

### Watchdog Supervision
The ESP32 task watchdog is enabled to detect loop stalls and force recovery when the application becomes unresponsive. A serial-triggered fault-injection path is included so recovery behavior can be validated under controlled failure conditions.

### End-to-End Integration
Temperature and humidity measurements are serialized as JSON and published over MQTT for downstream visualization, automation, or alerting. The result is a complete sensor-to-broker-to-consumer chain rather than an isolated peripheral demo.

## System Architecture

| Module | Responsibility |
| --- | --- |
| `main.cpp` | System bootstrap, watchdog servicing, telemetry scheduling, diagnostic fault injection |
| `SensorManager` | Non-blocking DHT11 acquisition, validation, and state management |
| `NetworkManager` | Wi-Fi connectivity, MQTT reconnect logic, publish support, discovery messaging |
| `config.h` | Build-time configuration for Wi-Fi, MQTT broker, and watchdog parameters |

## Hardware Connections

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 25 | DHT11 data | Environmental sensor input |
| 3.3V | VCC | Sensor power |
| GND | GND | Common ground |

## Software Stack
- PlatformIO
- Arduino framework for ESP32
- `WiFi` library
- `PubSubClient`
- ESP32 task watchdog (`esp_task_wdt`)

## Build and Flash

```bash
cd 12_industrial_resilient_env_sensor
pio run
pio run --target upload
pio device monitor
```

## Runtime Behavior
1. Power the ESP32 and open the serial monitor at `115200` baud.
2. The firmware initializes the sensor node, network stack, and watchdog supervision.
3. Once Wi-Fi and MQTT are available, telemetry is published every 5 seconds to `sensors/industrial/env`.
4. After reconnection, Home Assistant discovery messages are published automatically.
5. Send `f` through the serial monitor to simulate a fatal stall and verify watchdog recovery.

## Resilience Notes
This project is intentionally structured around embedded reliability principles:
- predictable boot sequencing,
- bounded reconnect behavior,
- periodic telemetry publishing,
- explicit fault injection for validation,
- watchdog-backed self-recovery.

These patterns are directly applicable to industrial edge nodes where uptime, observability, and safe recovery matter more than raw feature count.