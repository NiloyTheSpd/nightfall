# Project Nightfall - Enhanced Mission Control Dashboard v2.0

A production-ready, enterprise-grade Mission Control dashboard for the Project Nightfall rescue robot system. This enhanced dashboard provides real-time WebSocket communication, comprehensive system monitoring, intuitive control interfaces, and advanced analytics for high-stress emergency scenarios.

## ✨ What's New in v2.0

- 📊 **Real-time Telemetry Charts** - Historical data visualization for battery, gas, distance, and WiFi
- ⚙️ **Settings Panel** - Customizable preferences with import/export
- 🖼️ **Fullscreen Video Mode** - Immersive camera view with snapshot capture
- 💾 **Data Export** - Save mission data in JSON/CSV/TXT formats
- 🎨 **Modern UI Enhancements** - Smooth animations, tooltips, improved accessibility
- 🛡️ **Error Boundary** - Graceful error recovery without page reload
- ⏳ **Loading States** - Professional skeleton screens
- 📱 **Enhanced Responsive Design** - Better mobile and tablet support
- ♿ **WCAG 2.1 AA Compliant** - Full accessibility support
- 🔔 **Improved Alert System** - Better notifications and feedback

## 🚀 Features

### Real-Time WebSocket Communication

- **Direct robot connection** to `ws://192.168.4.1:8888`
- **Automatic reconnection** with exponential backoff (2s, 4s, 8s, 16s)
- **Connection state management** (connecting, connected, disconnected, error)
- **Latency monitoring** and performance metrics
- **Command throttling** (configurable, default 50ms)
- **Message statistics** tracking

### Analytics Dashboard (NEW)

- **Real-time Charts** with canvas rendering:
  - Battery voltage trends with min/max/avg statistics
  - Gas level monitoring with threshold alerts
  - Distance sensor history
  - WiFi signal strength tracking
- **50-point history** (configurable in settings)
- **Toggle visibility** to reduce visual clutter
- **Export chart data** with telemetry export

### Three-Zone Layout

#### Zone A: Video & Vision Processing (60% width)

- **Live video stream** from ESP32-CAM (`http://192.168.4.3:81/stream`)
- **Fullscreen mode** with snapshot capture
- **HUD overlay** with real-time telemetry:
  - Distance readings (color-coded: green >50cm, yellow 20-50cm, red <20cm)
  - Gas levels (0-4095 scale with threshold warnings)
  - Camera IP and timestamp
  - Recording indicator
- **FPS counter** and performance monitoring
- **Fallback UI** when camera is offline

#### Zone B: System Health Dashboard (25% width)

- **Connection status indicators** (32px circles with pulse animation):
  - "BRAIN" (Back ESP32): Green=connected, Red=disconnected, Yellow=reconnecting
  - "MOTORS" (Front ESP32): Green=UART healthy, Red=UART failed
  - "VISION" (Camera ESP32): Green=streaming, Red=no feed
- **Real-time sensor metrics** with animated progress bars:
  - Battery Level: 0-100% (red <25%, yellow 25-50%, green >50%)
  - WiFi Signal Strength: Quality indicator
  - Gas Concentration: 0-4095 scale with color coding
  - System Uptime: Hours and minutes
- **Performance monitoring**: Data rate, success rate, message counts, latency
- **Data Export Panel**: Export telemetry in multiple formats

#### Zone C: Command & Control Interface (40% width)

- **Mode Toggle** with visual feedback:
  - Manual Mode (default): Direct control
  - Autonomous Mode: Waypoint navigation
- **Keyboard controls** with full WASD support:
  - W/↑: Forward (L=200, R=200)
  - S/↓: Reverse (L=-150, R=-150)
  - A/←: Turn left (L=-100, R=100)
  - D/→: Turn right (L=100, R=-100)
  - Space/Esc: Emergency stop (L=0, R=0)
  - Ctrl+R: Rotate clockwise
