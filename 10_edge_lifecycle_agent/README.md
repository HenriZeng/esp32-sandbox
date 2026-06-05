# 09_ota_active_buzzer_rollout

An ESP32 OTA and acoustic alerting demo that implements a **dual-speaker audio subsystem** by decoupling the passive buzzer and the active buzzer into two distinct responsibilities:

- the **passive buzzer on GPIO18** is used for melody playback through a software-shaped acoustic envelope
- the **active buzzer on GPIO19** is reserved for hardware-level exception signaling and operational alerts during OTA and network events

This project extends the earlier rollout examples with a more specialized audio model for field-oriented firmware maintenance. It combines a non-blocking MQTT control plane, HTTP-based firmware transport, and a soft-attack tone engine designed to reduce the harshness of conventional square-wave buzzer playback.

## 1. Project Overview

This project demonstrates how to separate user-facing acoustic feedback from fault-grade alerting on a single ESP32 platform.

The passive buzzer handles structured melodies and routine status tones, while the active buzzer provides immediate, unmistakable alarm pulses when OTA, Wi-Fi, broker, or write-phase failures occur. This separation improves clarity, avoids acoustic contention, and mirrors the behavior expected from production-grade embedded service indicators.

At the system level, the firmware uses MQTT only as a command trigger and HTTP as the firmware delivery channel. The runtime remains responsive outside the update window, allowing MQTT keepalive processing to continue while the device waits for commands or plays background melodies.

## 2. Key Features

- **Dual-speaker acoustic design** with clear functional separation between melody playback and fault alerting
- **Soft-attack acoustic smoothing** to reduce the initial transient harshness of passive buzzer output
- **Linear decay envelope** to shape note release and produce a cleaner, more musical tone profile
- **Hardware-level active alerting** for critical conditions that require immediate operator attention
- **Non-blocking MQTT control path** that keeps the network session alive during normal operation
- **HTTP-based OTA download flow** that streams firmware in bounded chunks instead of buffering the full image in RAM
- **Manual local OTA trigger** through the BOOT button for bench validation and recovery workflows
- **Version-aware melody selection** for release-specific acoustic behavior

## 3. Hardware Setup

| Component | ESP32 GPIO | Electrical / Logic Note | Purpose |
| --- | --- | --- | --- |
| Passive buzzer | GPIO18 | Low-level triggering through the PWM-driven passive buzzer path | Melody playback and shaped acoustic feedback |
| Active buzzer | GPIO19 | Current-limiting resistor required for safe direct drive | Exception alerts and OTA fault signaling |
| BOOT button | GPIO0 | Active-low input | Manual OTA trigger |
| Status LED | GPIO2 | On-board indicator | Runtime and progress feedback |

### Wiring Notes

- GPIO18 is configured for the passive buzzer path and is driven through PWM so the firmware can apply a soft-attack envelope and controlled decay.
- GPIO19 drives the active buzzer directly. A current-limiting resistor must be used to protect the GPIO and the buzzer circuit.
- The BOOT button is read as an active-low input and can initiate the local OTA fallback path.

## 4. Build and Run

This project uses PlatformIO with the Arduino framework on the `esp32dev` target.

### Build

```bash
cd 09_ota_active_buzzer_rollout
pio run
```

### Upload

```bash
cd 09_ota_active_buzzer_rollout
pio run --target upload
```

### Serial Monitor

```bash
cd 09_ota_active_buzzer_rollout
pio device monitor
```

## 5. Repository Layout

```text
09_ota_active_buzzer_rollout/
├── platformio.ini
├── README.md
└── src/
    └── main.cpp
```

## 6. Operational Summary

1. The ESP32 connects to Wi-Fi.
2. The firmware subscribes to the MQTT OTA command topic.
3. A published HTTP URL triggers the OTA download path.
4. The binary is streamed into the inactive partition using the `Update` API.
5. On success, the device restarts into the new image.
6. On failure, the active buzzer emits a hardware-level alert pattern.

## 7. Engineering Notes

- The passive buzzer is optimized for musical playback, not alarm signaling.
- The active buzzer is optimized for immediate fault visibility and should be treated as the primary exception channel.
- The OTA path is intentionally split into control-plane messaging and transport-plane retrieval to keep the architecture simple and diagnosable.
- The main loop remains responsive during normal operation so MQTT keepalive servicing is preserved.

## 8. Practical Use Cases

This project is useful when you want to demonstrate:

- differentiated acoustic UX for normal and exceptional states
- embedded OTA workflows with a clear operator feedback model
- a small but realistic example of MQTT-triggered firmware rollout
- a simple field-maintenance pattern for ESP32-based devices

## 9. Quick Start

```bash
cd 09_ota_active_buzzer_rollout
pio run
pio run --target upload
pio device monitor
```

After flashing, you can publish an HTTP firmware URL to the configured MQTT topic to initiate OTA, or press the BOOT button to invoke the local fallback path.
