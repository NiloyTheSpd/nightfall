# Step-by-Step Troubleshooting Guide - Project Nightfall ESP32

## Quick Reference

| Issue                    | Start Here          | Document Section |
| ------------------------ | ------------------- | ---------------- |
| Can't upload firmware    | Hardware Check      | Section 1.1      |
| WiFi AP not visible      | Network Setup       | Section 2.1      |
| UART communication fails | Hardware Connection | Section 3.1      |
| Motors don't respond     | Motor Control       | Section 4.1      |
| WebSocket disconnects    | Network Diagnostics | Section 5.1      |
| ESP32-CAM won't program  | ESP32-CAM Specific  | Section 6.1      |
| System freezes/crashes   | System Recovery     | Section 7.1      |

## 1. Firmware Upload Issues

### 1.1 Hardware Connection Check

**Symptoms**: Upload fails, device not detected, timeout errors

**Step 1: Verify USB Connections**

```bash
# Check connected devices
pio device list

# Expected output:
# /dev/ttyUSB0  -> BACK ESP32 (Master)
# /dev/ttyUSB1  -> FRONT ESP32 (Motor Slave)
# /dev/ttyUSB2  -> CAMERA ESP32 (Camera)
```

**If no devices found**:

1. Check USB cable (data cable, not charge-only)
2. Try different USB port
3. Check device manager/lsusb for device recognition
4. Install FTDI/CH340 drivers if needed

**Step 2: Verify Power and Boot States**

For each ESP32:

1. **Check Power LED**: Should be lit when USB connected
2. **Check Boot Mode**: GPIO0 should be HIGH (floating) for normal operation
3. **Check Reset**: Press RESET button, device should restart

**Step 3: PlatformIO Environment Verification**

```bash
# Test build for each environment
pio run -e back_esp32      # Test master build
pio run -e front_esp32     # Test slave build
pio run -e camera_esp32    # Test camera build

# Check for compilation errors
# Look for missing dependencies or syntax errors
```

### 1.2 ESP32-CAM Specific Upload Issues

**Critical**: ESP32-CAM requires special programming sequence

**Step 1: Upload Preparation**

```bash
# 1. Connect GPIO0 to GND with jumper wire
# 2. Connect ESP32-CAM to USB
# 3. Verify device is detected
pio device list | grep ttyUSB
```

**Step 2: Upload with GPIO0 Grounded**

```bash
# Upload firmware (keep GPIO0->GND connected)
pio run -e camera_esp32 -t upload

# Watch for upload progress
# Should see: "Leaving... Done!"
```

**Step 3: Post-Upload Sequence**

```bash
# 1. IMMEDIATELY disconnect GPIO0 from GND
# 2. Press RESET button on ESP32-CAM
# 3. Monitor serial output
pio device monitor -e camera_esp32
```

**Expected Startup**:

```
╔═══════════════════════════════════════════╗
║     PROJECT NIGHTFALL CAMERA ESP32        ║
║            Telemetry Node                 ║
╚═══════════════════════════════════════════╝
```

**If ESP32-CAM won't start after upload**:

1. Double-check GPIO0 is floating (not connected to GND)
2. Press RESET button
3. Check power supply (ESP32-CAM can be power-hungry)
4. Try different USB cable/port

## 2. WiFi Network Issues

### 2.1 BACK ESP32 Access Point Not Visible

**Symptoms**: "ProjectNightfall" network not found, no WiFi AP

**Step 1: Verify BACK ESP32 Operation**

```bash
# Monitor BACK ESP32 serial output
pio device monitor -e back_esp32

# Expected startup sequence:
# ╔═══════════════════════════════════════════╗
# ║     PROJECT NIGHTFALL BACK ESP32          ║
# ║           Master/Brain Controller         ║
# ╚═══════════════════════════════════════════╝
# Initializing hardware...
# Setting up WiFi Access Point...
# Access Point IP: 192.168.4.1
# SSID: ProjectNightfall
# Password: rescue2025
# ✅ BACK ESP32 Master Controller Ready!
```

**If WiFi setup messages missing**:

1. Check for runtime errors in serial output
2. Verify power supply (insufficient power can disable WiFi)
3. Check for memory allocation failures
4. Try rebuilding and re-uploading

**Step 2: WiFi Network Verification**

```bash
# From a computer/phone:
# 1. Scan for WiFi networks
# 2. Look for "ProjectNightfall" network
# 3. Check signal strength
```

**If network not visible**:

