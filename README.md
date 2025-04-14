# Smart Solar-Aware Socket Controller (ESP32 + FreeRTOS + Wi-Fi Dashboard)

## Overview

This project implements a Wi-Fi-enabled, smart socket controller using the **ESP32**, **FreeRTOS**, and the **PZEM-004T** power monitoring module. The system interfaces with an appliance plug, dynamically deciding when to supply power based on solar system data including **Battery Capacity**, **Load Power**, **Supply Power**, and **Absolute Load Power**.

It supports multiple control modes, real-time monitoring, and web-based interaction through a dashboard for remote configuration and insights.

## Features

- 💡 **Intelligent Socket Control**: Dynamically powers appliances based on real-time solar data to conserve battery and manage loads efficiently.
- 🔌 **PZEM-004T Integration**: Accurately monitors voltage, current, and power of the connected appliance.
- ☀️ **Solar System-Aware Logic**: Adapts power decisions based on external solar metrics received via serial or wireless communication.
- 🧠 **Mode Support**:
  - **Automatic**: Fully autonomous operation based on all system inputs.
  - **Sense**: Reads solar data only when external power is absent.
  - **Bypass**: Socket remains powered regardless of conditions.
- 📶 **Wi-Fi Monitoring and Control**:
  - Real-time dashboard showing solar data and device status
  - Remote power toggle and configuration options
  - Optional over-the-air (OTA) firmware updates
- 🔄 **FreeRTOS Architecture**:
  - Dedicated tasks for sensor reading, socket control, data transmission, and Wi-Fi/webserver management
  - Multicore task scheduling using ESP32's dual-core CPU

## Inputs

- `BC` – Battery Capacity (%)
- `SP` – Solar Supply Power (W)
- `ALP` – Absolute Load Power (W)
- `LP` – Load Power of appliance (W) via **PZEM-004T**

## Outputs

- Relay control signal for powering the appliance
- JSON telemetry over WebSocket or HTTP (for dashboard)
- Optional local display or status LEDs

## Hardware Components

- 🧠 **ESP32** with Wi-Fi
- 🔌 **Relay Module** (AC appliance control)
- 🔍 **PZEM-004T** (appliance monitoring)
- 📡 External Solar Transmitter (providing BC, ALP, SP)
- 🔘 Mode Selector (toggle or web-based)
- 💡 Indicator LEDs or OLED display (optional)

## Software Architecture

- **ESP-IDF** or **Arduino-ESP32**
- **FreeRTOS** task-based structure
- **Web Server** (Async or Embedded) for dashboard interface
- **WebSocket or REST API** for real-time control/monitoring
- **UART/SPI/I²C** communication with solar transmitter
- **PZEM-004T** library for UART power monitoring

## Example Use Cases

- Prioritize essential appliances in solar-powered homes
- Prevent overload and protect battery health in off-grid systems
- Monitor and manage energy usage remotely
- Automate power scheduling based on sunlight availability

## Dashboard (Web-Based)

Accessible from any device on the same network:
- View: battery %, load wattage, power supply, and socket state
- Control: toggle socket power, switch modes
- Settings: update thresholds, Wi-Fi config, OTA updates

## License

MIT License

