# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Build/Upload Commands

**Multi-board ESP32 system with specific environments:**
```bash
# Upload to front ESP32 (master controller)
pio run -e front_esp32 -t upload
pio device monitor -e front_esp32

# Upload to rear ESP32 (slave controller) 
pio run -e rear_esp32 -t upload
pio device monitor -e rear_esp32

# Upload to ESP32-CAM (requires GPIO0->GND during upload)
pio run -e camera_esp32 -t upload
# Remove GPIO0->GND connection and press reset after upload
```

## Project-Specific Conventions

**Preprocessor Defines (critical for different board behaviors):**
- `FRONT_CONTROLLER` - Master ESP32 with decision logic
- `REAR_CONTROLLER` - Slave ESP32 for motor execution
- `CAMERA_MODULE` - ESP32-CAM for vision streaming

**UART Communication Pattern:**
- Master ESP32 #1 communicates with both Slave #2 and ESP32-CAM
- Serial2 connects ESP32 #1 ↔ ESP32 #2 (motor control)
- Serial1 connects ESP32 #1 ↔ ESP32-CAM (vision)

**Non-Obvious File Dependencies:**
- `rules/agent-protocol.md` contains mandatory MCP tool usage workflow
- Documentation in `docs/` is the actual implementation reference (not comments)
- WiFi network "ProjectNightfall" hardcoded in multiple files (not config-driven)

## Critical Gotchas

**ESP32-CAM Programming:** GPIO0 must connect to GND during upload, then disconnect and reset for normal operation

**Power System:** Motors run on 14.8V (battery pack), ESP32s on 5V via LM2596 buck converter

**ESP32 Voltage Safety:** ESP32 GPIO pins are NOT 5V tolerant (3.3V logic only) - must use level shifters for 5V sensors. Development boards accept 5V on VIN/USB pins (onboard regulator), but NEVER connect 5V directly to GPIO pins.

**MQ-2 Gas Sensor:** Requires 24-48 hour pre-heat period before reliable readings

**Three-ESP32 Architecture:** Master-slave pattern with JSON-based inter-board communication at 115200 baud