# Test Version 2 - Unified WiFi Networking Implementation

## Overview

Test Version 2 implements unified WiFi networking for the Project Nightfall rescue robot system, providing direct IP-based control through a browser dashboard accessible at 192.168.4.1.

## Architecture

### Network Topology

- **Rear ESP32 (Master)**: Access Point + Web Server (192.168.4.1)
- **Front ESP32 (Motor Slave)**: WiFi Client + UART Backup (192.168.4.2)
- **ESP-CAM (Vision)**: WiFi Client + Camera Stream (192.168.4.3)

### Communication Strategy

- **Primary**: WiFi/WebSocket for real-time control and telemetry
- **Backup**: UART for reliability when WiFi fails
- **Protocol**: JSON-based messaging over WebSocket and Serial

## Implementation Details

### 1. Simple HTML Dashboard (`data/index.html`)

- **Pure HTML/CSS/JavaScript** (no React, no build tools)
- **Responsive design** for mobile and desktop
- **Real-time telemetry** display with auto-refresh
- **Motor control buttons** (forward, backward, left, right, stop)
- **Emergency stop** functionality
- **Device connection status** indicators
- **Camera feed display** via iframe
- **WebSocket integration** for live updates

### 2. Rear ESP32 (Master Controller)

**Enhanced Features:**

- Serves HTML dashboard from LittleFS filesystem
- WiFi Access Point: "ProjectNightfall" / "rescue2025"
- WebSocket Server on port 8888
- REST API endpoints:
  - `GET /api/status` - System status
  - `GET /api/telemetry` - Real-time telemetry
  - `POST /api/motor` - Motor control
  - `GET /api/devices` - Device connection status
- Device heartbeat tracking and connection monitoring
- Hybrid communication: WiFi primary, UART backup

### 3. Front ESP32 (Motor Slave)

**WiFi Client Features:**

- Connects to rear ESP32 Access Point
- WebSocket client for real-time motor commands
- Heartbeat transmission every 1 second
- UART fallback when WiFi unavailable
- Emergency stop coordination
- Motor speed feedback to master

### 4. ESP-CAM (Vision Module)

**Enhanced WiFi Features:**

- Connects to ProjectNightfall network
- WebSocket client for telemetry
- Camera streaming capability (port 81)
- Status LED indicators for connection state
- Flash LED control via commands

## Configuration

### WiFi Network Settings

```cpp
#define WIFI_SSID "ProjectNightfall"
#define WIFI_PASSWORD "rescue2025"
#define REAR_ESP32_IP "192.168.4.1"
#define FRONT_ESP32_IP "192.168.4.2"
#define CAMERA_ESP32_IP "192.168.4.3"
```

### Build Environments

- `rear_esp32`: Master controller with filesystem support
- `front_esp32_v2`: Motor controller with WiFi client
- `camera_esp32_v2`: Camera module with streaming

## Key Features

### Safety Systems

- **Emergency Stop**: Immediate halt from dashboard
- **Distance Monitoring**: Ultrasonic sensors trigger auto-stop
- **Gas Detection**: MQ-2 sensor safety override
- **Connection Monitoring**: Device timeout detection

### Real-Time Communication

- **WebSocket Server**: Port 8888 for dashboard communication
- **Heartbeat System**: 1-second intervals for device status
- **Telemetry Broadcasting**: 500ms intervals for sensor data
- **Command Processing**: Real-time motor control via WiFi

### Dashboard Features

- **Live Telemetry**: Motor speeds, battery, sensors, uptime
- **Manual Control**: Joystick-style buttons with emergency stop
- **Device Status**: Connection indicators for all ESP32 modules
- **Camera Feed**: Embedded ESP-CAM stream display
- **Responsive UI**: Works on mobile devices and desktop

## Usage Instructions

### 1. Build and Upload

```bash
# Build and upload rear ESP32 (master)
pio run -e rear_esp32 -t upload
pio device monitor -e rear_esp32

# Build and upload front ESP32 (motor slave)
pio run -e front_esp32_v2 -t upload
pio device monitor -e front_esp32_v2

# Build and upload ESP-CAM (vision)
pio run -e camera_esp32_v2 -t upload
pio device monitor -e camera_esp32_v2
```

### 2. Access Dashboard

1. Connect to WiFi network "ProjectNightfall" with password "rescue2025"
2. Open browser and navigate to `http://192.168.4.1`
3. Dashboard will load with real-time telemetry

### 3. Control Robot

- Use manual control buttons for movement
- Monitor sensor data and device status
- Access camera feed for visual feedback
- Use emergency stop for immediate halt

## Technical Specifications

### WiFi Configuration

- **Access Point**: Rear ESP32 creates "ProjectNightfall" network
- **Channel**: 1 (configurable)
- **Security**: WPA2-PSK
- **IP Range**: 192.168.4.x

### Communication Protocol

- **WebSocket Port**: 8888
- **HTTP Port**: 80 (dashboard)
- **Camera Stream**: 81 (ESP-CAM)
- **Baud Rate**: 115200 (UART backup)

### Data Format

```json
{
  "type": "telemetry",
  "timestamp": 1234567890,
  "dist": 25.5,
  "rearDist": 30.2,
  "gas": 150,
  "battery": 14.8,
  "leftSpeed": 150,
  "rightSpeed": 150,
  "emergency": false,
  "devices": {
    "front": true,
    "camera": true,
    "wifi": true
  }
}
```

## Benefits of Test Version 2

1. **Unified Control**: Single dashboard for all robot functions
2. **Real-Time Feedback**: Live telemetry and status updates
3. **WiFi Reliability**: Hybrid communication with UART backup
4. **Mobile Support**: Responsive design works on phones/tablets
5. **Easy Access**: Direct IP access, no complex setup required
6. **Scalable**: Easy to add new devices to network
7. **Safety First**: Multiple emergency stop mechanisms
8. **Developer Friendly**: Simple HTML/JavaScript, no build tools

## Future Enhancements

1. **Camera Streaming**: MJPEG stream from ESP-CAM
2. **Autonomous Mode**: Path planning and obstacle avoidance
3. **Data Logging**: Telemetry storage and analysis
4. **Remote Access**: Internet connectivity for remote control
5. **Multi-Robot**: Support for multiple robot coordination
6. **Advanced UI**: Graph visualizations and control presets

## Troubleshooting

### Common Issues

1. **WiFi Connection**: Ensure correct SSID and password
2. **Dashboard Access**: Check IP address 192.168.4.1
3. **Device Status**: Monitor serial output for connection issues
4. **Emergency Stop**: Verify sensors and communication paths

### Debug Commands

- Monitor serial output on all ESP32 modules
- Check WebSocket connection status in browser console
- Verify device heartbeats in telemetry data
- Test UART communication as fallback

---

**Implementation Status**: ✅ Complete
**Dashboard**: ✅ Functional at 192.168.4.1
**WiFi Networking**: ✅ Unified across all devices
**Real-Time Control**: ✅ WebSocket implementation
**Safety Systems**: ✅ Emergency stops and monitoring
