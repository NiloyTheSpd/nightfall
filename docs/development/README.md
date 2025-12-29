# Development Documentation - Project Nightfall ESP32

## Overview

This directory contains comprehensive documentation for the Project Nightfall ESP32 system, including PlatformIO CLI commands, network connectivity verification, diagnostic modifications, and troubleshooting procedures.

## Quick Start

1. **[PlatformIO Commands](platformio-commands.md)** - Exact CLI commands for uploading firmware to each ESP32 device
2. **[Network Verification](network-verification.md)** - Step-by-step procedures to verify network connectivity
3. **[Diagnostic Modifications](diagnostic-modifications.md)** - Code enhancements for better diagnostics
4. **[Troubleshooting Guide](troubleshooting-guide.md)** - Comprehensive troubleshooting procedures

## System Architecture

The Project Nightfall system uses a **three-ESP32 architecture**:

- **BACK ESP32 (Master/Brain)**: WiFi Access Point + WebSocket server
- **FRONT ESP32 (Motor Slave)**: Receives commands via UART
- **CAMERA ESP32 (Telemetry Node)**: Sends telemetry via WebSocket

## Key Files Reference

### PlatformIO Configuration

- **platformio.ini**: Project configuration with all ESP32 environments
- **include/config.h**: Central configuration constants
- **src/main\_\*.cpp**: Source code for each ESP32 device

### Communication Libraries

- **lib/Communication/UARTComm.h/cpp**: UART communication handling
- **lib/Motors/MotorControl.h/cpp**: Motor control implementation
- **lib/Web/WebServer.h/cpp**: Web server and WebSocket handling

### Documentation Structure

```
docs/development/
├── README.md                    # This file
├── platformio-commands.md       # CLI commands for each device
├── network-verification.md      # Network connectivity procedures
├── diagnostic-modifications.md  # Code enhancements for diagnostics
└── troubleshooting-guide.md     # Step-by-step troubleshooting
```

## Critical Information

### Upload Ports

- **BACK ESP32**: `/dev/ttyUSB0`
- **FRONT ESP32**: `/dev/ttyUSB1`
- **CAMERA ESP32**: `/dev/ttyUSB2`

### ESP32-CAM Programming Sequence

1. **During Upload**: Connect GPIO0 to GND
2. **After Upload**: Disconnect GPIO0 and press RESET
3. **Normal Operation**: GPIO0 should be floating

### Network Configuration

- **WiFi SSID**: "ProjectNightfall"
- **WiFi Password**: "rescue2025"
- **Master IP**: 192.168.4.1
- **WebSocket Port**: 8888
- **HTTP Port**: 80

### UART Communication

- **Baud Rate**: 115200
- **Connection**: BACK ESP32 ↔ FRONT ESP32 (GPIO16/17)
- **Protocol**: JSON commands and responses

## Essential Commands

### Build and Upload

```bash
# Build all environments
pio run

# Upload to specific device
pio run -e back_esp32 -t upload      # Master
pio run -e front_esp32 -t upload     # Motor Slave
pio run -e camera_esp32 -t upload    # Camera (with GPIO0->GND)

# Monitor serial output
pio device monitor -e back_esp32
pio device monitor -e front_esp32
pio device monitor -e camera_esp32
```

### Diagnostics

```bash
# List connected devices
pio device list

# Build diagnostic version
pio run -e diagnostic -t upload

# Check system status
# (commands available via serial monitor)
status      # Show system status
wifi        # Check WiFi connection
uart_stats  # Show UART statistics
mem         # Show memory usage
diagnostic  # Generate full diagnostic report
```

## Network Topology

```
┌─────────────────┐    UART (Serial2)    ┌─────────────────┐
│   BACK ESP32    │◄───────────────────►│  FRONT ESP32    │
│   (Master)      │    115200 baud      │  (Motor Slave)  │
│                 │                     │                 │
│ WiFi AP:        │                     │                 │
│ • SSID:         │                     │                 │
│   ProjectNightfall                    │                 │
│ • Pass: rescue2025                    │                 │
│ • IP: 192.168.4.1                     │                 │
│                 │                     │                 │
│ WebSocket Srv:  │                     │                 │
│ • Port: 8888    │                     │                 │
│ • HTTP: 80      │                     │                 │
└─────────────────┘                     └─────────────────┘
         │                                       │
         │ WiFi Client                           │
         ▼                                       │
┌─────────────────┐                               │
│  CAMERA ESP32   │                               │
│ (Telemetry Node)│                               │
│                 │                               │
│ WiFi:           │◄──────────────────────────────┘
│ • Client to AP                          │
│ • WebSocket     │
│   Client:       │
│   192.168.4.1:8888                       │
│                 │
└─────────────────┘
```

