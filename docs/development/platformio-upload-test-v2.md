# PlatformIO Upload Commands - Test Version 2

## Project Nightfall ESP32 Multi-Board System

**Version:** 2.0.0  
**Date:** December 29, 2025  
**Project:** Test Version 2 - Unified WiFi Networking Architecture  
**Classification:** Hardware Deployment Guide

---

## Table of Contents

1. [PlatformIO Upload Commands](#1-platformio-upload-commands)
2. [Expected Behavior After Upload](#2-expected-behavior-after-upload)
3. [WiFi Startup Verification](#3-wifi-startup-verification)
4. [Verification Steps](#4-verification-steps)
5. [Troubleshooting Guide](#5-troubleshooting-guide)
6. [Critical Safety Notes](#6-critical-safety-notes)

---

## 1. PlatformIO Upload Commands

### 1.1 Three-Board Upload Sequence (Test Version 2)

**Critical Preprocessor Defines:**

- `BACK_CONTROLLER` - Master ESP32 with decision logic (uses back_esp32 environment)
- `FRONT_CONTROLLER` - Slave ESP32 for motor execution
- `CAMERA_MODULE` - ESP32-CAM for vision streaming

### 1.2 REAR ESP32 (Master Controller) - Upload First

**Device:** Master controller with WiFi Access Point  
**Upload Port:** COM port (auto-detected)  
**Serial Monitor:** COM port (auto-detected)
**Environment:** `back_esp32`

```bash
# Navigate to project directory
cd e:/night-fall

# Build and upload firmware to Rear ESP32 (Master)
python -m platformio run -e back_esp32 -t upload

# Monitor serial output (run in separate terminal)
python -m platformio device monitor -e back_esp32

# Alternative: Build only (no upload)
python -m platformio run -e back_esp32

# Clean build if needed
python -m platformio run --target clean -e back_esp32
python -m platformio run -e back_esp32 -t upload
```

### 1.3 FRONT ESP32 (Motor Controller) - Upload Second

**Device:** Motor control slave  
**Upload Port:** COM port (auto-detected)  
**Serial Monitor:** COM port (auto-detected)
**Environment:** `front_esp32`

```bash
# Build and upload firmware to Front ESP32 (Motor Slave)
python -m platformio run -e front_esp32 -t upload

# Monitor serial output (run in separate terminal)
python -m platformio device monitor -e front_esp32

# Alternative: Build only (no upload)
python -m platformio run -e front_esp32

# Clean build if needed
python -m platformio run --target clean -e front_esp32
python -m platformio run -e front_esp32 -t upload
```

### 1.4 ESP32-CAM (Vision Module) - Upload Last

**⚠️ CRITICAL: ESP32-CAM requires special programming sequence**

**Device:** ESP32-CAM with camera and WiFi  
**Upload Port:** COM port (auto-detected)  
**Serial Monitor:** COM port (auto-detected)
**Environment:** `camera_esp32`

#### ESP32-CAM Upload Steps:

**Step 1: Prepare for Upload**

```bash
# Build firmware first
python -m platformio run -e camera_esp32
```

**Step 2: Connect GPIO0 to GND**

- Connect a jumper wire from GPIO0 to GND on ESP32-CAM
- This puts ESP32-CAM in programming mode

**Step 3: Upload Firmware**

```bash
# Upload while GPIO0 is connected to GND
python -m platformio run -e camera_esp32 -t upload
```

**Step 4: Complete Programming Sequence**

```bash
# After upload completes:
# 1. Disconnect GPIO0 from GND (leave floating)
# 2. Press RESET button
# 3. Monitor should show startup messages

python -m platformio device monitor -e camera_esp32
```

### 1.5 Upload All Devices Script

```bash
#!/bin/bash
# Complete upload sequence for Test Version 2

echo "=== PROJECT NIGHTFALL TEST V2 UPLOAD ==="
echo "Uploading all ESP32 devices..."

# Upload Rear ESP32 (Master)
echo "1. Uploading Rear ESP32 (Master)..."
python -m platformio run -e back_esp32 -t upload
if [ $? -eq 0 ]; then
    echo "✅ Rear ESP32 upload successful"
else
    echo "❌ Rear ESP32 upload failed"
    exit 1
fi

# Upload Front ESP32 (Motor Slave)
echo "2. Uploading Front ESP32 (Motor Slave)..."
python -m platformio run -e front_esp32 -t upload
if [ $? -eq 0 ]; then
    echo "✅ Front ESP32 upload successful"
else
    echo "❌ Front ESP32 upload failed"
    exit 1
fi

# Upload ESP32-CAM (Vision)
echo "3. Uploading ESP32-CAM (Vision)..."
echo "⚠️  Remember: Connect GPIO0 to GND during upload!"
python -m platformio run -e camera_esp32 -t upload
if [ $? -eq 0 ]; then
    echo "✅ ESP32-CAM upload successful"
else
    echo "❌ ESP32-CAM upload failed"
    exit 1
fi

echo "🎉 All devices uploaded successfully!"
echo ""
echo "Next steps:"
echo "1. Disconnect GPIO0 from GND on ESP32-CAM"
echo "2. Press RESET on all devices"
echo "3. Connect to WiFi: ProjectNightfall"
echo "4. Open dashboard: http://192.168.4.1"
```

---

## 2. Expected Behavior After Upload

### 2.1 Rear ESP32 (Master Controller) Startup Sequence

**Power-On Behavior:**

```
╔═══════════════════════════════════════════╗
║     PROJECT NIGHTFALL REAR ESP32          ║
║           Master Controller + Motors      ║
║              Version 2.0.0                ║
╚═══════════════════════════════════════════╝

Initializing hardware...
Hardware initialized
Setting up WiFi Access Point...
Access Point IP: 192.168.4.1
SSID: ProjectNightfall
Password: rescue2025
Setting up Web Server and WebSocket Server...
LittleFS Mount Failed  [← Expected if no files uploaded]
Web server started on port 80
WebSocket server started on port 8888
LittleFS initialized - serving dashboard from /index.html

✅ REAR ESP32 Master Controller Ready!
🎯 Controlling Motors 5 & 6 (Rear Left & Right)
WiFi AP: ProjectNightfall
WebSocket Server: Port 8888

Dashboard URL: http://192.168.4.1
```

**LED Indicators:**

- **Status LED (GPIO2):** Should be ON (solid blue)
- **Error LED (GPIO0):** Should be OFF

### 2.2 Front ESP32 (Motor Controller) Startup Sequence

**Power-On Behavior:**

```
[Front ESP32 startup messages]
UART communication initialized
Motor control ready
WiFi client connecting to ProjectNightfall...
WiFi connected to 192.168.4.2
Waiting for commands from Master...
```

**LED Indicators:**

- **Power LED:** Should be ON
- **Status LED:** Should blink during WiFi connection

### 2.3 ESP32-CAM (Vision Module) Startup Sequence

**Power-On Behavior (after GPIO0 disconnected and reset):**

```
[ESP32-CAM startup messages]
Camera initialized
WiFi client connecting to ProjectNightfall...
WiFi connected to 192.168.4.3
WebSocket client connecting to 192.168.4.1:8888...
Camera stream server started on port 81
System ready for vision processing
```

**LED Indicators:**

- **Status LED (GPIO33):** Should blink during startup, then steady
- **Flash LED (GPIO4):** Available for illumination

### 2.4 WiFi Network Activation

**Access Point Details:**

- **SSID:** `ProjectNightfall`
- **Password:** `rescue2025`
- **IP Range:** `192.168.4.0/24`
- **Gateway:** `192.168.4.1`

**Expected Device Connections:**

- **Rear ESP32:** `192.168.4.1` (Gateway/AP)
- **Front ESP32:** `192.168.4.2` (Static IP)
- **ESP32-CAM:** `192.168.4.3` (Static IP)

---

## 3. WiFi Startup Verification

### 3.1 Network Connection Verification

**Check WiFi Network:**

```bash
# On your computer/phone, scan for WiFi networks
# Look for "ProjectNightfall" network
# Password: rescue2025
```

**Expected Network Info:**

```
Network Name: ProjectNightfall
Security: WPA2-PSK
IP Range: 192.168.4.0/24
Channel: 1 (2.4GHz)
```

### 3.2 Device IP Assignment

**Connect to ProjectNightfall network and check IP assignment:**

**Rear ESP32 (Master):**

- **IP:** `192.168.4.1`
- **Services:** HTTP (80), WebSocket (8888)

**Front ESP32 (Motor):**

- **IP:** `192.168.4.2`
- **Services:** WiFi Client, UART Communication

**ESP32-CAM (Vision):**

- **IP:** `192.168.4.3`
- **Services:** HTTP Stream (81), WebSocket Client

### 3.3 Dashboard Access Verification

**Primary Dashboard:**

```
URL: http://192.168.4.1
Status: Should load Project Nightfall Dashboard
Features: Real-time telemetry, motor control, system status
```

**Camera Stream:**

```
URL: http://192.168.4.1/camera
Status: Should show live camera feed from ESP32-CAM
URL: http://192.168.4.3:81/stream (direct camera stream)
```

**API Endpoints:**

```
http://192.168.4.1/api/status     - System status JSON
http://192.168.4.1/api/telemetry  - Telemetry data JSON
http://192.168.4.1/api/devices    - Device connection status
```

---

## 4. Verification Steps

### 4.1 Serial Monitor Verification Commands

**Rear ESP32 Serial Monitor:**

```bash
python -m platformio device monitor -e back_esp32
```

**Expected Output Patterns:**

```
Telemetry: dist=45.2cm, gas=150, emergency=NO
WebSocket client 0 connected
Command: forward
Emergency stop reset - system resumed
```

**Front ESP32 Serial Monitor:**

```bash
python -m platformio device monitor -e front_esp32
```

**Expected Output Patterns:**

```
Motor command received: L=150, R=150
Heartbeat sent to Master
WiFi connected to 192.168.4.2
UART communication established
```

**ESP32-CAM Serial Monitor:**

```bash
python -m platformio device monitor -e camera_esp32
```

**Expected Output Patterns:**

```
Camera initialized successfully
WiFi connected to 192.168.4.3
WebSocket connected to Master
Heartbeat sent: Camera online
```

### 4.2 WiFi Network Verification

**Check Connected Devices:**

```bash
# On computer connected to ProjectNightfall
ping 192.168.4.1  # Rear ESP32
ping 192.168.4.2  # Front ESP32
ping 192.168.4.3  # ESP32-CAM
```

**Expected Results:**

```
PING 192.168.4.1: 64 bytes from 192.168.4.1
PING 192.168.4.2: 64 bytes from 192.168.4.2
PING 192.168.4.3: 64 bytes from 192.168.4.3
```

### 4.3 Dashboard Functionality Testing

**Test Dashboard Features:**

1. **Main Dashboard:** `http://192.168.4.1`

   - [ ] Page loads successfully
   - [ ] Real-time telemetry updates
   - [ ] Motor control buttons work
   - [ ] Emergency stop button responds

2. **Camera Stream:** `http://192.168.4.1/camera`

   - [ ] Live video feed appears
   - [ ] Image updates every 1-2 seconds
   - [ ] No connection errors

3. **API Testing:**
   ```bash
   curl http://192.168.4.1/api/status
   curl http://192.168.4.1/api/telemetry
   curl http://192.168.4.1/api/devices
   ```

### 4.4 Communication Verification

**Test UART Communication:**

1. Send motor command from dashboard
2. Check Front ESP32 serial monitor for command reception
3. Verify motor response and heartbeat

**Test WebSocket Communication:**

1. Open browser developer tools (Network tab)
2. Connect to dashboard
3. Watch WebSocket connection to `ws://192.168.4.1:8888`
4. Verify real-time telemetry messages

---

## 5. Troubleshooting Guide

### 5.1 Upload Issues

**Problem: "Board not found"**

```bash
# Solution: Check USB connections and ports
python -m platformio device list

# Check if devices are detected:
# COM ports will be listed (Windows format)
# Use the correct COM port for each device
```

**Problem: ESP32-CAM upload fails**

```bash
# Critical: Ensure GPIO0 is connected to GND during upload
# Step 1: Connect GPIO0 to GND with jumper wire
# Step 2: Press RESET button
# Step 3: Run upload command
python -m platformio run -e camera_esp32 -t upload

# After upload:
# Step 1: Disconnect GPIO0 from GND
# Step 2: Press RESET
# Step 3: Monitor should show startup
```

**Problem: Build errors**

```bash
# Clean build environment
python -m platformio run --target clean
python -m platformio run -e [environment] -t upload
```

### 5.2 Serial Port Selection Issues

**Problem: Wrong serial port**

```bash
# List all available ports
python -m platformio device list

# Manually specify port if needed
python -m platformio device monitor --port /dev/ttyUSB0 -e back_esp32
python -m platformio run -e back_esp32 --upload-port /dev/ttyUSB0 -t upload
```

**Problem: Permission denied**

```bash
# Linux/Mac: Add user to dialout group
sudo usermod -a -G dialout $USER

# Or use sudo (not recommended for development)
sudo python -m platformio device monitor -e back_esp32
```

### 5.3 Board Detection Problems

**Problem: ESP32 not detected**

```bash
# Check board connections:
# 1. USB cable data+power capable (not charge-only)
# 2. Correct GPIO pins for programming
# 3. Proper power supply (5V recommended)

# Try different USB port
# Try different USB cable
```

**Problem: Wrong board type**

```bash
# Verify platformio.ini environment (Windows commands):
[env:back_esp32]
platform = espressif32
board = esp32dev

[env:camera_esp32]
platform = espressif32
board = esp32cam  # Different board type
```

### 5.4 WiFi Connection Issues

**Problem: "ProjectNightfall" network not visible**

```bash
# Check Rear ESP32 serial monitor for:
# - WiFi Access Point creation messages
# - IP address assignment (192.168.4.1)
# - Error messages

# Verify GPIO pins:
# - Antenna connection (if external)
# - Power supply stability
```

**Problem: Devices can't connect to WiFi**

```bash
# Check Front ESP32 and ESP32-CAM serial monitors:
# Look for WiFi connection attempts
# Verify SSID and password match exactly
# Check signal strength and interference
```

**Problem: Dashboard not accessible**

```bash
# Verify IP connectivity:
ping 192.168.4.1

# Check if HTTP server started:
telnet 192.168.4.1 80

# Check WebSocket server:
telnet 192.168.4.1 8888
```

### 5.5 Communication Problems

**Problem: UART communication fails**

```bash
# Verify UART wiring:
# Rear GPIO16(TX) → Front GPIO17(RX)
# Rear GPIO17(RX) ← Front GPIO16(TX)
# Common Ground connection

# Check serial monitor on both devices
# Look for startup communication messages
```

**Problem: WebSocket disconnections**

```bash
# Check network stability
# Verify IP address conflicts
# Monitor serial output for connection errors
# Check firewall settings on client device
```

### 5.6 Motor Control Issues

**Problem: Motors not responding**

```bash
# Check motor driver connections:
# L298N power supply (12V battery)
# PWM pins connected correctly
# Direction pins (IN1-IN4) wiring
# Motor encoder connections (if present)

# Verify Front ESP32 serial monitor shows commands
# Check Rear ESP32 motor control output
```

**Problem: Erratic motor behavior**

```bash
# Check power supply voltage (should be 12V)
# Verify PWM signal quality on oscilloscope
# Check for electromagnetic interference
# Verify proper grounding (star ground configuration)
```

---

## 6. Critical Safety Notes

### 6.1 ESP32 GPIO Voltage Safety

**⚠️ CRITICAL WARNING:**

- **ESP32 GPIO pins are 3.3V ONLY** - NOT 5V tolerant
- **NEVER** connect 5V directly to GPIO pins
- **NEVER** connect sensor outputs directly without level shifting
- ESP32 accepts 5V on VIN/USB pins (built-in regulator)
- Always verify voltage levels before connection

### 6.2 ESP32-CAM Programming Sequence

**⚠️ CRITICAL UPLOAD SEQUENCE:**

1. **During Upload:** Connect GPIO0 to GND
2. **After Upload:** Disconnect GPIO0 (leave floating)
3. **Normal Operation:** GPIO0 should be floating
4. **Reset:** Press RESET button after GPIO0 changes

### 6.3 Power System Safety

**Power Distribution:**

- **14.8V Battery Pack:** High voltage - use proper fuses
- **5V Rail:** ESP32s and sensors - verify LM2596 output
- **3.3V Logic:** GPIO communication - verify buck converter
- **Common Ground:** Star ground configuration recommended

### 6.4 Motor System Safety

**Emergency Procedures:**

1. **Emergency Stop:** Dashboard button or proximity/gas triggers
2. **Manual Override:** Disconnect battery pack
3. **Software Reset:** Press RESET on all ESP32s
4. **Hardware Reset:** Power cycle entire system

---

## Quick Reference Commands

### Upload All Devices

```bash
# Complete upload sequence
python -m platformio run -e back_esp32 -t upload
python -m platformio run -e front_esp32 -t upload
python -m platformio run -e camera_esp32 -t upload
```

### Monitor All Devices

```bash
# Separate terminals for each device
python -m platformio device monitor -e back_esp32    # Terminal 1
python -m platformio device monitor -e front_esp32   # Terminal 2
python -m platformio device monitor -e camera_esp32  # Terminal 3
```

### Test Network Connectivity

```bash
# Ping all devices
ping 192.168.4.1  # Rear ESP32
ping 192.168.4.2  # Front ESP32
ping 192.168.4.3  # ESP32-CAM
```

### Dashboard Access

```
Primary: http://192.168.4.1
Camera:  http://192.168.4.1/camera
API:     http://192.168.4.1/api/status
```

---

**End of PlatformIO Upload Guide for Test Version 2**
