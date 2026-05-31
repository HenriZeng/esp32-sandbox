# 08_ota_feature_rollout

An ESP32 OTA feature-rollout demo that combines **MQTT as the control plane** with **HTTP as the firmware transport**. The device listens for remote OTA commands, can also trigger a manual OTA from the BOOT button, downloads the binary over HTTP, writes it into the inactive OTA partition, validates the flash result, and restarts into the new image.

This project is written for PlatformIO + Arduino ESP32 and is intended to demonstrate a more feature-rich rollout flow than the baseline OTA example:

- event-driven Wi-Fi and MQTT connectivity
- manual recovery/trigger path through the BOOT button
- bounded HTTP streaming for firmware delivery
- OTA slot safety and validation
- version-aware runtime behavior
- practical field rollout considerations

## 1. System Overview

### OTA Control Flow

1. ESP32 joins Wi-Fi and connects to the MQTT broker.
2. Firmware subscribes to the OTA command topic.
3. An operator publishes an HTTP URL pointing to a `.bin` image.
4. The device downloads the image over HTTP in bounded chunks.
5. `Update` writes the image into the inactive flash slot.
6. The firmware verifies the byte count and finalization result.
7. The device restarts and boots the newly flashed image.

### Manual Rollout Path

In addition to MQTT-triggered OTA, the firmware also supports a local button-driven OTA path:

- press the BOOT button
- the firmware pauses normal runtime behavior
- the device fetches the preconfigured firmware image URL
- the same OTA validation and restart flow is executed

This makes the board easier to test on a bench and provides a practical fallback when remote triggering is not convenient.

## 2. Hardware and Firmware Baseline

| Item | Value |
| --- | --- |
| Target board | `esp32dev` |
| Framework | Arduino |
| Build system | PlatformIO |
| Status LED | GPIO 2 |
| Buzzer output | GPIO 18 |
| Manual OTA trigger | GPIO 0 / BOOT button |
| OTA transport | HTTP download triggered by MQTT or local button |
| MQTT topic | `henri/ota/cmd` |
| Default broker | `broker.emqx.io:1883` |
| Partition scheme | `min_spiffs.csv` |

## 3. Firmware Dependencies

The project currently uses the following core libraries:

- `WiFi.h` for station connectivity
- `PubSubClient` for MQTT subscription and reconnect handling
- `HTTPClient` for remote firmware retrieval
- `Update` for flash programming and OTA slot finalization
- `ArduinoJson` is included as a dependency for future rollout payload expansion

## 4. Repository Layout

```text
08_ota_feature_rollout/
├── platformio.ini
├── README.md
└── src/
    └── main.cpp
```

## 5. Local Build and Flash

### Prerequisites

Install the following tools on macOS:

- PlatformIO CLI
- Python 3
- MQTTX Desktop, or another MQTT client that can publish plain text payloads

Verify the basics:

```bash
python3 --version
pio --version
```

### Build and Upload

```bash
cd 08_ota_feature_rollout
pio run
pio run --target upload
pio device monitor
```

## 6. macOS Local Execution Playbook

This is the recommended end-to-end workflow for local validation on a Mac.

### Step 1 — Prepare a firmware binary

Build the project first so PlatformIO generates a `.bin` artifact:

```bash
cd 08_ota_feature_rollout
pio run
```

The compiled application binary is typically produced under the PlatformIO build directory.

### Step 2 — Start a local HTTP server with Python

Serve the directory that contains the firmware binary so the ESP32 can fetch it over HTTP.

If you want to serve the current project directory from macOS:

```bash
cd 08_ota_feature_rollout
python3 -m http.server 8000
```

If the binary is in a build output directory, serve that directory directly instead. For example:

```bash
cd 08_ota_feature_rollout/.pio/build/esp32dev
python3 -m http.server 8000
```

Confirm the file is reachable from a browser on your Mac:

```text
http://127.0.0.1:8000/<firmware-file-name>.bin
```

Important note for real hardware: the ESP32 cannot use `127.0.0.1` or `localhost` to reach your Mac. Use your Mac’s LAN IP address instead, for example:

```text
http://192.168.1.23:8000/<firmware-file-name>.bin
```

You can discover the Mac IP with:

```bash
ipconfig getifaddr en0
```

If you are on Ethernet or a different interface, adjust the interface accordingly.

### Step 3 — Open an MQTT client

In MQTTX, create a new connection with these baseline settings:

- Host: `broker.emqx.io`
- Port: `1883`
- Client ID: any unique value
- Username/password: none for the public sandbox
- Protocol: MQTT 3.1.1

After connecting:

1. Open the publish panel.
2. Set the topic to `henri/ota/cmd`.
3. Set the message payload to the full HTTP URL of the firmware binary.
4. Publish the message.

Example payload:

```text
http://192.168.1.23:8000/firmware.bin
```

### Step 4 — Watch the device behavior

Open the serial monitor:

```bash
pio device monitor
```

You should see the ESP32:

