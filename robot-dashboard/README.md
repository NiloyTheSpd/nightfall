# Project Nightfall - Enhanced Mission Control Dashboard

A production-ready, enterprise-grade Mission Control dashboard for the Project Nightfall rescue robot system. This enhanced dashboard provides real-time WebSocket communication, comprehensive system monitoring, and intuitive control interfaces for high-stress emergency scenarios.

## 🚀 Features

### Real-Time WebSocket Communication

- **Direct robot connection** to `ws://192.168.4.1:8888`
- **Automatic reconnection** with exponential backoff (2s, 4s, 8s, 16s)
- **Connection state management** (connecting, connected, disconnected, error)
- **Latency monitoring** and performance metrics
- **Command throttling** (50ms minimum between commands)

### Three-Zone Layout

#### Zone A: Video & Vision Processing (60% width)

- **Live video stream** from ESP32-CAM (`http://192.168.4.3:81/stream`)
- **HUD overlay** with real-time telemetry:
  - Distance readings (color-coded: green >50cm, yellow 20-50cm, red <20cm)
  - Gas levels (0-4095 scale)
  - Camera IP and timestamp
  - Video resolution/bitrate
- **FPS counter** and performance monitoring
- **Fallback UI** when camera is offline

#### Zone B: System Health Dashboard (25% width)

- **Connection status indicators** (32px circles):
  - "BRAIN" (Back ESP32): Green=connected, Red=disconnected, Yellow=reconnecting
  - "MOTORS" (Front ESP32): Green=UART healthy, Red=UART failed
  - "VISION" (Camera ESP32): Green=streaming, Red=no feed
- **Real-time sensor metrics** with vertical progress bars:
  - Battery Level: 0-100% (red <25%, yellow 25-50%, green >50%)
  - Signal Strength: WiFi quality indicator
  - Gas Concentration: 0-4095 scale with color coding
- **Performance monitoring**: Data rate, success rate, message counts, latency

#### Zone C: Command & Control Interface (40% width)

- **Keyboard controls** with full WASD support:
  - W/↑: Forward (L=200, R=200)
  - S/↓: Reverse (L=-150, R=-150)
  - A/←: Counter-clockwise (L=-100, R=100)
  - D/→: Clockwise (L=100, R=-100)
  - Space/Esc: Emergency stop (L=0, R=0)
  - Ctrl+R: Rotate clockwise
- **Touch controls** (80x80px buttons) with visual feedback
- **Emergency stop button** (120px diameter, red, with pulse animation)
- **Mode toggle** between Manual and Autonomous operation

### Mission Planning & Navigation

- **Interactive mission map** with click-to-add waypoints
- **Waypoint management** with visual status indicators
- **Mission patterns**: Square and circular predefined patterns
- **Autonomous navigation simulation** with real-time robot position
- **Mission controls**: Start, Pause, Resume, Stop, Clear

### Performance Optimization

- **Direct WebSocket transmission** bypassing React state for commands
- **Command throttling** to prevent flooding
- **FPS counter** for video stream performance
- **Network latency measurement** with ping/pong
- **Memory usage tracking** and performance metrics

### Error Handling & Resilience

- **Graceful degradation** for subsystem failures
- **Visual error indicators** with specific failure types
- **Automatic retry mechanisms** with exponential backoff
- **Data validation** for incoming telemetry
- **Connection timeout handling** with user notifications

### Accessibility & Responsive Design

- **Full keyboard navigation** with focus indicators
- **ARIA labels** for screen readers contrast mode\*\* support
- \*\*High
- **Responsive layout**:
  - Desktop: Full keyboard support with three-zone layout
  - Mobile/Tablet: Touch controls with gesture recognition
  - <768px width: Vertical stacking of zones
- **High-DPI display** support

## 🏗️ Architecture

### Component Structure

```
src/
├── DashboardEnhanced.jsx     # Main dashboard component
├── main.jsx                  # React entry point
├── index.css                 # Tailwind CSS with custom components
└── components/
    ├── ConnectionStatus.jsx  # WebSocket connection management
    └── SystemHealthPanel.jsx # System health monitoring
```

### WebSocket Communication Protocol

#### Incoming Telemetry (from robot)

```json
{
  "type": "telemetry",
  "payload": {
    "back_status": "ok",
    "front_status": true,
    "camera_status": true,
    "dist": 15.5,
    "gas": 100,
    "cam_ip": "192.168.4.3",
    "battery": 14.2,
    "signal_strength": 85,
    "uptime": 3600
  }
}
```

#### Outgoing Commands (to robot)

```json
{
  "L": 200,
  "R": -100
}
```

#### Ping/Pong for Latency

```json
// Outgoing ping
{"type": "ping", "timestamp": 1640995200000}

// Incoming pong
{"type": "pong", "timestamp": 1640995200050}
```

