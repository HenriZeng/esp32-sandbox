# 06_telemetry_hub

## Overview
This project is an ESP32 telemetry node that measures ultrasonic distance and publishes a numeric value over MQTT. It is intended for basic industrial level monitoring or simple remote sensing demos.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 18 | TRIG | Ultrasonic trigger output |
| GPIO 19 | ECHO | Ultrasonic echo input |
| 3.3V | VCC | Sensor supply |
| GND | GND | Common ground |

## Firmware Architecture & Dependencies
The firmware uses the Arduino framework on PlatformIO and `PubSubClient` (`knolleary/PubSubClient @ ^2.8`) for MQTT transport. Telemetry is scheduled with `millis()` so sampling and publishing stay separate from MQTT keepalive handling in `client.loop()`.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 06_telemetry_hub
pio run
pio run --target upload
pio device monitor
```

Update `platformio.ini` and `src/main.cpp` with your Wi-Fi and broker settings before flashing.

## Engineering Notes & Edge Cases
- Non-blocking scheduling: `millis()` is used for periodic sampling so MQTT servicing is not starved while waiting for the next sensor read.
- Sensor timeout: `pulseIn()` includes a timeout to avoid hanging the firmware when the echo line is disconnected or the target is out of range.
- Payload format: The firmware publishes a plain float string instead of JSON to reduce heap use and match clients that only accept a numeric series.
