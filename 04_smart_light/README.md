# 04_smart_light

## Overview
This project is a standalone 4-channel relay controller with a local web UI hosted by the ESP32 access point. It is used for direct on-device switching without MQTT or external network access.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 2 | IN1 | Relay channel 0 |
| GPIO 4 | IN2 | Relay channel 1 |
| GPIO 16 | IN3 | Relay channel 2 |
| GPIO 17 | IN4 | Relay channel 3 |
| 3.3V / 5V | VCC | Relay module supply, per module rating |
| GND | GND | Common ground |

Relay outputs are configured for HIGH-level trigger mode.

## Firmware Architecture & Dependencies
The firmware uses the Arduino framework on PlatformIO and the ESP32 `WiFi` library for soft AP mode. The HTTP server is implemented directly with `WiFiServer`, and the relay state is stored in simple arrays so the page can be rebuilt without extra dependencies.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 04_smart_light
pio run
pio run --target upload
pio device monitor
```

Connect to the access point `Henri-Smart-Hub` with password `12345678` after flashing.

## Engineering Notes & Edge Cases
- Local-only access: This design does not depend on an external broker, which avoids WAN latency and outbound port restrictions.
- HTTP parsing: Button toggles are matched from the request URI string, so malformed requests should be ignored rather than treated as valid relay commands.
- Trigger polarity: The relay board is configured for HIGH trigger mode, which must match the actual module jumper setting before deployment.
