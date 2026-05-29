# 05_cloud_hub | WAN MQTT Smart Control Hub

This project implements a 4-channel relay controller communicating over the **Wide Area Network (WAN)** using the **MQTT protocol**. The ESP32 acts as an MQTT client, connecting to a local Wi-Fi network and subscribing to a cloud-based MQTT broker. This enables users to securely achieve real-time, low-latency (millisecond-level) remote control over physical hardware from anywhere in the world using cellular networks.

---

## 🛠️ Hardware & Cloud Specifications
* **MCU**: ESP32 Dev Module
* **Peripherals**: 4-Channel Relay Module (Mapped to GPIOs: `2`, `4`, `16`, `17`)
* **Hardware Trigger Mode**: Physical jumper caps locked to **HIGH (High-Level Trigger)** for optimal 3.3V-5V logic compatibility.
* **MQTT Broker**: `broker.emqx.io` (Public sandbox broker, Port: `1883`)

---

## 📱 Mobile Client (IoT OnOff) Deployment Guide

To configure your mobile device for cross-network remote control, download the **IoT OnOff** app and follow these deployment steps:

### 1. Create a New Panel
* Launch the app, tap **Add a Panel**.
* Name the panel `Henri Smart Hub` and save it to enter the dashboard.

### 2. Configure MQTT Connection Settings
Tap the settings/plug icon in the top right corner of your panel and fill in the following parameters to authenticate with the cloud broker:

| Parameter | Configuration Value |
| :--- | :--- |
| **Protocol** | `MQTT` |
| **Host** | `broker.emqx.io` |
| **Port** | `1883` |
| **Client ID** | `Phone-Henri` (or any unique identifier) |
| **Username** | *Leave Blank* |
| **Password** | *Leave Blank* |

> 💡 **Upon successful authentication, the connection indicator status will turn green or display "Connected".**

### 3. Add Control Widgets
Tap **Add Widget** in the blank dashboard space and select the **Switch** component. Configure the 4-channel control matrix exactly as follows:

| Device Name | GPIO Pin | Publish Topic | Payload On | Payload Off |
| :--- | :---: | :--- | :---: | :---: |
| **Light 1** | 2 | `henri/hub/control` | `0:ON` | `0:OFF` |
| **Fan 2** | 4 | `henri/hub/control` | `1:ON` | `1:OFF` |
| **Device 3**| 16 | `henri/hub/control` | `2:ON` | `2:OFF` |
| **Device 4**| 17 | `henri/hub/control` | `3:ON` | `3:OFF` |

---

## 📡 Bidirectional Status Feedback (Optional Feature)
The firmware supports active state reporting. To implement telemetry confirmation on your mobile client, subscribe to the telemetry topic:
* **Telemetry Subscription Topic**: `henri/hub/status`
* **LWT / Online Message**: The ESP32 publishes `Henri Hub Online!` immediately upon establishing a cloud handshake.
* **Command Acknowledgement**: Upon execution of a toggle payload, the hardware returns an execution log (e.g., `Channel 0 is ON`).