- receive the MQTT message
- validate the URL
- connect to the HTTP server
- download the binary
- write the OTA image
- reboot after successful verification

### Step 5 — Confirm the new version boots

After reboot, the device should resume normal operation and print the updated firmware banner and periodic heartbeat messages.

## 7. Feature-Rollout Engineering Notes

These are the key implementation insights that matter in real deployments and interview discussions.

### 7.1 MQTT is only the trigger, not the transport

We intentionally avoid sending firmware bytes over MQTT. MQTT is excellent for small control messages, but firmware images are better delivered through HTTP because:

- the payload size is simpler to manage
- the client code is easier to keep deterministic
- failure modes are easier to diagnose
- the server can be any simple static file host

### 7.2 Manual trigger improves field debugging

The BOOT-button OTA path gives you a local fallback when remote control is unavailable. This is useful for:

- bench validation
- recovery during network bring-up
- operator-assisted flashing in a lab
- debugging OTA behavior without waiting for an external command

### 7.3 Bounded chunk streaming keeps RAM usage predictable

The download path writes the binary in small fixed-size chunks rather than buffering the entire firmware image in memory. This is critical on ESP32-class hardware where heap fragmentation and peak RAM pressure can become field issues under real network conditions.

### 7.4 OTA write and validation are separated

The firmware does not assume that a successful download automatically means a valid image. It explicitly checks:

- HTTP response status
- content length
- total bytes written
- `Update.end()` result
- `Update.isFinished()` state

That final validation sequence is what protects the device from booting an incomplete or corrupted image.

### 7.5 Version-aware runtime behavior supports staged rollout

The firmware includes a version constant and uses it to adjust runtime timing behavior. This pattern is useful when demonstrating staged feature rollout, because you can alter the device personality by version without changing the entire system architecture.

### 7.6 The main loop remains responsive outside the flash window

The code keeps `client.loop()` running during steady state so MQTT keepalive is maintained. The flash-update section is treated as a short-lived critical window where normal application work is temporarily suspended.

### 7.7 Reboot is deliberate and controlled

After a successful OTA finalization, the firmware performs an explicit restart so the bootloader can switch to the newly written image slot. This avoids ambiguous runtime state and matches expected embedded update behavior.

## 8. Field-Observed Edge Cases and Practical Mitigations

### 8.1 Public MQTT broker collision risk

The default topic and broker are suitable for demo work, but public sandboxes can be noisy. In production, the OTA topic should be namespaced, authenticated, and ideally bound to device identity.

### 8.2 HTTP URL trust boundary

The current implementation accepts any payload that begins with `http`. That is acceptable for a lab demo, but not sufficient for production. A hardened design should add:

- broker-side authorization
- TLS for MQTT and HTTP
- firmware signature verification
- version rollback protection
- possibly a manifest-based update process

### 8.3 Network reliability during download

Production Wi-Fi environments may drop packets, stall TCP streams, or delay DNS resolution. The implementation is intentionally simple, but for robust fleet rollout you should consider:

- retry strategy with backoff
- timeout tuning
- image hash validation
- resume support if the delivery model requires it

### 8.4 Partition sizing must match image growth

OTA requires adequate free slot space. If the firmware grows significantly, the selected partition scheme must be revisited. This is one of the first constraints reviewers should check during a production readiness assessment.

## 9. Operational Checklist

Before triggering an OTA:

- Confirm the ESP32 is online and subscribed to the OTA topic.
- Confirm the firmware binary is reachable from the ESP32 over HTTP.
- Confirm the binary URL uses the Mac’s LAN IP, not `localhost`.
- Confirm the MQTT payload is a plain text URL.
- Confirm the serial monitor is open so you can observe the update path.
- If using the local fallback, confirm the BOOT button is wired and the preconfigured URL is reachable.

## 10. Suggested Interview Talking Points

If you are presenting this project in a senior embedded interview, the strongest discussion points are:

- why MQTT was chosen as a control plane rather than a data plane
- how `Update` handles OTA slot safety on ESP32
- why fixed-size streaming matters on memory-constrained devices
- how the firmware avoids blocking the MQTT keepalive path during normal operation
- how a manual trigger improves serviceability in the field
- what production hardening steps are still required before fleet deployment
- how you would add integrity, authentication, and rollback guarantees

## 11. Security and Production Readiness Caveat

This repository is intentionally a **technical demonstration** of the OTA workflow. It is not a complete production security solution.

For real deployments, add at minimum:

- MQTT over TLS
- HTTPS firmware hosting
- authenticated command authorization
- signed firmware images
- anti-rollback controls
- secure manufacturing and provisioning flow

## 12. Quick Start Summary

```bash
cd 08_ota_feature_rollout
pio run
python3 -m http.server 8000
pio device monitor
```

Then publish the binary URL in MQTTX to topic `henri/ota/cmd`, or press the BOOT button to use the local rollout path.

## 13. Example MQTT Payload

```text
http://192.168.1.23:8000/firmware.bin
```

Replace the IP address and file name with your actual Mac host and built artifact.
