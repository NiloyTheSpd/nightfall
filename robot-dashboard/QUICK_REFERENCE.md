# Quick Reference Guide

## Dashboard Access

```
WiFi Network: ProjectNightfall
Dashboard URL: http://192.168.4.1
WebSocket: ws://192.168.4.1:8888
Camera Feed: http://192.168.4.3:81/stream
```

## Keyboard Shortcuts

### Movement (Manual Mode Only)

| Key         | Action           |
| ----------- | ---------------- |
| W or ↑      | Move Forward     |
| S or ↓      | Move Backward    |
| A or ←      | Turn Left        |
| D or →      | Turn Right       |
| Ctrl + R    | Rotate Clockwise |
| Space / Esc | Emergency Stop   |

### Interface

| Key   | Action                 |
| ----- | ---------------------- |
| Tab   | Navigate elements      |
| Enter | Activate button        |
| Esc   | Close modal/fullscreen |

## Color Coding

### Battery

- 🟢 Green (>50%): Healthy
- 🟡 Yellow (25-50%): Warning
- 🔴 Red (<25%): Critical

### Distance

- 🟢 Green (>50cm): Safe
- 🟡 Yellow (20-50cm): Caution
- 🔴 Red (<20cm): Obstacle

### Gas Level

- 🟢 Green (<1500): Normal
- 🟡 Yellow (1500-3000): Elevated
- 🔴 Red (>3000): Dangerous

### System Status

- 🟢 Green: Online/Healthy
- 🟡 Yellow: Warning/Reconnecting
- 🔴 Red: Offline/Critical

## Quick Actions

### Start Mission

1. Switch to Autonomous Mode
2. Click map to add waypoints
3. Press Start button
4. Monitor progress in waypoint list

### Emergency Procedures

1. **Stop Robot**: Press Space or Emergency Stop button
2. **Switch to Manual**: Click mode toggle
3. **Review Alerts**: Check system alerts panel
4. **Export Data**: Use data export in System Health panel

### Export Data

1. Scroll to System Health panel
2. Find "Export Data" section
3. Select format (JSON/CSV/TXT)
4. Click Export button
5. File downloads automatically

### Customize Settings

1. Click gear icon in header
2. Adjust preferences
3. Export settings to save (optional)
4. Click "Save & Close"
5. Settings persist automatically

## Component Layout

```
┌─────────────────────────────────────────────────────────┐
│ HEADER: Status | Connection | Latency | Settings | Analytics │
├──────────────────┬──────────────┬─────────────────────┤
│                  │              │                     │
│ VIDEO FEED       │ SYSTEM       │ CONTROLS            │
│ (60%)            │ HEALTH       │ (15%)               │
│                  │ (25%)        │                     │
│ - Camera stream  │ - ESP32      │ - Mode toggle       │
│ - FPS counter    │   Status     │ - Movement pad      │
│ - HUD overlay    │ - Sensors    │ - Emergency stop    │
│ - Fullscreen     │ - Metrics    │ - Motor status      │
│                  │ - Export     │ - Shortcuts         │
│                  │              │                     │
├──────────────────┴──────────────┴─────────────────────┤
│ ANALYTICS (Optional Toggle)                           │
│ [Battery] [Gas] [Distance] [WiFi Signal]              │
├───────────────────────────────────────────────────────┤
│ MISSION PLANNING                                      │
│ ┌────────────────┐  ┌───────────────────────────────┐│
│ │ Mission Map    │  │ Waypoint Management           ││
│ │ (Interactive)  │  │ - List with status            ││
│ │                │  │ - Quick patterns              ││
│ │ [Canvas]       │  │ - Mission controls            ││
│ │                │  │                                ││
│ └────────────────┘  └───────────────────────────────┘│
│ ┌────────────────────────────────────────────────────┐│
│ │ System Alerts                                      ││
│ │ - Recent notifications                             ││
│ │ - Error messages                                   ││
│ └────────────────────────────────────────────────────┘│
└───────────────────────────────────────────────────────┘
```

