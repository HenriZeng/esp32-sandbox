# 07_ota_update

## Overview
This project implements a secure, MQTT-triggered HTTP OTA update pipeline for the ESP32. The firmware listens for a remote firmware image URL, downloads the binary over HTTP, validates the transfer, and writes the new application image into the inactive flash slot before performing a controlled reboot.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 2 | On-board LED | Status heartbeat and OTA progress indicator |

## Firmware Architecture & Dependencies
The firmware is built on the Arduino framework for PlatformIO and uses `PubSubClient` (`knolleary/PubSubClient @ ^2.8`) for MQTT command ingress, `HTTPClient` for binary retrieval, and `Update` for flash programming. OTA execution is intentionally event-driven: the device remains connected to the broker, receives a URL payload on a dedicated topic, and then streams the remote firmware image into flash in bounded chunks to reduce heap pressure and avoid socket starvation.

The project currently targets the `esp32dev` board profile and uses the `min_spiffs.csv` partition scheme to reserve sufficient application space for OTA operation.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 07_ota_update
pio run
pio run --target upload
pio device monitor
```

Before flashing, update the Wi-Fi credentials, MQTT broker parameters, and OTA topic in `src/main.cpp` to match your deployment environment.

To trigger an update, publish the firmware binary URL to the configured MQTT topic:

```bash
mosquitto_pub -h broker.emqx.io -p 1883 -t henri/ota/cmd -m "http://<firmware-host>/firmware.bin"
```

## Engineering Notes & Edge Cases
- Non-blocking supervision: the main loop continues to service `client.loop()` whenever the device is not actively writing flash, preserving broker keepalive during steady-state operation.
- Transfer validation: the OTA handler checks HTTP status code, payload length, and byte count completion before calling `Update.end()`.
- Flash safety: firmware data is written through `Update` in 1 KB chunks to keep RAM usage deterministic and reduce the risk of watchdog starvation.
- Reboot strategy: after a successful image write and verification, the device performs an explicit restart so the bootloader can switch to the newly installed application slot.
- Operational caveat: the current implementation accepts an HTTP URL payload from MQTT. For production deployments, this should be hardened with authenticated transport, message authorization, and signed firmware verification.
