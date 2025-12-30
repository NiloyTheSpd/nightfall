# Dashboard User Guide

## Quick Start

1. **Access the Dashboard**

   - Connect to robot WiFi network: "ProjectNightfall"
   - Open browser and navigate to: http://192.168.4.1

2. **Wait for Connection**

   - Dashboard loads with skeleton screens
   - WebSocket automatically connects to robot
   - Green "Connected" indicator appears when ready

3. **Start Controlling**
   - Use keyboard shortcuts or on-screen buttons
   - Toggle between Manual and Autonomous modes
   - Monitor real-time telemetry data

## Interface Overview

### Header Section

- **Robot Status** - Current operational state (IDLE, AUTONOMOUS, EMERGENCY)
- **Connection Indicator** - WebSocket connection status
- **Latency Display** - Network response time
- **Settings Button** - Access configuration panel
- **Analytics Toggle** - Show/hide telemetry charts

### Main Layout (Three Zones)

#### Zone A: Live Vision (Left, 60%)

- **Camera Feed** - Real-time video from ESP32-CAM
- **FPS Counter** - Current video framerate
- **HUD Overlay** - Distance, gas level, camera IP, timestamp
- **Recording Indicator** - Red REC badge
- **Fullscreen Button** - Expand video to full screen

#### Zone B: System Health (Middle, 25%)

- **ESP32 Status**:
  - BRAIN (Back ESP32) - Main controller
  - MOTORS (Front ESP32) - Motor control
  - VISION (Camera ESP32) - Video streaming
- **Real-time Sensors**:
  - Battery voltage with bar graph
  - WiFi signal strength
  - Gas level detection
  - System uptime
- **Performance Metrics**:
  - Data rate (KB/s)
  - Command success rate
  - Message count
  - Network latency
- **Data Export** - Save telemetry data

#### Zone C: Control Interface (Right, 15%)

- **Mode Toggle** - Switch between Manual/Autonomous
- **Movement Controls** - Directional buttons
- **Emergency Stop** - Large red button
- **Motor Status** - Current motor values and indicators
- **Keyboard Shortcuts** - Reference guide

### Mission Planning (Bottom Section)

#### Mission Map

- **Interactive Canvas** - Click to add waypoints
- **Robot Position** - Red circle with direction indicator
- **Waypoint Path** - Blue dashed line showing route
- **Grid Background** - For spatial reference

#### Mission Controls

- **Start** - Begin autonomous navigation
- **Pause/Resume** - Pause mission execution
- **Stop** - End mission and return control
- **Clear** - Remove all waypoints

#### Waypoint Management

- **Waypoint List** - All defined waypoints with coordinates
- **Status Indicators**:
  - Green checkmark - Completed
  - Orange navigation - Current target
  - Blue circle - Upcoming
- **Quick Patterns** - Pre-defined routes (Square, Circle)
- **Remove Button** - Delete individual waypoints

#### System Alerts

- **Alert Feed** - Recent system notifications
- **Color Coding**:
  - Red - Errors
  - Yellow - Warnings
  - Blue - Information
- **Timestamps** - When each alert occurred

## Features Guide

### 📊 Analytics Dashboard

Click the chart icon in header to show/hide:

- **Battery Voltage Chart** - Historical trend with min/max/avg
- **Gas Level Chart** - Air quality monitoring
- **Distance Chart** - Obstacle detection history
- **WiFi Signal Chart** - Connection quality over time

Charts update in real-time and show last 50 data points.

### ⚙️ Settings Panel

Access via gear icon in header:

**Appearance**

- Theme selection (Dark/Light/Auto) - Coming soon

**Audio**

- Enable/disable sound notifications

**Connection**

- Auto-reconnect toggle
- Show notifications toggle
- Command throttle slider (10-200ms)

**Video**

- Quality selection (Low/Medium/High)

**Data Management**

- Export settings as JSON
- Import saved settings
- Reset to defaults

### 🎮 Control Modes

#### Manual Mode (Default)

- Direct control via keyboard or buttons
- Real-time motor response
- Immediate emergency stop
- Best for exploration and testing

#### Autonomous Mode

- Robot follows waypoint path
- Automatic navigation
- Pause/resume capability
- Mission progress tracking

### 🖼️ Fullscreen Video

Click maximize icon on video feed:

- Immersive camera view
- Save snapshot button
- Press ESC to exit
- HUD overlay with telemetry

### 💾 Data Export

Available formats:

- **JSON** - Complete telemetry data structure
- **CSV** - Sensor values for spreadsheet analysis
- **TXT** - Human-readable report

Export includes:

- Current telemetry snapshot
- Connection statistics
- Recent alerts
- Waypoint coordinates
- Timestamp and metadata

### 🔔 Alert System

Automatic notifications for:

- Connection status changes
- Mode switches
- Mission events
- System errors
- Emergency stops

Alerts auto-dismiss after 5 seconds (errors persist until acknowledged).

## Keyboard Controls

### Navigation