1. **Check WiFi channel**: Some channels may be restricted in your region
2. **Check interference**: Other 2.4GHz networks may interfere
3. **Verify antenna**: Ensure ESP32 has antenna connected
4. **Distance test**: Move closer to ESP32

**Step 3: Manual Connection Test**

```bash
# Try connecting with:
# SSID: ProjectNightfall
# Password: rescue2025
# Security: WPA2
```

**If connection fails**:

1. **Password verification**: Ensure password is exactly "rescue2025"
2. **DHCP issues**: Check if IP assignment fails
3. **Firewall**: Disable firewall temporarily for testing
4. **Device compatibility**: Test with different devices

### 2.2 CAMERA ESP32 WiFi Connection Issues

**Symptoms**: Camera connects to WiFi but can't reach WebSocket server

**Step 1: Check CAMERA ESP32 WiFi Status**

```bash
# In CAMERA ESP32 serial monitor
wifi

# Expected output:
# WiFi Status: Connected
# IP: 192.168.4.x
# RSSI: -xx dBm
```

**Step 2: Verify WebSocket Connection**

```bash
# In CAMERA ESP32 serial monitor
websocket

# Expected output:
# WebSocket Status: Connected
# Host: 192.168.4.1:8888
```

**If WiFi connects but WebSocket fails**:

1. **Check BACK ESP32 WebSocket server**: Verify it's running
2. **Check IP address**: CAMERA should connect to 192.168.4.1
3. **Check port**: Should be 8888
4. **Network traffic**: Use WiFi analyzer to check traffic

**Step 3: Manual Reconnection**

```bash
# In CAMERA ESP32 serial monitor
reconnect

# Should see:
# Attempting WiFi reconnection...
# WiFi connected successfully!
# Attempting WebSocket connection...
# WebSocket connected to master ESP32
```

## 3. UART Communication Issues

### 3.1 No Motor Response from FRONT ESP32

**Symptoms**: BACK ESP32 sends commands but FRONT ESP32 doesn't respond

**Step 1: Verify Hardware Connections**

```
BACK ESP32 GPIO16 (TX) ←→ FRONT ESP32 GPIO16 (RX)
BACK ESP32 GPIO17 (RX) ←→ FRONT ESP32 GPIO17 (TX)
Common GND between devices
```

**Check with multimeter**:

1. **Continuity test**: Verify connections between pins
2. **Voltage levels**: Should be 0V (LOW) or 3.3V (HIGH)
3. **No shorts**: Check for unintended connections

**Step 2: Monitor Both Serial Outputs**

**Terminal 1 - BACK ESP32**:

```bash
pio device monitor -e back_esp32

# Send test command:
# {"L":100,"R":100}

# Expected output:
# UART Command received: {"L":100,"R":100}
# Motor speeds updated - Left: 100, Right: 100
```

**Terminal 2 - FRONT ESP32**:

```bash
pio device monitor -e front_esp32

# Expected when BACK ESP32 sends command:
# UART Command received: {"L":100,"R":100}
# Motor speeds updated - Left: 100, Right: 100
# Heartbeat sent - Emergency: NO, Left Speed: 100, Right Speed: 100
```

**If FRONT ESP32 doesn't receive commands**:

1. **Check serial output**: FRONT ESP32 should show "Listening for UART commands on Serial2"
2. **Check baudrate**: Both devices must use 115200 baud
3. **Check UART pins**: Verify GPIO16/17 connections
4. **Check GND**: Common ground is essential

**Step 3: Test UART Loopback**

```bash
# Connect FRONT ESP32 GPIO16 to GPIO17 (loopback)
# Run this command in FRONT ESP32 serial monitor
# You should see echo of commands

# If no echo:
# - Hardware issue with UART pins
# - Faulty ESP32
# - Connection problem
```

### 3.2 Intermittent UART Communication

**Symptoms**: Commands work sometimes, random failures

**Step 1: Check for Signal Quality Issues**

```bash
# Monitor for JSON parse errors in both devices:
# "JSON parse error in motor command"
# "JSON parse error"

# This indicates data corruption or noise
```

**Common causes**:

1. **Loose connections**: Check all wire connections
2. **Electromagnetic interference**: Keep away from motors/power supplies
3. **Long wires**: Keep UART wires short (<30cm)
4. **Ground loops**: Ensure solid ground connection

**Step 2: Check for Timeout Issues**

```bash
# In FRONT ESP32 serial monitor, check for:
# "⚠️ UART timeout - No data for 1000ms, entering emergency stop"

# This means BACK ESP32 stopped sending commands
```