## 🚀 Getting Started

### Prerequisites

- Node.js 16+
- npm or yarn
- Robot system running WebSocket server on `192.168.4.1:8888`

### Installation

1. **Install dependencies:**

   ```bash
   cd robot-dashboard
   npm install
   ```

2. **Start development server:**

   ```bash
   npm run dev
   ```

3. **Build for production:**

   ```bash
   npm run build
   ```

4. **Preview production build:**
   ```bash
   npm run preview
   ```

### Development

The dashboard will be available at `http://localhost:3000` during development.

## 🔧 Configuration

### WebSocket Connection

Edit `DashboardEnhanced.jsx` to modify the WebSocket URL:

```javascript
const WEBSOCKET_URL = "ws://192.168.4.1:8888"; // Change this IP/port as needed
```

### Tailwind CSS Customization

Modify `tailwind.config.js` to customize:

- Nightfall color scheme
- Robot system colors
- Custom animations and fonts
- Responsive breakpoints

### Command Throttling

Adjust command transmission timing in `DashboardEnhanced.jsx`:

```javascript
const COMMAND_THROTTLE = 50; // Minimum ms between commands
```

### Reconnection Strategy

Modify exponential backoff delays:

```javascript
const RECONNECT_DELAYS = [2000, 4000, 8000, 16000]; // ms
```

## 📱 Usage

### Manual Control Mode

1. **Keyboard controls**: Use WASD or arrow keys for movement
2. **Mouse/touch**: Click control buttons for precise movement
3. **Emergency stop**: Press Space/Esc or click red emergency button

### Autonomous Mode

1. **Add waypoints**: Click on the mission map to create navigation points
2. **Start mission**: Click "Start" to begin autonomous navigation
3. **Monitor progress**: Watch real-time robot position updates
4. **Control mission**: Pause, resume, or stop as needed

### System Monitoring

- **Connection status**: Monitor WebSocket connection health
- **Subsystem status**: Track Brain, Motors, and Vision ESP32s
- **Sensor data**: Real-time battery, distance, gas, and signal readings
- **Performance metrics**: Data rate, latency, and success rates

## 🛡️ Safety Features

### Emergency Procedures

- **Emergency stop**: Immediately halts all motor movement
- **Connection loss**: Automatic reconnection with user notifications
- **System alerts**: Visual warnings for critical conditions
- **Graceful degradation**: Continues operating with failed subsystems

### Error Handling

- **WebSocket errors**: Automatic reconnection with exponential backoff
- **Invalid data**: Data validation with error logging
- **Network timeouts**: Connection monitoring with timeout handling
- **UI fallbacks**: Alternative interfaces when components fail

## 🔍 Troubleshooting

### Common Issues

**Connection Failed**

- Ensure robot WebSocket server is running on `192.168.4.1:8888`
- Check network connectivity and firewall settings
- Verify robot is powered on and accessible

**Camera Stream Offline**

- Confirm ESP32-CAM is connected and powered
- Check camera IP address in telemetry data
- Verify camera web server is running on port 81

**Commands Not Working**

- Ensure WebSocket is connected (green status)
- Check robot is in Manual mode
- Verify motor control system is operational

**Performance Issues**

- Monitor data rate in System Health panel
- Check network latency measurements
- Consider reducing update frequencies if needed

### Debug Mode

Enable console logging by modifying the dashboard:

```javascript
// Add to DashboardEnhanced.jsx
const DEBUG = true;

// Log WebSocket messages
if (DEBUG) {
  console.log("WebSocket message:", event.data);
}
```

## 📊 Performance Metrics

The dashboard tracks and displays:

- **Connection latency** (ping/pong response times)
- **Message throughput** (messages per second)
- **Data transfer rate** (KB/s)
- **Command success rate** (percentage of successful transmissions)
- **Video stream FPS** (frames per second)
- **Memory usage** (browser memory consumption)

## 🎯 Future Enhancements

### Planned Features

- **Historical data charts** for sensor readings over time
- **Multi-robot support** for coordinated missions
- **Advanced mission planning** with obstacle avoidance
- **Voice control integration** for hands-free operation
- **Augmented reality** overlay for enhanced situational awareness
- **Export capabilities** for mission data and telemetry logs

### Integration Opportunities

- **Machine learning** integration for predictive maintenance
- **Cloud synchronization** for multi-device access
- **Real-time collaboration** for team-based operations
- **Advanced analytics** for mission performance analysis

## 📄 License

This project is part of the Project Nightfall rescue robot system. All rights reserved.

## 🤝 Contributing

For development guidelines and contribution requirements, see the main Project Nightfall repository documentation.

---

**Built with ❤️ for emergency response and rescue operations**
