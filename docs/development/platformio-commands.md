# PlatformIO CLI Commands - Project Nightfall ESP32

## Overview

Project Nightfall uses a **three-ESP32 architecture** with distinct roles and communication patterns:

- **BACK ESP32 (Master/Brain)**: Central controller with WiFi AP and WebSocket server
- **FRONT ESP32 (Motor Slave)**: Receives motor commands via UART
- **CAMERA ESP32 (Telemetry Node)**: Sends telemetry via WebSocket

## Exact Upload Commands

### 1. BACK ESP32 (Master/Brain Controller)

**Device**: Master controller with WiFi Access Point
**Upload Port**: `/dev/ttyUSB0`
**Serial Monitor**: `/dev/ttyUSB0`

```bash
# Build and upload firmware
pio run -e back_esp32 -t upload

# Monitor serial output
pio device monitor -e back_esp32

# Build only (no upload)
pio run -e back_esp32

# Clean build
pio run --target clean -e back_esp32
```

### 2. FRONT ESP32 (Motor Slave Controller)

**Device**: Motor control slave
**Upload Port**: `/dev/ttyUSB1`
**Serial Monitor**: `/dev/ttyUSB1`

```bash
# Build and upload firmware
pio run -e front_esp32 -t upload

# Monitor serial output
pio device monitor -e front_esp32

# Build only (no upload)
pio run -e front_esp32

# Clean build
pio run --target clean -e front_esp32
```

### 3. CAMERA ESP32 (Telemetry Node)

**Device**: ESP32-CAM with camera and WiFi
**Upload Port**: `/dev/ttyUSB2`
**Serial Monitor**: `/dev/ttyUSB2`

```bash
# Build and upload firmware
pio run -e camera_esp32 -t upload

# Monitor serial output
pio device monitor -e camera_esp32

# Build only (no upload)
pio run -e camera_esp32

# Clean build
pio run --target clean -e camera_esp32
```

## Critical ESP32-CAM Upload Sequence

⚠️ **IMPORTANT**: ESP32-CAM requires special programming sequence:

1. **During Upload**: Connect GPIO0 to GND
2. **After Upload**: Disconnect GPIO0 and press RESET button
3. **Normal Operation**: GPIO0 should be floating (disconnected)

```bash
# Upload to ESP32-CAM (with GPIO0->GND connected)
pio run -e camera_esp32 -t upload

# After upload completes:
# 1. Disconnect GPIO0 from GND
# 2. Press RESET button
# 3. Monitor should show startup messages
pio device monitor -e camera_esp32
```

## Build and Upload All Devices

```bash
# Build all environments
pio run

# Upload to specific device (choose one)
pio run -e back_esp32 -t upload      # Master
pio run -e front_esp32 -t upload     # Motor Slave
pio run -e camera_esp32 -t upload    # Camera/Telemetry

# Monitor all devices (run in separate terminals)
pio device monitor -e back_esp32     # Terminal 1
pio device monitor -e front_esp32    # Terminal 2
pio device monitor -e camera_esp32   # Terminal 3
```

## Development Environment

```bash
# Single-device development (builds all source files)
pio run -e development -t upload

# Debug build with extra symbols
pio run -e debug -t upload

# Production build (optimized)
pio run -e production -t upload
```

## Board-Specific Configuration

### Environment Identifiers

- `back_esp32`: ESP32 DevKit (Master Controller)
- `front_esp32`: ESP32 DevKit (Motor Slave)
- `camera_esp32`: ESP32-CAM (Telemetry Node)
- `development`: Single-board development environment

### Key Preprocessor Defines

- `BACK_ESP32` / `FRONT_ESP32` / `CAMERA_MODULE`: Board identification
- `WIFI_SSID="ProjectNightfall"`: Network name
- `WIFI_PASSWORD="rescue2025"`: Network password
- `UART_BAUDRATE=115200`: Communication speed
- `WEBSOCKET_PORT=8888`: WebSocket server port

### Upload Port Mapping

- **Master ESP32**: `/dev/ttyUSB0`
- **Motor Slave ESP32**: `/dev/ttyUSB1`
- **Camera ESP32**: `/dev/ttyUSB2`

## Troubleshooting Commands

```bash
# List connected devices
pio device list

# Check device monitoring
pio device monitor --help

# Clean entire project
pio run --target clean

# Force rebuild
pio run --force
```

## Serial Monitoring Tips

```bash
# Monitor with specific baud rate
pio device monitor -e back_esp32 --baud 115200

# Monitor and save to file
pio device monitor -e back_esp32 > log.txt

# Monitor with timestamps
pio device monitor -e back_esp32 --timestamp
```

## Dependencies

All environments require:

- **ArduinoJson** v7.0.0 (or v6.21.3 for development)
- **ESPAsyncWebServer** v1.2.3 (Master only)
- **AsyncTCP** v1.1.3 (Master only)
- **WebSockets** v2.4.1 (Master & Camera)

## Build Output Locations

```
.pio/build/back_esp32/        # Master firmware
.pio/build/front_esp32/       # Motor Slave firmware
.pio/build/camera_esp32/      # Camera firmware
```

## Version Information

- **Project Version**: 2.0.0
- **Platform**: ESP32 Arduino Framework
- **Flash Size**: 4MB
- **CPU Frequency**: 240MHz
- **Flash Mode**: DIO
