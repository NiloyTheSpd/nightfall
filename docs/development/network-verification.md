# Network Connectivity Verification - Project Nightfall ESP32

## Network Topology Overview

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

## Step 1: Hardware Connection Verification

### Required Connections

#### BACK ESP32 ↔ FRONT ESP32 (UART)

- **BACK ESP32 GPIO16 (TX)** → **FRONT ESP32 GPIO16 (RX)**
- **BACK ESP32 GPIO17 (RX)** → **FRONT ESP32 GPIO17 (TX)**
- **Common GND** between devices

#### ESP32-CAM Programming Setup

- **GPIO0 → GND** (during upload only)
- **GPIO0 floating** (during normal operation)
- **RESET button** accessible

### Verification Commands

```bash
# Check all connected USB devices
pio device list

# Expected output:
# /dev/ttyUSB0  -> BACK ESP32 (Master)
# /dev/ttyUSB1  -> FRONT ESP32 (Motor Slave)
# /dev/ttyUSB2  -> CAMERA ESP32 (Camera)
```

## Step 2: WiFi Network Verification

### 2.1 BACK ESP32 Access Point Setup

**Procedure**:

1. Upload firmware to BACK ESP32
2. Monitor serial output
3. Verify AP creation

```bash
# Upload and monitor BACK ESP32
pio run -e back_esp32 -t upload
pio device monitor -e back_esp32

# Expected startup messages:
# ╔═══════════════════════════════════════════╗
# ║     PROJECT NIGHTFALL BACK ESP32          ║
# ║           Master/Brain Controller         ║
# ║              Version 2.0.0                ║
# ╚═══════════════════════════════════════════╝
# Setting up WiFi Access Point...
# Access Point IP: 192.168.4.1
# SSID: ProjectNightfall
# Password: rescue2025
# Web server started on port 80
# WebSocket server started on port 8888
# ✅ BACK ESP32 Master Controller Ready!
```

**Verification Steps**:

1. **Check WiFi Networks**: Scan for "ProjectNightfall" network
2. **Connect**: Password "rescue2025"
3. **Verify IP**: Should get 192.168.4.x IP address
4. **Test HTTP**: Visit `http://192.168.4.1` → Should show robot dashboard

### 2.2 CAMERA ESP32 WiFi Client Connection

**Procedure**:

1. Upload firmware to CAMERA ESP32 (with GPIO0→GND)
2. Remove GPIO0→GND connection and press RESET
3. Monitor serial output

```bash
# Upload CAMERA ESP32 (GPIO0->GND connected)
pio run -e camera_esp32 -t upload

# Remove GPIO0->GND, press RESET, then monitor
pio device monitor -e camera_esp32

# Expected startup messages:
# ╔═══════════════════════════════════════════╗
# ║     PROJECT NIGHTFALL CAMERA ESP32        ║
# ║            Telemetry Node                 ║
# ║              Version 2.0.0                ║
# ╚═══════════════════════════════════════════╝
# Initializing hardware...
# Setting up WiFi client connection...
# Connecting to WiFi.....
# WiFi connected successfully!
# IP Address: 192.168.4.x
# RSSI: -xx dBm
# Setting up WebSocket client...
# ✅ CAMERA ESP32 Telemetry Node Ready!
```

**Network Diagnostics**:

```bash
# In CAMERA ESP32 serial monitor, use commands:
status    # Full system status
wifi      # WiFi connection details
websocket # WebSocket connection status
reconnect # Attempt reconnection
```

## Step 3: UART Communication Verification

### 3.1 FRONT ESP32 Motor Slave Setup

```bash
# Upload and monitor FRONT ESP32
pio run -e front_esp32 -t upload
pio device monitor -e front_esp32

# Expected startup messages:
# ╔═══════════════════════════════════════════╗
# ║     PROJECT NIGHTFALL FRONT ESP32         ║
# ║            Motor Slave Controller         ║
# ║              Version 2.0.0                ║
# ╚═══════════════════════════════════════════╝
# Initializing motor control hardware...
# Motor control hardware initialized
# ✅ FRONT ESP32 Motor Slave Ready!
# Listening for UART commands on Serial2
```

### 3.2 UART Communication Test

**From BACK ESP32 serial monitor**:

```bash
# Send motor commands manually
# Forward: {"L":150,"R":150}
# Stop: {"L":0,"R":0}
# Emergency: {"cmd":"emergency_stop"}
# Test: {"cmd":"test"}

# Expected FRONT ESP32 responses:
# UART Command received: {"L":150,"R":150}
# Motor speeds updated - Left: 150, Right: 150
# Heartbeat sent - Emergency: NO, Left Speed: 150, Right Speed: 150
```

**From FRONT ESP32 serial monitor**:

```bash
# Use built-in commands
status    # Show current motor speeds and UART status
forward   # Test forward movement
stop      # Stop motors
test      # Run motor test sequence
emergency # Trigger emergency stop
reset     # Reset emergency stop
help      # Show available commands
```

### 3.3 UART Troubleshooting

**Common Issues**:

1. **No Communication**:

   - Check GPIO16/17 cross-connections
   - Verify common GND
   - Check UART baudrate (115200)

2. **Motor Commands Not Executing**:

   - Monitor FRONT ESP32 for command reception
   - Check emergency stop status
   - Verify JSON parsing