## Troubleshooting Quick Fixes

### Dashboard won't load

→ Check WiFi connection to "ProjectNightfall"  
→ Try http://192.168.4.1 exactly  
→ Clear browser cache and reload

### WebSocket won't connect

→ Check green status indicator  
→ Wait for auto-reconnect (up to 30s)  
→ Refresh page manually

### Video not showing

→ Verify Camera ESP32 status is green  
→ Check camera IP matches 192.168.4.3  
→ Try fullscreen mode

### Controls not responding

→ Check not in Autonomous mode  
→ Verify connection status  
→ Press Emergency Stop first

### Slow performance

→ Disable analytics dashboard  
→ Close other browser tabs  
→ Move closer to robot

## Feature Overview

| Feature          | Location        | Quick Access       |
| ---------------- | --------------- | ------------------ |
| Settings         | Header          | Gear icon          |
| Analytics        | Header          | Chart icon         |
| Fullscreen Video | Video panel     | Maximize icon      |
| Data Export      | System Health   | Bottom of panel    |
| Emergency Stop   | Control panel   | Large red button   |
| Mode Switch      | Control panel   | Top button         |
| Mission Start    | Mission section | Green Start button |
| Waypoint Add     | Mission map     | Click canvas       |
| Alert History    | Mission section | Bottom right       |

## Status Indicators

### Connection States

- 🔵 **Connecting**: Initial connection attempt
- 🟢 **Connected**: Active WebSocket link
- 🟡 **Reconnecting**: Attempting to restore connection
- 🔴 **Disconnected**: No connection
- ⚠️ **Error**: Connection failed

### Mission States

- ⚫ **Idle**: No active mission
- 🟢 **Active**: Mission in progress
- 🟡 **Paused**: Mission paused
- 🔴 **Emergency**: Emergency stop activated

### Waypoint Status

- ✅ **Completed**: Already visited
- 🎯 **Current**: Currently navigating to
- ⭕ **Pending**: Not yet reached

## Performance Indicators

### Good Performance ✅

- Latency: < 100ms
- FPS: 15-30
- Data Rate: Steady
- Signal: > 50%
- Battery: > 12V

### Warning ⚠️

- Latency: 100-200ms
- FPS: 10-15
- Signal: 25-50%
- Battery: 11-12V

### Critical 🔴

- Latency: > 200ms
- FPS: < 10
- Signal: < 25%
- Battery: < 11V

## Mission Planning Tips

### Quick Patterns

- **Square**: 4-point patrol route
- **Circle**: 6-point circular path
- Custom: Click map for specific points

### Best Practices

1. Start with simple paths
2. Keep waypoints visible on map
3. Test in manual mode first
4. Monitor battery during mission
5. Use pause if needed
6. Emergency stop always accessible

## Data Export Use Cases

### JSON

- Full data structure
- Import into other tools
- Backup telemetry
- Programming analysis

### CSV

- Open in Excel/Google Sheets
- Create charts/graphs
- Statistical analysis
- Report generation

### TXT

- Human-readable report
- Quick review
- Share via text
- Print documentation

## Accessibility Features

- ♿ Full keyboard navigation
- 🔊 Screen reader support
- 🎯 Large touch targets (44px)
- 🔍 Zoom up to 200%
- 🎨 High contrast compatible
- 💡 Tooltips for context
- ⌨️ Keyboard shortcuts
- 📱 Mobile responsive

## Browser Recommendations

1. **Chrome** (Recommended) - Best performance
2. **Firefox** - Full compatibility
3. **Safari** - Works on iOS
4. **Edge** - Windows optimized

---

**For detailed information**, see:

- `USER_GUIDE.md` - Complete manual
- `ACCESSIBILITY.md` - Accessibility guide
- `TESTING.md` - Testing protocols
- `README.md` - Full documentation