**If timeout occurs**:

1. **BACK ESP32 status**: Check if BACK ESP32 is still running
2. **Motor command frequency**: Commands should be sent regularly
3. **Emergency stop**: May be triggered by safety conditions

## 4. Motor Control Issues

### 4.1 Motors Don't Move

**Symptoms**: Commands received but no motor movement

**Step 1: Check Motor Power Supply**

```
14.8V Battery Pack → Motor Driver Input
Check voltage with multimeter: Should be 14.8V ± 0.5V
```

**Step 2: Check Motor Driver Connections**

```
Motor Driver Inputs:
- PWM signals from ESP32 (0-255, ~3.3V)
- Direction signals (HIGH/LOW, 3.3V)
- Enable signals if present
```

**Step 3: Test Motor Drivers Directly**

```bash
# In FRONT ESP32 serial monitor
test

# This should run motor test sequence:
# Forward → Turn → Reverse → Stop

# If no movement:
# 1. Check motor power supply
# 2. Check motor driver connections
# 3. Check individual motors
# 4. Verify motor driver enable pins
```

**Step 4: Check Emergency Stop Status**

```bash
# In FRONT ESP32 serial monitor
status

# Check "Emergency Stop" field
# If YES, motors will not respond to commands

# To reset:
reset
```

### 4.2 Erratic Motor Behavior

**Symptoms**: Motors move inconsistently, wrong directions, stuttering

**Step 1: Check Motor Command Values**

```bash
# Monitor motor speeds in serial output:
# "Motor speeds updated - Left: 150, Right: 150"

# Verify values are reasonable (-255 to 255)
# Negative values = reverse, positive = forward
```

**Step 2: Check PWM Signal Quality**

```bash
# Use oscilloscope or logic analyzer on PWM pins:
# - GPIO13 (Left Motor PWM)
# - GPIO25 (Right Motor PWM)
# - GPIO14 (Motor 2 Left PWM)
# - GPIO15 (Motor 2 Right PWM)

# Should show PWM signal with correct duty cycle
```

**Common issues**:

1. **PWM frequency**: Should be ~500Hz
2. **Signal interference**: Keep PWM wires short
3. **Power supply noise**: Use proper filtering
4. **Motor driver damage**: May cause erratic behavior

## 5. WebSocket Communication Issues

### 5.1 Dashboard Won't Connect

**Symptoms**: Web page loads but shows "Disconnected"

**Step 1: Verify BACK ESP32 WebSocket Server**

```bash
# In BACK ESP32 serial monitor
# Should see:
# WebSocket server started on port 8888

# Check for WebSocket events:
# "WebSocket client X connected"
# "WebSocket client X disconnected"
```

**Step 2: Browser Console Check**

```javascript
// Open browser console (F12)
// Look for WebSocket connection errors:
// "WebSocket connection to ws://192.168.4.1:8888 failed"
```

**Step 3: Network Connectivity Test**

```bash
# From browser console:
var ws = new WebSocket('ws://192.168.4.1:8888');
ws.onopen = function() { console.log('Connected'); };
ws.onerror = function(e) { console.log('Error:', e); };

# Should show "Connected" message
```

**If connection fails**:

1. **IP address**: Verify device IP is 192.168.4.1
2. **Firewall**: Check browser/system firewall settings
3. **Port**: Ensure WebSocket port 8888 is open
4. **Browser compatibility**: Test with different browsers

### 5.2 WebSocket Disconnects Frequently

**Symptoms**: Connection established but drops repeatedly

**Step 1: Check Signal Quality**

```bash
# Monitor WebSocket events in BACK ESP32 serial output
# Look for pattern:
# "WebSocket client 0 connected"
# "WebSocket client 0 disconnected"
# "WebSocket client 0 connected"
# "WebSocket client 0 disconnected"
```

**Step 2: Check CAMERA ESP32 Heartbeat**

```bash
# In CAMERA ESP32 serial monitor
# Should see heartbeat every 5 seconds:
# "Heartbeat sent - WiFi RSSI: -45 dBm, Free Memory: 123456 bytes"
```

**If heartbeat missing**:

1. **WiFi connection**: Check WiFi status
2. **WebSocket client**: Verify WebSocket connection
3. **Memory issues**: Check available heap memory

**Step 3: Network Traffic Analysis**

