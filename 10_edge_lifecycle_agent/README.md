# 10_edge_lifecycle_agent

An ESP32 edge lifecycle management agent that extends the `09_ota_active_buzzer_rollout` architecture into a more complete **industrial service node**. Instead of treating OTA as a one-shot firmware rollback or a manual recovery action, this project models the device as a continuously supervised edge endpoint with autonomous update polling, resilient network reattachment, hardware-level alerting, and explicit operator-triggered recovery.

The firmware combines MQTT as the command/control plane, HTTP as the firmware transport path, periodic metadata polling for declarative update decisions, and direct GPIO interaction for status indication and recovery signaling. The result is a compact but realistic foundation for edge devices that must remain serviceable in constrained or unreliable network environments.

## 1. Project Overview

This project demonstrates a higher-order lifecycle pattern for ESP32-based edge equipment.

Compared with the earlier OTA rollout examples, the agent is designed to continuously evaluate its own firmware state, compare local and remote versions, and initiate an update when a newer image is published by the orchestration server. The same system also accepts MQTT-triggered OTA commands and provides a physical BOOT-button fallback for local maintenance.

At the infrastructure level, the firmware is structured around four operational concerns:

- **autonomous lifecycle supervision** through periodic metadata polling
- **OTA execution** through HTTP streaming into the inactive flash partition
- **high-availability network recovery** through repeated Wi-Fi and MQTT reattachment
- **hardware interaction** through status LED feedback, BOOT-button override, and active buzzer signaling

This makes the project closer to a production-style edge agent than a simple demo node.

## 2. Key Features

- **Autonomous OTA lifecycle management** with periodic remote version checks
- **Declarative update metadata ingestion** using a lightweight JSON endpoint
- **MQTT control-plane trigger support** for operator-initiated firmware rollout
- **HTTP-based firmware transport** with bounded streaming into flash
- **High-availability Wi-Fi reconnection logic** with retry, radio reset, and sleep-disable hardening
- **Persistent MQTT keepalive servicing** during normal runtime operation
- **Active buzzer fault signaling** for deployment and lifecycle events
- **Status LED runtime indication** for link and operational state
- **BOOT-button local recovery path** for bench validation and field intervention
- **Partition-aware OTA workflow** designed for dual-slot firmware deployment

## 3. Hardware Setup

| Component | ESP32 GPIO | Electrical / Logic Note | Purpose |
| --- | --- | --- | --- |
| Status LED | GPIO2 | On-board indicator | Network and runtime state feedback |
| BOOT button | GPIO0 | Active-low input | Local maintenance and OTA fallback trigger |
| Active buzzer | GPIO19 | Direct GPIO drive with current limiting | Operational alerts and OTA fault signaling |

### Wiring Notes

- GPIO2 is used as the visual status indicator for network join and runtime health.
- GPIO19 drives the active buzzer directly and should be wired with appropriate current limiting.
- GPIO0 is configured as an active-low mechanical input and can initiate the local OTA fallback path.
- The board should use an OTA-capable partition table with enough room for the deployed image size.

## 4. Firmware Architecture

### 4.1 Control Plane

MQTT is used as the command plane for remote OTA triggering. The device subscribes to `henri/ota/cmd` and accepts an HTTP URL payload for manual update activation.

### 4.2 Lifecycle Supervisor

The firmware periodically polls a remote metadata endpoint to determine whether the published server version is newer than the locally running firmware. If a version gap is detected, the agent downloads the referenced image and performs OTA automatically.

### 4.3 Transport Layer

Firmware delivery uses HTTP rather than MQTT payload streaming. This keeps the control path small and the transport path deterministic, while allowing the image to be written in a streaming fashion instead of buffering the whole binary in RAM.

### 4.4 Network Resilience

The Wi-Fi connection routine is hardened for field use:

- station mode is enforced explicitly
- Wi-Fi sleep is disabled to reduce jitter-related failures
- stale connection state is cleared before reconnect attempts
- retry loops include a radio reset fallback

This approach improves reattachment reliability on unstable networks and makes the device more predictable during long-running deployments.

### 4.5 Hardware Feedback Path

The active buzzer provides a direct audible channel for important lifecycle events. The BOOT button offers a local override path for maintenance and recovery. Together, these hardware interfaces make the system easier to operate on a bench and in the field.

## 5. Build and Run

This project uses PlatformIO with the Arduino framework on the `esp32dev` target.

### Build

```bash
cd 10_edge_lifecycle_agent
pio run
```

### Upload

```bash
cd 10_edge_lifecycle_agent
pio run --target upload
```

### Serial Monitor

```bash
cd 10_edge_lifecycle_agent
pio device monitor
```

## 6. Repository Layout

```text
10_edge_lifecycle_agent/
├── platformio.ini
├── README.md
└── src/
    └── main.cpp
```

## 7. Operational Flow

1. The ESP32 boots and initializes the status LED, active buzzer, and BOOT button.
2. The firmware connects to Wi-Fi using a hardened reconnect sequence.
3. MQTT is connected and the device subscribes to the OTA command topic.
4. The agent polls the remote metadata endpoint on a fixed interval.
5. If the remote version is newer, the firmware downloads the referenced binary over HTTP.
6. The image is written into the inactive OTA slot and validated.
7. The device restarts into the new image after successful finalization.
8. If the network or update path fails, the active buzzer provides immediate audible feedback.

## 8. Engineering Notes

- The project is intentionally structured as a lifecycle agent rather than a one-shot updater.
- MQTT remains the control plane only; it is not used for binary transport.
- The periodic metadata check enables a declarative rollout pattern suitable for edge fleet supervision.
- The Wi-Fi recovery path prioritizes service continuity over minimal code size.
- The BOOT-button override is useful for local recovery when remote orchestration is unavailable.
- The OTA pipeline depends on a partition table that supports dual-image update workflows.
- The active buzzer is reserved for operational signaling rather than melody playback.

## 9. Practical Use Cases

This project is useful when you want to demonstrate:

- autonomous firmware lifecycle supervision on constrained edge hardware
- a production-style OTA control model with separate trigger and transport paths
- resilient network reattachment strategies for unreliable deployments
- hardware-backed operator feedback for edge serviceability
- a clear architectural step beyond a simple rollback-oriented OTA demo

## 10. Relationship to 09_ota_active_buzzer_rollout

`09_ota_active_buzzer_rollout` focuses on a specialized OTA and acoustic alerting workflow with explicit separation between passive and active buzzer responsibilities.

`10_edge_lifecycle_agent` builds on that foundation and advances the architecture into a full edge lifecycle management model by adding:

- autonomous version polling
- declarative update orchestration
- stronger network resilience logic
- a broader operational role for the device as a managed edge agent

In other words, the 09 project emphasizes **rollout execution**, while the 10 project emphasizes **continuous lifecycle supervision**.

## 11. Quick Start

```bash
cd 10_edge_lifecycle_agent
pio run
pio run --target upload
pio device monitor
```

After flashing, ensure the metadata endpoint and OTA binary URL are reachable from the ESP32, then observe the device while it polls for updates and responds to MQTT or BOOT-button triggers.