3. **Heartbeat Missing**:
   - FRONT ESP32 should send heartbeat every 1000ms
   - Check for UART timeout (1000ms emergency stop)

## Step 4: WebSocket Communication Verification

### 4.1 BACK ESP32 WebSocket Server

**Dashboard Connection Test**:

1. Connect to WiFi "ProjectNightfall"
2. Open browser to `http://192.168.4.1`
3. Check WebSocket connection in browser console
4. Verify telemetry updates every 500ms

**Expected Dashboard Data**:

- Front Distance: X cm
- Rear Distance: X cm
- Gas Level: XXX
- Battery: 14.8 V
- Emergency: NO
- Robot State: READY

### 4.2 CAMERA ESP32 WebSocket Client

**From CAMERA ESP32 serial monitor**:

```bash
# Check WebSocket status
websocket

# Expected output:
# WebSocket Status: Connected
# Host: 192.168.4.1:8888

# Force heartbeat
heartbeat

# Expected telemetry:
# {"type":"heartbeat","source":"camera","timestamp":12345,"uptime":12345,"wifi_rssi":-45,"wifi_ip":"192.168.4.2","memory_free":123456,"boot_count":1,"version":"2.0.0"}
```

### 4.3 WebSocket Message Flow

**BACK ESP32 → WebSocket Clients**:

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

**CAMERA ESP32 → BACK ESP32**:

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

**BACK ESP32 ← FRONT ESP32 (UART)**:

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

## Step 5: End-to-End System Test

### 5.1 Complete Startup Sequence

1. **Power on all devices**
2. **Verify BACK ESP32 AP** (check WiFi networks)
3. **Connect to "ProjectNightfall"** (password: rescue2025)
4. **Verify CAMERA ESP32 connection** (serial monitor shows WiFi connected)
5. **Test WebSocket dashboard** (visit http://192.168.4.1)
6. **Test UART communication** (send motor commands from BACK ESP32)

### 5.2 Functional Tests

**Motor Control Test**:

```bash
# From BACK ESP32 dashboard or serial monitor:
forward    # Should see FRONT ESP32 execute movement
left       # Should turn left
right      # Should turn right
backward   # Should move backward
stop       # Should stop all motors
```

**Safety System Test**:

```bash
# Place obstacle <20cm in front of ultrasonic sensor
# Should trigger emergency stop automatically
# Buzzer should sound
# Dashboard should show "EMERGENCY: YES"
```

**Communication Test**:

```bash
# Disconnect UART cables
# FRONT ESP32 should enter emergency stop after 1000ms
# Dashboard should show emergency state

# Reconnect UART cables
# Send emergency_reset command
# System should resume normal operation
```

## Step 6: Network Diagnostics Tools

### 6.1 Serial Command Reference

**BACK ESP32 Commands** (via serial monitor):

- Manual motor control via JSON commands
- System status queries
- Emergency controls

**FRONT ESP32 Commands**:

- `status` - Show motor speeds and UART status
- `forward`/`left`/`right`/`backward` - Test movement
- `stop` - Stop all motors
- `emergency` - Trigger emergency stop
- `reset` - Reset emergency stop
- `test` - Run motor test sequence

**CAMERA ESP32 Commands**:

- `status` - Full system status
- `wifi` - WiFi connection details
- `websocket` - WebSocket connection status
- `flash` - Test flash LED
- `heartbeat` - Send manual heartbeat
- `reconnect` - Attempt reconnection

### 6.2 Network Monitoring

**WiFi Network Analysis**:

```bash
# Scan for ProjectNightfall network
# Check signal strength (RSSI)
# Verify client count (should show CAMERA ESP32)

# From a computer connected to the network:
ping 192.168.4.1    # Test BACK ESP32 connectivity
nmap -p 80,8888 192.168.4.1  # Check open ports
```

**WebSocket Testing**:

```javascript
// Browser console test:
var ws = new WebSocket("ws://192.168.4.1:8888");
ws.onopen = function () {
  console.log("Connected");
};
ws.onmessage = function (event) {
  console.log("Data:", event.data);
};
ws.send(JSON.stringify({ command: "status" }));
```

## Troubleshooting Quick Reference

| Issue                    | Symptoms                          | Solution                              |
| ------------------------ | --------------------------------- | ------------------------------------- |
| No WiFi AP               | BACK ESP32 doesn't create network | Check GPIO pins, verify power         |
| CAMERA won't connect     | No WiFi client connection         | Check SSID/password, signal strength  |
| UART no response         | Motor commands ignored            | Check GPIO16/17 connections, GND      |
| WebSocket fails          | Dashboard shows disconnected      | Check IP/port, firewall settings      |
| ESP32-CAM won't upload   | Programming fails                 | Ensure GPIO0→GND during upload        |
| Emergency stop always on | System stuck in emergency         | Check sensor connections, gas levels  |
| Motors don't move        | No motor response                 | Check motor driver power, connections |

## Performance Benchmarks

**Expected Response Times**:

- UART Command → Motor Response: <50ms
- WebSocket Telemetry: Every 500ms
- WiFi Connection: <10 seconds
- Emergency Stop Reaction: <100ms

**Network Reliability**:

- UART: 100% reliable (wired)
- WiFi: 95%+ reliable (depends on environment)
- WebSocket: Auto-reconnect every 5 seconds
