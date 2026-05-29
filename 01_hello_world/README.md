# 01_hello_world

## Overview
This project is an ESP32 serial bring-up test with a built-in LED blink loop. It is used to verify basic board power, UART output, and GPIO control before moving to networked firmware.

## Hardware Configuration & Pinout

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 2 | On-board LED | Status blink output |
| 3.3V | Board supply | Device power |
| GND | Board ground | Common reference |

## Firmware Architecture & Dependencies
The firmware uses the Arduino framework on PlatformIO with no external libraries beyond the core ESP32 package. The loop is intentionally simple and uses blocking delays because the project is only a local hardware sanity check.

## Deployment Guide
Build and flash with PlatformIO CLI:

```bash
cd 01_hello_world
pio run
pio run --target upload
pio device monitor
```

## Engineering Notes & Edge Cases
- Serial startup: The code waits after `Serial.begin()` so the USB-UART console is ready before the first message is printed.
- LED mapping: GPIO 2 is used for the board LED on common ESP32 dev modules, but some boards wire the indicator differently.
- Timing: The blink cycle uses `delay(1000)` because there is no background communication task to service.
