# 05_cloud_hub

## Overview
This project is an ESP32-based 4-channel relay controller for remote switching over MQTT. It is intended for WAN control of simple industrial or lab loads through a broker reachable from a mobile client or other MQTT endpoint.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 2  | IN1 | Relay channel 0 |
| GPIO 4  | IN2 | Relay channel 1 |
| GPIO 16 | IN3 | Relay channel 2 |
| GPIO 17 | IN4 | Relay channel 3 |
| 3.3V / 5V | VCC | Relay module supply, per module rating |
| GND | GND | Common ground |

Relay outputs are configured for HIGH-level trigger mode. The broker uses `broker.emqx.io:1883`.

## Firmware Architecture & Dependencies
The firmware uses the Arduino framework on PlatformIO and `PubSubClient` (`knolleary/PubSubClient @ ^2.8`) for MQTT transport. Relay control is handled with fixed GPIO mapping, and the main loop stays non-blocking so `client.loop()` runs continuously for MQTT keepalive and inbound message handling.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 05_cloud_hub
pio run
pio run --target upload
pio device monitor
```

Update `platformio.ini` and `src/main.cpp` with your Wi-Fi and broker settings before flashing.

## Engineering Notes & Edge Cases
- WiFi/MQTT jitter: The control path avoids long blocking delays in `loop()`. This keeps `client.loop()` serviced often enough to maintain the MQTT heartbeat and avoid broker disconnects during normal network jitter.
- Network port ingress: Public and remote access depends on the path to port `1883`. In restricted environments, corporate or campus Wi-Fi may block the broker connection; switching to a 2.4 GHz cellular hotspot can bypass that ingress filter.
- Payload constraints vs formats: The control topic uses short primitive commands such as `0:ON` and `0:OFF`. The status topic publishes plain strings instead of JSON to keep heap use low and satisfy simple client-side graphing or automation tools that expect a raw value.
