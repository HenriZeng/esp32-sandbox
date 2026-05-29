# 03_ble_advertiser

## Overview
This project starts the ESP32 BLE stack and advertises a custom service UUID. It is used to confirm BLE radio operation and provide a discoverable peripheral for phone or gateway tests.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| None | None | BLE radio only |
| 3.3V | Board supply | Device power |
| GND | Board ground | Common reference |

## Firmware Architecture & Dependencies
The firmware uses the Arduino framework on PlatformIO with the ESP32 BLE classes from the core package. Advertising is started once in `setup()`, and the main loop remains idle except for a periodic status print because BLE advertising runs in the background.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 03_ble_advertiser
pio run
pio run --target upload
pio device monitor
```

## Engineering Notes & Edge Cases
- UUID management: The service UUID must stay unique to avoid collisions with other BLE test devices.
- Connection quirks: The advertising parameters include `setMinPreferred()` values to reduce connection issues on some iPhone clients.
- Background behavior: BLE advertisement continues without explicit loop handling, so the firmware does not need polling for radio activity.