```bash
# Use WiFi analyzer to check:
# - Network congestion
# - Signal strength
# - Interference sources

# Strong signal (-50 to -60 dBm) is ideal
# Weak signal (-80 dBm or lower) causes disconnects
```

## 6. ESP32-CAM Specific Issues

### 6.1 ESP32-CAM Won't Enter Download Mode

**Symptoms**: Upload fails, "Failed to connect to ESP32"

**Step 1: Verify GPIO0 Connection**

```
ESP32-CAM Programming Setup:
- GPIO0 must be connected to GND during upload
- All other connections should be secure
- USB cable must provide sufficient power
```

**Step 2: Check Power Supply**

```
ESP32-CAM Power Requirements:
- USB 5V input (via onboard regulator)
- Can consume up to 320mA during startup
- Use quality USB cable and port
```

**Step 3: Manual Reset Sequence**

```bash
# 1. Connect GPIO0 to GND
# 2. Press and hold RESET button
# 3. Start upload: pio run -e camera_esp32 -t upload
# 4. Release RESET button when upload starts
# 5. Keep GPIO0->GND connected until upload completes
```

### 6.2 ESP32-CAM Boot Loop

**Symptoms**: ESP32-CAM restarts continuously after programming

**Step 1: Check GPIO0 State**

```
Post-Programming Sequence:
1. Upload completes successfully
2. DISCONNECT GPIO0 from GND immediately
3. Press RESET button
4. ESP32 should boot normally

If GPIO0 remains connected to GND, device stays in download mode
```

**Step 2: Verify Boot Messages**

```bash
# After GPIO0 disconnected and reset pressed
pio device monitor -e camera_esp32

# Expected boot sequence:
# rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
# configsip: 0, SPIWP:0xee
# clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
# mode:DIO, clock div:2
# load:0x3fff0030,len:1184
# ...
# ╔═══════════════════════════════════════════╗
# ║     PROJECT NIGHTFALL CAMERA ESP32        ║
```

**If boot loop continues**:

1. **Flash corruption**: Re-upload firmware
2. **Power issues**: Check power supply stability
3. **Memory issues**: Check for memory leaks in code
4. **Hardware fault**: ESP32-CAM may be defective

## 7. System Recovery Procedures

### 7.1 Complete System Reset

**When to use**: Multiple devices failing, corrupted firmware, hardware issues

**Step 1: Power Down All Devices**

```
1. Disconnect all ESP32 devices from power
2. Disconnect all USB cables
3. Disconnect UART wires between devices
4. Wait 10 seconds
```

**Step 2: Hardware Verification**

```
1. Check all wire connections
2. Verify power supply voltages
3. Test individual components
4. Check for damaged components
```

**Step 3: Firmware Reinstallation**

```bash
# Upload in sequence:
# 1. BACK ESP32 (Master)
pio run -e back_esp32 -t upload
pio device monitor -e back_esp32

# 2. FRONT ESP32 (Motor Slave)
pio run -e front_esp32 -t upload
pio device monitor -e front_esp32

# 3. CAMERA ESP32 (with GPIO0->GND)
pio run -e camera_esp32 -t upload
# Remove GPIO0->GND, press reset
pio device monitor -e camera_esp32
```

**Step 4: Reconnect Communication**

```
1. Connect UART wires (GPIO16/17 between BACK and FRONT)
2. Verify WiFi AP is active ("ProjectNightfall")
3. Connect CAMERA ESP32 to WiFi
4. Test communication paths
```

### 7.2 Individual Device Recovery

**BACK ESP32 Issues**:

```bash
# Emergency recovery commands:
# From serial monitor:
emergency_reset     # Clear emergency stop
diagnostic          # Show system status
reset_errors        # Clear error counters
```

**FRONT ESP32 Issues**:

```bash
# Recovery commands:
reset               # Reset emergency stop
stop                # Stop all motors
status              # Check system state
uart_stats          # Check UART statistics
```

**CAMERA ESP32 Issues**:

```bash
# Recovery commands:
reconnect           # Attempt WiFi/WebSocket reconnection
wifi                # Check WiFi status
websocket           # Check WebSocket status
status              # Full system status
```

### 7.3 Memory and Performance Issues

**Symptoms**: System slow, crashes, memory allocation failures

**Step 1: Check Memory Usage**

```bash
# In any ESP32 serial monitor:
mem                 # Show memory status

# Check values:
# Free Heap: Should be >50KB
# Min Heap: Should not be decreasing rapidly
```

**Step 2: Monitor System Performance**

