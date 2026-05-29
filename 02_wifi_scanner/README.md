# 02_wifi_scanner

## Overview
This project scans nearby Wi-Fi networks on the ESP32 and prints SSID, RSSI, and security type over serial. It is used for site surveys and quick validation of 2.4 GHz coverage.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| None | None | Wi-Fi radio only |
| 3.3V | Board supply | Device power |
| GND | Board ground | Common reference |

## Firmware Architecture & Dependencies
The firmware uses the Arduino framework on PlatformIO and the ESP32 `WiFi` library from the core package. Each scan runs sequentially, prints results to the serial console, and calls `WiFi.scanDelete()` after use to release scan memory.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 02_wifi_scanner
pio run
pio run --target upload
pio device monitor
```

## Engineering Notes & Edge Cases
- Scan latency: Full scans can take several seconds, so the loop waits between scans instead of polling continuously.
- Memory cleanup: `WiFi.scanDelete()` is called after each pass to avoid holding old results in RAM.
- Security labels: Encryption modes are mapped to readable names for field debugging, but some access points may still report vendor-specific values as unknown.