- **W** or **↑** - Move forward
- **S** or **↓** - Move backward
- **A** or **←** - Turn left
- **D** or **→** - Turn right
- **Ctrl + R** - Rotate clockwise

### Actions

- **Space** - Emergency stop
- **Escape** - Emergency stop / Close modals
- **Tab** - Navigate interface elements

### Interface

- All buttons accessible via keyboard
- Focus indicators show current selection
- Enter/Space to activate buttons

## Best Practices

### Connection Management

1. Wait for "Connected" status before operating
2. Monitor latency - keep below 100ms for best control
3. Enable auto-reconnect in settings
4. Check WiFi signal strength regularly

### Mission Planning

1. Test manual controls before autonomous mode
2. Plan waypoint routes away from obstacles
3. Start with simple paths (Square pattern)
4. Monitor mission progress in waypoint list
5. Keep Emergency Stop accessible

### Safety

1. **Always** know where Emergency Stop is
2. Switch to Manual mode if robot behaves unexpectedly
3. Monitor battery voltage (below 12V is critical)
4. Watch gas sensor for hazardous environments
5. Keep visual contact with robot when possible

### Performance

1. Close unused browser tabs
2. Use wired connection for laptop if possible
3. Monitor data rate - should be steady
4. Reduce telemetry history in settings if laggy
5. Disable analytics charts if not needed

## Troubleshooting

### Dashboard Won't Load

- ✓ Connected to "ProjectNightfall" WiFi?
- ✓ Navigate to exactly http://192.168.4.1
- ✓ Try different browser (Chrome recommended)
- ✓ Clear browser cache
- ✓ Check ESP32 is powered on

### WebSocket Won't Connect

- ✓ Check connection status indicator
- ✓ Auto-reconnect enabled in settings?
- ✓ ESP32 back controller powered?
- ✓ Check browser console for errors
- ✓ Try manual page refresh

### Video Not Showing

- ✓ Camera ESP32 status should be green
- ✓ Correct camera IP (default: 192.168.4.3)
- ✓ Check camera power connection
- ✓ Try fullscreen mode
- ✓ Refresh page

### Controls Not Working

- ✓ Not in Autonomous mode?
- ✓ Check connection status
- ✓ Try emergency stop first
- ✓ Verify motor status shows values changing
- ✓ Check command throttle setting

### Slow Performance

- ✓ Disable analytics dashboard
- ✓ Close other browser tabs
- ✓ Reduce telemetry history points (settings)
- ✓ Check WiFi signal strength
- ✓ Move closer to robot

### Mission Won't Start

- ✓ Waypoints defined (click map)?
- ✓ In Autonomous mode?
- ✓ Previous mission stopped?
- ✓ Robot connected?
- ✓ Try clearing and re-adding waypoints

## Advanced Features

### Custom Waypoint Patterns

Use the pattern buttons for:

- **Square** - 4-point rectangular patrol
- **Circle** - 6-point circular route
- Or click custom points on map

### Telemetry Analysis

1. Enable analytics dashboard
2. Monitor trends over time
3. Export data for external analysis
4. Identify patterns in sensor readings

### Settings Backup

1. Configure dashboard to your preference
2. Export settings via settings panel
3. Save JSON file
4. Import on new devices or after reset

## Tips & Tricks

💡 **Quick Emergency Stop** - Space bar or ESC key from anywhere

💡 **Fast Mode Switch** - Click mode button or use autonomous toggle

💡 **Snapshot Capture** - Fullscreen video mode has snapshot button

💡 **Clear Alerts** - Alerts auto-dismiss; errors stay until addressed

💡 **Keyboard Focus** - Tab through interface faster than using mouse

💡 **Mission Patterns** - Use quick patterns as starting points, customize after

💡 **Analytics Toggle** - Show charts when diagnosing, hide for cleaner view

💡 **Settings Persistence** - Your preferences save automatically

💡 **Tooltips** - Hover over any icon for helpful information

💡 **Responsive Design** - Works on tablets and phones too!

## FAQ

**Q: Can I control multiple robots?**
A: No, one dashboard per robot. Connect to the specific robot's WiFi.

**Q: Does it work offline?**
A: Requires connection to robot's WiFi. No internet needed.

**Q: How do I update the dashboard?**
A: Upload new files to ESP32's SPIFFS filesystem.

**Q: Can I use a game controller?**
A: Not currently supported. Keyboard/mouse only.

**Q: What browsers are supported?**
A: Chrome, Firefox, Safari, Edge - latest 2 versions.

**Q: Does it remember my settings?**
A: Yes, settings are saved in browser localStorage.

**Q: Can I run it on mobile?**
A: Yes! Fully responsive design works on phones/tablets.

**Q: Is the data encrypted?**
A: WebSocket uses WS (not WSS). Robot creates isolated network.

---

**Dashboard Version**: 2.0.0  
**Last Updated**: December 30, 2025  
**Support**: Check ACCESSIBILITY.md for assistive technology help
