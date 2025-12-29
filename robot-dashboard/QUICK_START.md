# Quick Start Guide - Project Nightfall Mission Control Dashboard

Get the enhanced Mission Control dashboard up and running in under 5 minutes!

## ⚡ Fast Setup (2 minutes)

### 1. Install Dependencies

```bash
cd robot-dashboard
npm install
```

### 2. Start Development Server

```bash
npm run dev
```

### 3. Open Dashboard

Navigate to `http://localhost:3000` in your browser.

**That's it! The dashboard is now running.**

## 🤖 Robot Connection Setup

### Prerequisites

- Project Nightfall robot system powered on
- Robot WiFi network "ProjectNightfall" accessible
- Robot WebSocket server running on `192.168.4.1:8888`

### Network Connection

1. **Connect to robot WiFi**: Join "ProjectNightfall" network
2. **Verify connection**: Robot should appear as `192.168.4.1`
3. **Test WebSocket**: Dashboard should auto-connect when accessible

### Connection Status Indicators

- 🟢 **Green**: Fully connected and operational
- 🟡 **Yellow**: Connecting or reconnecting
- 🔴 **Red**: Connection failed or error state

## 🎮 Basic Operations (3 minutes)

### Manual Control Mode

1. **Movement**: Use WASD keys or click control buttons

   - W/↑: Move forward
   - S/↓: Move backward
   - A/←: Turn left
   - D/→: Turn right
   - Space: Emergency stop

2. **Test movement**:
   ```bash
   # Try these commands:
   - Press W to move forward
   - Press A to turn left
   - Press Space for emergency stop
   ```

### Camera Feed

1. **View live feed**: Camera zone shows robot's perspective
2. **HUD overlay**: Real-time distance, gas, and system data
3. **FPS counter**: Video performance monitoring

### System Monitoring

1. **Health panel**: Check Brain, Motors, Vision status
2. **Sensor data**: Monitor battery, distance, gas levels
3. **Performance**: View connection stats and data rates

## 🗺️ Mission Planning (Quick Demo)

### Create Waypoint Mission

1. **Click mission map**: Add navigation points
2. **Switch to autonomous mode**: Toggle button in control panel
3. **Start mission**: Click "Start" to begin navigation
4. **Monitor progress**: Watch robot move between waypoints

### Quick Mission Patterns

1. **Square pattern**: Click "Square Pattern" for rectangular route
2. **Circle pattern**: Click "Circle Pattern" for circular route
3. **Clear waypoints**: Remove all points with "Clear" button

## 🚨 Emergency Procedures

### Emergency Stop

- **Keyboard**: Press Space or Escape
- **Mouse**: Click red emergency button (large circle)
- **Effect**: Immediately stops all motor movement

### Connection Loss

- **Auto-reconnect**: Dashboard attempts reconnection automatically
- **Status updates**: Visual indicators show reconnection progress
- **Manual retry**: Refresh page if auto-reconnect fails

## 📱 Device Compatibility

### Desktop (Recommended)

- **Full features**: All controls and monitoring
- **Keyboard support**: WASD movement controls
- **Multi-zone layout**: Optimal three-zone display

### Tablet

- **Touch controls**: Tap-based movement
- **Responsive layout**: Zones stack vertically
- **Gesture support**: Pinch to zoom mission map

### Mobile

- **Basic controls**: Essential movement functions
- **Stacked layout**: Vertical zone arrangement
- **Touch optimization**: Large control buttons

## 🔧 Troubleshooting

### Connection Issues

```bash
# Check robot network
ping 192.168.4.1

# Verify WebSocket server
telnet 192.168.4.1 8888
```

### Dashboard Not Loading

```bash
# Clear browser cache
Ctrl+Shift+R (Hard refresh)

# Check console errors
F12 → Console tab
```

### Performance Issues

- **High latency**: Check WiFi signal strength
- **Low FPS**: Reduce browser zoom level
- **Memory usage**: Close other browser tabs

## 📊 Testing Checklist

### ✅ Basic Functionality

- [ ] Dashboard loads without errors
- [ ] WebSocket connection established (green status)
- [ ] Camera feed displays (or shows offline message)
- [ ] Movement controls respond (WASD/buttons)
- [ ] Emergency stop works immediately
- [ ] System health panel shows data

### ✅ Advanced Features

- [ ] Mode switching (Manual ↔ Autonomous)
- [ ] Waypoint creation and management
- [ ] Mission start/pause/stop functions
- [ ] Performance metrics update
- [ ] Alerts system working
- [ ] Responsive design on different screen sizes

### ✅ Emergency Procedures

- [ ] Emergency stop activates instantly
- [ ] Connection loss triggers reconnection
- [ ] System alerts display properly
- [ ] Graceful handling of robot disconnections

## 🎯 Next Steps

### For Operators

1. **Practice controls**: Get comfortable with movement commands
2. **Test emergency procedures**: Ensure quick response capability
3. **Explore mission planning**: Create and execute waypoint routes
4. **Monitor system health**: Regularly check sensor readings

### For Developers

1. **Review code structure**: Examine `DashboardEnhanced.jsx`
2. **Customize appearance**: Modify colors in `tailwind.config.js`
3. **Add features**: Extend functionality as needed
4. **Performance tuning**: Optimize for specific hardware

## 📞 Support

### Getting Help

- **Documentation**: See `README.md` for detailed information
- **Code comments**: Review inline documentation in components
- **Console logs**: Check browser developer tools for errors

### Common Solutions

- **Refresh page**: Most issues resolve with a page refresh
- **Check network**: Verify robot WiFi connection
- **Clear cache**: Hard refresh (Ctrl+Shift+R)
- **Restart server**: Stop and restart development server

---

**🚀 Ready to control your rescue robot!**

For advanced usage and customization, see the full documentation in `README.md`.