```bash
# Watch for warnings in serial output:
# "Low memory: X bytes remaining"
# "JSON parse error: Not enough memory"
# "Failed to allocate memory"
```

**Step 3: Optimize Code**

```cpp
// Common memory optimizations:
1. Reduce JSON document sizes
2. Use StaticJsonDocument instead of DynamicJsonDocument when possible
3. Clear unused variables
4. Avoid memory leaks in loops
5. Use smaller buffer sizes where appropriate
```

## 8. Advanced Diagnostics

### 8.1 Network Packet Analysis

**Tools needed**: WiFi analyzer, network monitoring software

**Step 1: Monitor WiFi Traffic**

```
1. Use WiFi analyzer app on phone/computer
2. Connect to "ProjectNightfall" network
3. Monitor for:
   - Excessive retransmissions
   - Network congestion
   - Interference from other networks
```

**Step 2: WebSocket Message Flow**

```javascript
// Browser console - monitor WebSocket messages:
var ws = new WebSocket("ws://192.168.4.1:8888");
ws.onmessage = function (event) {
  console.log("Received:", event.data);
  var data = JSON.parse(event.data);
  console.log("Type:", data.type);
  console.log("Timestamp:", data.timestamp);
};
```

### 8.2 Hardware Signal Analysis

**Tools needed**: Oscilloscope, logic analyzer

**Step 1: UART Signal Verification**

```
1. Connect scope to UART TX/RX lines
2. Send test commands
3. Verify signal characteristics:
   - Baud rate: 115200 Hz (≈8.68μs bit time)
   - Voltage levels: 0V (LOW), 3.3V (HIGH)
   - Signal integrity: Clean edges, no noise
```

**Step 2: PWM Signal Analysis**

```
1. Monitor motor PWM signals
2. Verify frequency and duty cycle
3. Check for signal distortion
4. Measure voltage levels
```

## 9. Prevention and Maintenance

### 9.1 Regular Maintenance Tasks

**Weekly Checks**:

1. **Power connections**: Check for corrosion or looseness
2. **Wire integrity**: Inspect UART and power cables
3. **Device logs**: Review serial output for warnings
4. **Network performance**: Check WiFi signal strength

**Monthly Tasks**:

1. **Firmware updates**: Check for bug fixes and improvements
2. **Hardware inspection**: Check for physical damage
3. **Performance review**: Monitor system responsiveness
4. **Backup configuration**: Save working configurations

### 9.2 Best Practices

**Development**:

1. **Incremental testing**: Test each component separately
2. **Version control**: Use git for firmware versions
3. **Documentation**: Keep detailed change logs
4. **Testing environment**: Maintain stable test setup

**Hardware**:

1. **Quality connections**: Use proper crimping and soldering
2. **Power management**: Use appropriate power supplies
3. **Signal integrity**: Keep sensitive wires short and shielded
4. **Environmental protection**: Protect from moisture and vibration

## 10. Emergency Contacts and Resources

### 10.1 Quick Reference Commands

```bash
# Essential diagnostic commands:
pio device list                    # List connected devices
pio device monitor -e <env>        # Monitor specific device
pio run -e <env> -t upload         # Upload firmware
diagnostic                         # Generate diagnostic report
status                             # Show system status
wifi                               # Check WiFi connection
uart_stats                         # Show UART statistics
mem                                # Show memory usage
reconnect                          # Attempt reconnection
reset                              # Reset emergency stop
```

### 10.2 Troubleshooting Decision Tree

```
System Issue?
├─ Firmware Upload Problem?
│  ├─ Device Not Detected → Check USB connections/drivers
│  ├─ Upload Timeout → Check GPIO0 (ESP32-CAM) / power supply
│  └─ Verification Failed → Check hardware connections
├─ Communication Problem?
│  ├─ WiFi Issues → Check AP setup / signal strength
│  ├─ UART Issues → Check wire connections / baud rate
│  └─ WebSocket Issues → Check server status / network
├─ Motor Control Problem?
│  ├─ No Movement → Check power supply / emergency stop
│  ├─ Erratic Behavior → Check PWM signals / interference
│  └─ Wrong Directions → Check motor wiring
└─ System Stability Problem?
   ├─ Crashes/Freezes → Check memory usage / power supply
   ├─ Disconnections → Check signal quality / interference
   └─ Performance Issues → Check system load / optimization
```

This comprehensive troubleshooting guide covers the most common issues encountered in the Project Nightfall ESP32 system. Always start with the simplest checks and work systematically toward more complex diagnostics.
