# Project Nightfall - Six-Motor Manual Rescue Robot Implementation Guide

## Overview

This guide provides complete implementation instructions for six-motor manual rescue robot system. The autonomous navigation features have been completely removed, and system now operates entirely under manual control via a web dashboard.

## System Architecture

### Six-Motor Configuration
- **Motor 1**: Front Left (via Front ESP32)
- **Motor 2**: Front Right (via Front ESP32)  
- **Motor 3**: Center Left (via Front ESP32)
- **Motor 4**: Center Right (via Front ESP32)
- **Motor 5**: Rear Left (direct Rear ESP32 control)
- **Motor 6**: Rear Right (direct Rear ESP32 control)

### ESP32 Controllers
1. **Rear ESP32 (Master)**: Sensor monitoring, web dashboard, motor 5-6 control
2. **Front ESP32 (Slave)**: Motor 1-4 control, heartbeat communication
3. **ESP32-CAM (Optional)**: Vision streaming (if needed)

## Pin Assignments

### Rear ESP32 (Master Controller)
```
Motor 5 (Rear Left):
  - PWM: GPIO 18 (MOTOR3_ENA)
  - Forward: GPIO 19 (MOTOR3_IN1)
  - Reverse: GPIO 21 (MOTOR3_IN2)

Motor 6 (Rear Right):
  - PWM: GPIO 32 (MOTOR4_ENA)
  - Forward: GPIO 33 (MOTOR4_IN1)
  - Reverse: GPIO 35 (MOTOR4_IN2)

Sensors:
  - Front Ultrasonic: Trig GPIO 14, Echo GPIO 18
  - Rear Ultrasonic: Trig GPIO 19, Echo GPIO 21
  - Gas Sensor: Analog GPIO 32, Digital GPIO 33
  - Smoke Sensor: Analog GPIO 12, Digital GPIO 13
  - Buzzer: GPIO 4

Communication:
  - UART TX: GPIO 16
  - UART RX: GPIO 17
```

### Front ESP32 (Slave Controller)
```
Motor 1-2 (Front Pair):
  - Motor 1 Left: PWM GPIO 13, IN1 GPIO 23, IN2 GPIO 22
  - Motor 2 Right: PWM GPIO 25, IN1 GPIO 26, IN2 GPIO 27

Motor 3-4 (Center Pair):
  - Motor 3 Left: PWM GPIO 14, IN1 GPIO 32, IN2 GPIO 33
  - Motor 4 Right: PWM GPIO 15, IN1 GPIO 19, IN2 GPIO 21

Communication:
  - UART TX: GPIO 16
  - UART RX: GPIO 17
```

## Build and Upload Instructions

### Prerequisites
- PlatformIO IDE or CLI
- Three ESP32 development boards
- Six motor drivers
- Six DC motors
- Sensors (2x ultrasonic, gas, smoke)
- Buzzer
- 14.8V battery pack
- 5V buck converter for ESP32s

### Build Commands

```bash
# Build and upload to Rear ESP32 (Master)
pio run -e rear_esp32 -t upload
pio device monitor -e rear_esp32

# Build and upload to Front ESP32 (Slave)
pio run -e front_esp32 -t upload
pio device monitor -e front_esp32

# Build and upload to ESP32-CAM (Optional)
pio run -e camera_esp32 -t upload
# Remove GPIO0->GND connection and press reset after upload
```

### Upload Sequence
1. Upload to Rear ESP32 first
2. Upload to Front ESP32 second
3. Upload to ESP32-CAM last (if using camera)

## Web Dashboard Usage

### Access
- Connect to WiFi: "ProjectNightfall"
- Password: "rescue2025"
- Open browser to: http://192.168.4.1

### Controls
- **Manual Control**: Forward, Left, Right, Backward, Stop
- **Emergency Stop**: Immediate halt of all motors
- **Individual Motor Test**: Test specific motor groups
- **System Status**: Real-time telemetry and sensor data

### Real-time Data
- Front/Rear Distance (cm)
- Gas Level (analog value)
- Smoke Level (analog value)
- Battery Voltage
- Front ESP32 Connection Status
- Emergency Stop Status

## Safety Features

### Emergency Stop Triggers
1. **Obstacle Detection**: Front distance < 20cm
2. **Gas Leak**: Gas level > 400 (analog)
3. **Smoke Detection**: Smoke level > 300 (analog)
4. **Rear Obstacle**: Rear distance < 15cm
5. **Communication Loss**: No UART data for >1000ms
6. **Manual Emergency**: Dashboard emergency button

### Buzzer and Alarm
- **Emergency Buzzer**: Activates for 5 seconds during emergency
- **Smoke Alarm**: Continuous alarm when smoke detected
- **Audio Feedback**: Different patterns for different alerts