## Communication Protocols

### UART Messages (BACK ↔ FRONT)

**Motor Commands (BACK → FRONT)**:

```json
{"L": 150, "R": 150}  // Move forward
{"L": 0, "R": 0}      // Stop
{"cmd": "emergency_stop"}  // Emergency stop
```

**Heartbeat (FRONT → BACK)**:

```json
{
  "type": "heartbeat",
  "source": "front",
  "timestamp": 123456,
  "emergency": false,
  "leftSpeed": 150,
  "rightSpeed": 150,
  "uptime": 123456
}
```

### WebSocket Messages

**Telemetry (BACK → Clients)**:

```json
{
  "type": "telemetry",
  "timestamp": 123456,
  "dist": 45.2,
  "rearDist": 120.5,
  "gas": 150,
  "battery": 14.8,
  "uptime": 123456,
  "emergency": false
}
```

**Camera Heartbeat (CAMERA → BACK)**:

```json
{
  "type": "heartbeat",
  "source": "camera",
  "timestamp": 123456,
  "uptime": 123456,
  "wifi_rssi": -45,
  "wifi_ip": "192.168.4.2",
  "memory_free": 123456,
  "boot_count": 1,
  "version": "2.0.0"
}
```

## Safety Systems

### Emergency Stop Conditions

- **Front Distance < 20cm**: Obstacle detected
- **Gas Level > 400**: Dangerous gas levels
- **UART Timeout > 1000ms**: Communication loss
- **Manual Emergency**: User-triggered stop

### Safety Response

1. Immediate motor stop
2. Buzzer activation (500ms)
3. Emergency state broadcast
4. WebSocket dashboard update
5. System remains stopped until reset

## Troubleshooting Quick Reference

| Issue                | Primary Check     | Secondary Check         |
| -------------------- | ----------------- | ----------------------- |
| Upload fails         | USB connections   | GPIO0 state (ESP32-CAM) |
| No WiFi AP           | BACK ESP32 power  | Serial monitor output   |
| No motor response    | UART connections  | Emergency stop status   |
| WebSocket fails      | BACK ESP32 server | Network connectivity    |
| ESP32-CAM won't boot | GPIO0 floating    | Power supply quality    |

## Performance Benchmarks

### Expected Response Times

- **UART Command → Motor Response**: <50ms
- **WebSocket Telemetry**: Every 500ms
- **WiFi Connection**: <10 seconds
- **Emergency Stop Reaction**: <100ms

### System Reliability

- **UART Communication**: 100% (wired)
- **WiFi Connectivity**: 95%+ (environment dependent)
- **WebSocket Auto-reconnect**: Every 5 seconds

## Development Workflow

1. **Setup**: Install PlatformIO and dependencies
2. **Hardware**: Connect ESP32 devices according to wiring diagram
3. **Upload**: Use exact CLI commands for each device
4. **Verify**: Follow network verification procedures
5. **Test**: Run functional tests for all components
6. **Debug**: Use troubleshooting guide for issues
7. **Deploy**: Use production environment for final deployment

## Support Resources

- **PlatformIO Documentation**: https://docs.platformio.org/
- **ESP32 Arduino Reference**: https://github.com/espressif/arduino-esp32
- **ArduinoJson Documentation**: https://arduinojson.org/
- **Project Documentation**: See main `docs/` directory

## Version Information

- **Project Version**: 2.0.0
- **Documentation Version**: 1.0.0
- **Last Updated**: December 29, 2025
- **PlatformIO Version**: 6.x
- **ESP32 Arduino Core**: 2.0.0+

For detailed procedures and advanced diagnostics, refer to the individual documentation files in this directory.
