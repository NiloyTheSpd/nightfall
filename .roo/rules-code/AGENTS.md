# Project Coding Rules (Non-Obvious Only)

**Three-Board Architecture:** ESP32 #1 (Master), ESP32 #2 (Slave), ESP32-CAM (Vision) - each requires separate PlatformIO environment

**Critical Preprocessor Defines:** Must use exactly `FRONT_CONTROLLER`, `REAR_CONTROLLER`, or `CAMERA_MODULE` - board behavior differs significantly based on these flags

**UART Cross-Connections:** Serial2 connects Master↔Slave for motor control, Serial1 connects Master↔CAM for vision (NOT the typical Serial/Serial1 pattern)

**ESP32-CAM Programming Sequence:** GPIO0 must connect to GND during upload, then disconnect GPIO0 and press reset for normal operation

**Gas Sensor Calibration:** MQ-2 requires 24-48 hour preheat period before reliable readings - not mentioned in hardware documentation

**Power System Gotcha:** Motors use 14.8V battery pack directly, ESP32s use 5V via LM2596 buck converter - different voltage levels per component

**ESP32 Voltage Safety:** ESP32 GPIO pins are NOT 5V tolerant (3.3V logic only) - must use level shifters for 5V sensors. Development boards accept 5V on VIN/USB pins (onboard regulator), but NEVER connect 5V directly to GPIO pins.

**WiFi Network:** "ProjectNightfall" hardcoded in multiple source files, not config-driven as expected