## Testing Procedures

### Initial System Test
1. **Hardware Check**: Verify all connections
2. **Individual Motor Test**: Test each motor separately
3. **Pair Motor Test**: Test front pair, center pair, rear pair
4. **Full System Test**: Test all six motors simultaneously
5. **Sensor Validation**: Verify all sensors provide valid readings
6. **Emergency Stop Test**: Test all emergency stop triggers

### Motor Testing Commands
```bash
# Via Serial Monitor (Front ESP32)
test                    # Test all motors
motor 1                 # Test individual motor
status                  # Show system status
emergency              # Trigger emergency stop
reset                  # Reset emergency stop

# Via Web Dashboard
"Test Front Motors"     # Test motors 1-4
"Test Rear Motors"      # Test motors 5-6
"Test All Motors"       # Test all six motors
```

## Code Organization

### Modified Files
- `include/config.h`: Updated pin definitions and removed autonomous enums
- `src/main_rear.cpp`: Enhanced with smoke sensor and removed autonomous features
- `src/main_rear_enhanced.cpp`: New six-motor master controller
- `src/main_front_enhanced.cpp`: New four-motor slave controller
- `platformio.ini`: Updated for rear ESP32 motor control

### Removed Files
- `lib/Navigation/AutonomousNav.cpp` (renamed to .removed)
- `lib/Navigation/AutonomousNav.h` (renamed to .removed)

### New Features
- **Enhanced Sensor Validation**: Timeout detection for invalid readings
- **Six-Motor Control**: Simultaneous control of all motors
- **Improved Web Dashboard**: Real-time status and individual motor testing
- **Enhanced Safety System**: Smoke detection with continuous alarm
- **Motor Diagnostics**: Individual motor testing and status reporting

## Hardware Requirements

### Motor Drivers
- 3x Dual H-Bridge motor drivers (L298N or similar)
- Each driver controls 2 motors
- Support for PWM speed control

### Motors
- 6x DC motors (12V or 24V depending on application)
- Current rating: 2-5A per motor
- Gearbox recommended for torque

### Sensors
- 2x HC-SR04 ultrasonic sensors
- 1x MQ-2 gas sensor
- 1x MQ-7 smoke sensor (or similar)
- Appropriate voltage dividers for analog sensors

### Power System
- 14.8V battery pack for motors
- 5V buck converter for ESP32s
- Proper grounding between systems

## Troubleshooting

### Common Issues
1. **Front ESP32 Offline**: Check UART connections and power
2. **Motor Not Responding**: Verify motor driver connections and PWM signals
3. **Sensor Reading Invalid**: Check power and signal connections
4. **Emergency Stop Not Working**: Verify sensor thresholds and connections
5. **Web Dashboard Not Accessible**: Check WiFi AP settings and IP address

### Diagnostic Commands
```bash
# Via Serial Monitor
status                  # Full system status
emergency              # Test emergency stop
test                   # Motor movement test
help                   # Available commands
```

## Configuration Parameters

### Safety Thresholds (config.h)
```cpp
#define EMERGENCY_STOP_DISTANCE 20.0    // cm
#define GAS_THRESHOLD_ANALOG 400        // ADC value
#define SMOKE_THRESHOLD_ANALOG 300      // ADC value
```

### Motor Settings
```cpp
#define MAX_MOTOR_SPEED 180             // PWM value (0-255)
#define MOTOR_SPEED_RAMP 5              // Increment per cycle
```

### Communication
```cpp
#define UART_BAUDRATE 115200            // baud
#define HEARTBEAT_INTERVAL 1000         // ms
#define TELEMETRY_INTERVAL 500          // ms
```

## Version History

### Version 3.0.0 (Current)
- Removed all autonomous navigation features
- Implemented six-motor manual control system
- Enhanced sensor validation and safety features
- Improved web dashboard with individual motor testing
- Added smoke detection with continuous alarm
- Complete code organization and documentation

### Previous Versions
- Version 2.0.0: Basic autonomous navigation system
- Version 1.0.0: Initial implementation

## Support and Maintenance

### Regular Checks
1. **Motor Performance**: Monitor for unusual sounds or reduced performance
2. **Sensor Calibration**: Verify sensor readings against known references
3. **Battery Health**: Monitor voltage levels and charging cycles
4. **Connection Integrity**: Check all wiring for corrosion or loose connections

### Software Updates
- Backup current configuration before updates
- Test new firmware with individual motor tests first
- Verify all safety features work after updates
- Update documentation with any changes

This implementation guide provides complete instructions for building, deploying, and maintaining six-motor manual rescue robot system. All autonomous features have been removed, and system now operates entirely under manual control with enhanced safety features.