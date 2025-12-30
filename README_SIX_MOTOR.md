# Project Nightfall - Six-Motor Manual Rescue Robot

## Overview

Project Nightfall is a six-motor manual rescue robot platform. The autonomous navigation system has been completely removed, and the robot now operates entirely under manual control via a web dashboard.

## Key Features

### Manual Control

- Six-motor simultaneous control (4 front + 2 rear)
- Web-based dashboard at http://192.168.4.1
- Real-time motor status and telemetry
- Individual motor testing and diagnostics

### Enhanced Safety System

- Multiple emergency stop triggers:
  - Front obstacle detection (< 20cm)
  - Rear obstacle detection (< 15cm)
  - Gas leak detection (> 400 analog)
  - Smoke detection (> 300 analog)
  - Communication loss (> 1000ms)
  - Manual emergency button
- Buzzer with different alert patterns
- Continuous smoke alarm

### Sensor Monitoring

- 2x HC-SR04 ultrasonic sensors with validation
- MQ-2 gas sensor with digital threshold
- MQ-7 smoke sensor with continuous alarm
- Enhanced sensor accuracy with timeout detection

## System Architecture

### Controllers

1. **Rear ESP32 (Master)**: Web dashboard, sensors, motor 5-6 control
2. **Front ESP32 (Slave)**: Motor 1-4 control, heartbeat communication
3. **ESP32-CAM (Optional)**: Vision streaming

### Motor Configuration

```
Motor 1 (Front Left)   -> Front ESP32 -> Motor Driver 1
Motor 2 (Front Right)  -> Front ESP32 -> Motor Driver 1
Motor 3 (Center Left)  -> Front ESP32 -> Motor Driver 2
Motor 4 (Center Right) -> Front ESP32 -> Motor Driver 2
Motor 5 (Rear Left)    -> Rear ESP32  -> Motor Driver 3
Motor 6 (Rear Right)   -> Rear ESP32  -> Motor Driver 3
```

## Quick Start

### 1. Hardware Setup

- Connect all components according to pin assignments
- Power motors with 14.8V battery pack
- Power ESP32s with 5V buck converter
- Ensure proper grounding

### 2. Build and Upload

```bash
# Upload to Rear ESP32
pio run -e rear_esp32 -t upload

# Upload to Front ESP32
pio run -e front_esp32 -t upload
```

### 3. Access Dashboard

- Connect to WiFi: "ProjectNightfall"
- Password: "rescue2025"
- Open browser: http://192.168.4.1

### 4. Test Motors

- Use dashboard controls to test individual motors
- Verify all six motors respond correctly
- Test emergency stop functionality

## Documentation

- **Implementation Guide**: [`docs/implementation/SIX_MOTOR_GUIDE.md`](docs/implementation/SIX_MOTOR_GUIDE.md)
- **Changes Summary**: [`docs/implementation/CHANGES_SUMMARY.md`](docs/implementation/CHANGES_SUMMARY.md)
- **Original AGENTS Guide**: [`AGENTS.md`](AGENTS.md)

## Project Structure

```
e:/night-fall/
├── docs/
│   └── implementation/
│       ├── SIX_MOTOR_GUIDE.md
│       └── CHANGES_SUMMARY.md
├── include/
│   └── config.h
├── lib/
│   ├── Communication/
│   ├── Motors/
│   ├── Safety/
│   ├── Sensors/
│   ├── Web/
│   └── Navigation/
│       └── AutonomousNav.*.removed  (Autonomous removed)
├── src/
│   ├── main_rear.cpp           (Original)
│   ├── main_rear_enhanced.cpp   (New six-motor master)
│   ├── main_front.cpp          (Original)
│   └── main_front_enhanced.cpp  (New four-motor slave)
├── platformio.ini
└── README_SIX_MOTOR.md
```

## Hardware Requirements

### Components

- 3x ESP32 Development Boards
- 3x Dual H-Bridge Motor Drivers (L298N)
- 6x DC Motors with Gearboxes
- 2x HC-SR04 Ultrasonic Sensors
- 1x MQ-2 Gas Sensor
- 1x MQ-7 Smoke Sensor
- 1x Buzzer
- 14.8V Battery Pack
- 5V Buck Converter

### Power System

- **Motors**: 14.8V directly from battery
- **ESP32s**: 5V via LM2596 buck converter
- **Sensors**: 5V from ESP32 or buck converter
- **Buzzer**: 5V from ESP32

## Safety Information

### Emergency Stop

The system includes multiple safety triggers that immediately stop all motors:

- Obstacle detection (front and rear)
- Gas leak detection
- Smoke detection
- Communication loss
- Manual emergency button

### Buzzer Alerts

- **Emergency**: Alternating beep for 5 seconds
- **Smoke**: Continuous alarm while detected
- **Status**: Different patterns for different alerts

## Troubleshooting

### Common Issues

1. **Front ESP32 Offline**: Check UART connections and power
2. **Motor Not Responding**: Verify motor driver connections
3. **Sensor Reading Invalid**: Check power and signal connections
4. **Emergency Stop Not Working**: Verify sensor thresholds

### Diagnostic Commands

```bash
# Via Serial Monitor (Front ESP32)
status    # Show system status
test       # Test all motors
motor 1-4  # Test individual motor
emergency   # Trigger emergency stop
reset       # Reset emergency stop
```

## Configuration

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

## License

This project is part of Project Nightfall. See LICENSE file for details.

## Support

For detailed implementation instructions, troubleshooting guides, and configuration options, please refer to the documentation in the [`docs/`](docs/) directory.