- **Touch controls** (80x80px buttons) with hover states and tooltips
- **Emergency stop button** (120px diameter, red, with pulse animation)
- **Motor status display** with real-time value bars
- **Keyboard shortcuts reference** built-in

### Mission Planning & Navigation

- **Interactive mission map** (800x360px canvas) with:
  - Click-to-add waypoints
  - Visual robot position and heading
  - Path preview with dashed lines
  - Grid background for reference
- **Waypoint management** with:
  - Visual status indicators (completed, current, pending)
  - Coordinate display
  - Individual removal
  - Batch clear
- **Mission patterns**: Square and circular predefined patterns
- **Autonomous navigation** with:
  - Real-time position tracking
  - Progress indicators
  - Pause/Resume capability
  - Mission completion alerts
- **Mission controls**: Start, Pause, Resume, Stop, Clear

### Settings & Customization (NEW)

- **Appearance**: Theme selection (Dark/Light/Auto - coming soon)
- **Audio**: Enable/disable sound notifications
- **Connection**: Auto-reconnect, notifications, command throttle slider
- **Video**: Quality selection (Low/Medium/High)
- **Data Management**: Export/Import settings, Reset to defaults
- **Persistence**: Settings saved to localStorage

### Data Export (NEW)

- **JSON Format**: Complete structured data with metadata
- **CSV Format**: Sensor values for spreadsheet analysis
- **TXT Format**: Human-readable mission report
- **Includes**: Telemetry, waypoints, alerts, statistics, timestamps

### Error Handling & Resilience

- **Error Boundary** component catches React errors
- **Graceful degradation** for subsystem failures
- **Visual error indicators** with specific failure types
- **Automatic retry mechanisms** with exponential backoff
- **Data validation** for incoming telemetry
- **Connection timeout handling** with user notifications
- **User-friendly error messages** with reload option

### Accessibility & Responsive Design

- **WCAG 2.1 Level AA Compliant**:
  - Full keyboard navigation with visible focus indicators
  - ARIA labels for all interactive elements
  - Semantic HTML structure
  - Screen reader support (tested with NVDA, JAWS, VoiceOver)
  - High contrast mode compatible
  - Color-independent information
- **Responsive layout**:
  - Desktop (1920x1080): Full three-zone layout
  - Laptop (1366x768): Optimized spacing
  - Tablet (768x1024): Two-column layout
  - Mobile (375x667): Single-column stack
- **Touch optimization**: 44x44px minimum tap targets
- **Zoom support**: Tested up to 200%
- **Reduced motion**: Respects prefers-reduced-motion

## 📁 Project Structure

```
robot-dashboard/
├── src/
│   ├── DashboardEnhanced.jsx     # Main dashboard (enhanced)
│   ├── main.jsx                  # React entry with ErrorBoundary
│   ├── index.css                 # Enhanced Tailwind CSS
│   └── components/
│       ├── ErrorBoundary.jsx     # Error recovery component
│       ├── LoadingSkeleton.jsx   # Loading state components
│       ├── TelemetryChart.jsx    # Real-time chart component
│       ├── SettingsPanel.jsx     # Settings management
│       ├── FullscreenVideo.jsx   # Fullscreen video viewer
│       ├── DataExportPanel.jsx   # Data export utilities
│       ├── Tooltip.jsx           # Tooltip component
│       ├── ConnectionStatus.jsx  # Connection management
│       └── SystemHealthPanel.jsx # System health monitoring
├── index.html                    # Enhanced HTML with metadata
├── package.json                  # Updated to v2.0.0
├── tailwind.config.js            # Extended theme config
├── vite.config.js                # Vite configuration
├── README.md                     # This file
├── ENHANCEMENTS.md               # Detailed enhancement list
├── ACCESSIBILITY.md              # Accessibility documentation
├── USER_GUIDE.md                 # Comprehensive user guide
├── TESTING.md                    # Testing checklist
└── QUICK_START.md                # Quick setup guide
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
