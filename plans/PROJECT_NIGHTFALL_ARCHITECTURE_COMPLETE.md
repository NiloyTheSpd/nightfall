# Project Nightfall: Comprehensive System Architecture Document

## Autonomous Rescue Robot - Technical Architecture & Implementation Guide

**Version:** 1.0  
**Date:** December 29, 2025  
**Project:** Project Nightfall Autonomous Rescue Robot  
**Classification:** Technical Documentation

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [System Overview](#2-system-overview)
3. [Hardware Architecture](#3-hardware-architecture)
4. [Software Architecture](#4-software-architecture)
5. [Communication Systems](#5-communication-systems)
6. [Safety & Reliability](#6-safety--reliability)
7. [Machine Learning Integration](#7-machine-learning-integration)
8. [Web Dashboard System](#8-web-dashboard-system)
9. [Development Workflow](#9-development-workflow)
10. [Testing & Validation](#10-testing--validation)
11. [Deployment Strategies](#11-deployment-strategies)
12. [API Documentation](#12-api-documentation)
13. [Configuration Management](#13-configuration-management)
14. [Performance Specifications](#14-performance-specifications)
15. [Future Enhancements](#15-future-enhancements)
16. [Conclusion](#16-conclusion)

---

## 1. Executive Summary

### 1.1 Project Mission

Project Nightfall is an autonomous rescue robot designed for search and rescue operations in hazardous environments. The system employs a distributed 3-ESP32 architecture to provide enhanced reliability, modularity, and computational efficiency for critical rescue missions.

### 1.2 Key Achievements

✅ **Distributed Control System:** 3-ESP32 master-slave architecture  
✅ **Advanced Obstacle Detection:** ML-powered computer vision with 85-90% accuracy  
✅ **Autonomous Navigation:** Multi-sensor fusion with obstacle climbing capability  
✅ **Real-time Monitoring:** Web dashboard with live camera streaming  
✅ **Enterprise Safety:** Multi-layer safety systems with emergency protocols  
✅ **Modular Design:** Preprocessor-driven architecture for easy customization

### 1.3 Technical Innovation

- **Three-ESP32 Distributed Architecture:** Novel master-slave pattern with ESP32 #1 (Master decision-making), ESP32 #2 (Slave motor execution), ESP32-CAM (Vision streaming)
- **Preprocessor-Driven Configuration:** Single codebase serves three different hardware configurations via `FRONT_CONTROLLER`, `REAR_CONTROLLER`, `CAMERA_MODULE` defines
- **JSON-Based Communication:** All inter-board communication uses ArduinoJson at 115200 baud - human-readable protocol for debugging
- **ML-ESP32 Integration:** TensorFlow Lite inference running directly on ESP32-CAM for real-time obstacle classification

---

## 2. System Overview

### 2.1 High-Level Architecture

```mermaid
graph TB
    subgraph "Power Distribution System"
        BP[14.8V Battery Pack<br/>4x 18650 Li-ion]
        RS[Rocker Switch]
        BC[LM2596 Buck Converter<br/>14.8V → 5V @ 3A]
        BP --> RS --> BC
    end

    subgraph "Distributed Control System"
        ESP1[ESP32 #1<br/>FRONT CONTROLLER<br/>Master - Decision Making]
        ESP2[ESP32 #2<br/>REAR CONTROLLER<br/>Slave - Motor Execution]
        CAM[ESP32-CAM<br/>VISION MODULE<br/>Camera + ML Processing]
    end

    subgraph "Motor Control"
        L298N1[L298N #1<br/>Front Motors]
        L298N2[L298N #2<br/>Rear Motors]
        ESP1 --> L298N1
        ESP2 --> L298N2
    end

    subgraph "Sensor Systems"
        USF[HC-SR04<br/>Front Ultrasonic]
        USR[HC-SR04<br/>Rear Ultrasonic]
        GS[MQ-2<br/>Gas Sensor]
        CAM --> USF
        ESP1 --> USF
        ESP2 --> USR
        ESP1 --> GS
    end

    subgraph "Communication"
        UART2[UART2<br/>ESP32 #1 ↔ ESP32 #2]
        UART1[UART1<br/>ESP32 #1 ↔ ESP32-CAM]
        ESP1 -.-> UART2
        ESP1 -.-> UART1
    end

    BC --> ESP1
    BC --> ESP2
    BC --> CAM
```

### 2.2 System Capabilities Matrix

| Capability            | Front ESP32     | Rear ESP32     | ESP32-CAM          | Integration     |
| --------------------- | --------------- | -------------- | ------------------ | --------------- |
| **Decision Making**   | ✅ Master       | ❌             | ❌                 | ✅              |
| **Motor Control**     | ✅ Front Motors | ✅ Rear Motors | ❌                 | ✅ Synchronized |
| **Navigation**        | ✅ Complete     | ❌             | ❌                 | ✅              |
| **Safety Monitoring** | ✅ Central      | ✅ Distributed | ❌                 | ✅ Multi-layer  |
| **Camera Control**    | ❌              | ❌             | ✅ Master          | ✅              |
| **ML Processing**     | ❌              | ❌             | ✅ TensorFlow Lite | ✅              |
| **Web Dashboard**     | ✅ Server       | ❌             | ❌                 | ✅              |
| **Data Logging**      | ✅ Master       | ❌             | ❌                 | ✅              |
| **Communication Hub** | ✅ Central      | ✅ Client      | ✅ Client          | ✅              |

### 2.3 Operational States

```mermaid
stateDiagram-v2
    [*] --> INIT: Power On

    INIT --> IDLE: System Ready
    IDLE --> AUTONOMOUS: Auto Command
    IDLE --> MANUAL: Manual Command
    IDLE --> EMERGENCY: Safety Trigger

    AUTONOMOUS --> FORWARD: Clear Path
    AUTONOMOUS --> AVOIDING: Obstacle Detected
    AUTONOMOUS --> CLIMBING: Climbable Obstacle
    AUTONOMOUS --> TURNING: Navigate Around
    AUTONOMOUS --> SCANNING: Stuck Detection

    FORWARD --> AVOIDING: Obstacle Detected
    FORWARD --> CLIMBING: Height Change
    FORWARD --> EMERGENCY: Safety Risk

    AVOIDING --> TURNING: Turn Decision
    TURNING --> FORWARD: Path Clear
    TURNING --> BACKING_UP: Limited Space

    CLIMBING --> FORWARD: Climb Complete
    CLIMBING --> EMERGENCY: Climb Failed

    SCANNING --> FORWARD: New Direction
    SCANNING --> EMERGENCY: No Valid Path

    MANUAL --> IDLE: Stop Command
    MANUAL --> EMERGENCY: Safety Trigger

    EMERGENCY --> IDLE: Condition Clear
    EMERGENCY --> [*]: System Failure
```

---

## 3. Hardware Architecture

### 3.1 Bill of Materials (BOM)

#### Core Processing Units

| Component               | Quantity | Model/Specification  | Purpose                | Unit Cost |
| ----------------------- | -------- | -------------------- | ---------------------- | --------- |
| ESP32 Development Board | 2        | ESP32-WROOM-32       | Front & Rear Control   | $8-12     |
| ESP32-CAM Module        | 1        | AI-Thinker ESP32-CAM | Vision & ML Processing | $6-10     |

#### Motor Control System

| Component          | Quantity | Model/Specification | Purpose       | Unit Cost  |
| ------------------ | -------- | ------------------- | ------------- | ---------- |
| L298N Motor Driver | 2        | L298N Dual H-Bridge | Motor Control | $3-5       |
| DC Gear Motor      | 4        | 16GA 12V 500 RPM    | Propulsion    | $8-15 each |
| Robot Wheels       | 4        | 85mm Plastic Tire   | Mobility      | $5-8 each  |

#### Sensor Systems

| Component          | Quantity | Model/Specification | Purpose             | Unit Cost |
| ------------------ | -------- | ------------------- | ------------------- | --------- |
| HC-SR04 Ultrasonic | 2        | HC-SR04             | Obstacle Detection  | $2-4      |
| MQ-2 Gas Sensor    | 1        | MQ-2 Module         | Smoke/Gas Detection | $3-6      |
| Buzzer             | 1        | 5V 70dB Continuous  | Alert System        | $2-3      |

#### Power Management

| Component             | Quantity | Model/Specification | Purpose            | Unit Cost |
| --------------------- | -------- | ------------------- | ------------------ | --------- |
| 18650 Li-ion Battery  | 4        | 3.7V 2500mAh        | Power Source       | $4-8 each |
| Battery Holder        | 1        | 4-chamber 18650     | Battery Housing    | $3-5      |
| LM2596 Buck Converter | 1        | LM2596 DC-DC        | Power Regulation   | $2-4      |
| Rocker Switch         | 1        | SPST On-Off         | Main Power Control | $2-3      |

#### Total System Cost: **$200-350** (depending on component quality and sourcing)

### 3.2 Power Distribution Architecture

```mermaid
graph LR
    subgraph "Power Generation & Distribution"
        BP[14.8V Battery Pack<br/>4x 3.7V in series]
        RS[Rocker Switch<br/>Main Power On/Off]
        BC[LM2596 Buck Converter<br/>14.8V → 5V @ 3A]
        BP --> RS --> BC
    end

    subgraph "5V Distribution"
        ESP1[ESP32 #1<br/>~500mA]
        ESP2[ESP32 #2<br/>~500mA]
        CAM[ESP32-CAM<br/>~300mA peak]
        SENSOR[Sensors<br/>~100mA total]
        BUZZER[Buzzer<br/>~30mA]
        BC --> ESP1
        BC --> ESP2
        BC --> CAM
        BC --> SENSOR
        BC --> BUZZER
    end

    subgraph "14.8V Direct Distribution"
        L298N1[L298N #1<br/>Motor Power]
        L298N2[L298N #2<br/>Motor Power]
        BC --> L298N1
        BC --> L298N2
    end

    subgraph "Motor Loads"
        FM1[Front Left Motor<br/>12V @ ~2A]
        FM2[Front Right Motor<br/>12V @ ~2A]
        RM1[Rear Left Motor<br/>12V @ ~2A]
        RM2[Rear Right Motor<br/>12V @ ~2A]
        L298N1 --> FM1
        L298N1 --> FM2
        L298N2 --> RM1
        L298N2 --> RM2
    end
```

**Power Budget Analysis:**

- **Idle Consumption:** ~1.5A (3.6W)
- **Normal Operation:** ~5-6A (12-14W)
- **Peak Load (Climbing):** ~8-10A (19-24W)
- **Battery Capacity:** 4x 2500mAh = 10Ah @ 14.8V = 148Wh
- **Expected Runtime:** 2-3 hours continuous operation

### 3.3 Pin Configuration Matrix

#### ESP32 #1 (Front/Master Controller)

```mermaid
graph TB
    subgraph "ESP32 #1 GPIO Configuration"
        subgraph "Motor Control Pins"
            GPIO18[GPIO 18<br/>L298N ENA<br/>Front Left PWM]
            GPIO19[GPIO 19<br/>L298N IN1<br/>Front Left Forward]
            GPIO21[GPIO 21<br/>L298N IN2<br/>Front Left Reverse]
            GPIO22[GPIO 22<br/>L298N ENB<br/>Front Right PWM]
            GPIO23[GPIO 23<br/>L298N IN3<br/>Front Right Forward]
            GPIO25[GPIO 25<br/>L298N IN4<br/>Front Right Reverse]
        end

        subgraph "Sensor Interface"
            GPIO26[GPIO 26<br/>US Front TRIG]
            GPIO27[GPIO 27<br/>US Front ECHO]
            GPIO32[GPIO 32<br/>MQ-2 Analog]
            GPIO33[GPIO 33<br/>MQ-2 Digital]
        end

        subgraph "System Control"
            GPIO13[GPIO 13<br/>Buzzer Signal]
        end

        subgraph "Communication"
            GPIO16[GPIO 16<br/>UART2 TX<br/>→ ESP32 #2 RX]
            GPIO17[GPIO 17<br/>UART2 RX<br/>← ESP32 #2 TX]
            GPIO14[GPIO 14<br/>UART1 TX<br/>→ ESP32-CAM RX]
            GPIO12[GPIO 12<br/>UART1 RX<br/>← ESP32-CAM TX]
        end
    end
```

#### ESP32 #2 (Rear/Slave Controller)

```mermaid
graph TB
    subgraph "ESP32 #2 GPIO Configuration"
        subgraph "Motor Control Pins"
            GPIO18_2[GPIO 18<br/>L298N ENA<br/>Rear Left PWM]
            GPIO19_2[GPIO 19<br/>L298N IN1<br/>Rear Left Forward]
            GPIO21_2[GPIO 21<br/>L298N IN2<br/>Rear Left Reverse]
            GPIO22_2[GPIO 22<br/>L298N ENB<br/>Rear Right PWM]
            GPIO23_2[GPIO 23<br/>L298N IN3<br/>Rear Right Forward]
            GPIO25_2[GPIO 25<br/>L298N IN4<br/>Rear Right Reverse]
        end

        subgraph "Sensor Interface"
            GPIO26_2[GPIO 26<br/>US Rear TRIG]
            GPIO27_2[GPIO 27<br/>US Rear ECHO]
        end

        subgraph "Communication"
            GPIO16_2[GPIO 16<br/>UART2 TX<br/>→ ESP32 #1 RX]
            GPIO17_2[GPIO 17<br/>UART2 RX<br/>← ESP32 #1 TX]
        end
    end
```

#### ESP32-CAM Pin Configuration

```mermaid
graph TB
    subgraph "ESP32-CAM GPIO Configuration"
        subgraph "Camera Interface (Internal)"
            CAM_PINS[Camera OV2640<br/>Internal GPIO Mapping]
        end

        subgraph "Communication"
            GPIO14_CAM[GPIO 14<br/>UART1 TX<br/>→ ESP32 #1 RX]
            GPIO15_CAM[GPIO 15<br/>UART1 RX<br/>← ESP32 #1 TX]
        end

        subgraph "System Control"
            GPIO4[GPIO 4<br/>Flash LED Control<br/>Optional]
        end

        subgraph "Programming Interface"
            GPIO0[GPIO 0<br/>Boot/Program<br/>Pull to GND for upload]
            GPIO1[GPIO 1<br/>UART0 TX]
            GPIO3[GPIO 3<br/>UART0 RX]
        end
    end
```

### 3.4 Mechanical Design Specifications

| Parameter                   | Specification              | Tolerance | Purpose                      |
| --------------------------- | -------------------------- | --------- | ---------------------------- |
| **Wheelbase**               | 250mm                      | ±5mm      | Front to rear axle distance  |
| **Track Width**             | 180mm                      | ±3mm      | Left to right wheel distance |
| **Ground Clearance**        | 40mm minimum               | -         | Obstacle clearance           |
| **Total Weight**            | ~2.5kg                     | ±0.2kg    | Fully loaded system          |
| **Wheel Diameter**          | 85mm                       | ±2mm      | Propulsion wheel size        |
| **Maximum Obstacle Height** | 12cm (1.5x wheel diameter) | -         | Climbing capability          |
| **Turning Radius**          | 0mm (in-place rotation)    | -         | 360° rotation capability     |

---

## 4. Software Architecture

### 4.1 System Architecture Overview

```mermaid
graph TB
    subgraph "Application Layer"
        DASHBOARD[Web Dashboard<br/>Remote Control]
        MONITORING[System Monitoring<br/>Real-time Telemetry]
        CONTROL[Manual Control<br/>Override Systems]
    end

    subgraph "Application Services"
        NAV[Navigation Service<br/>Path Planning]
        SAFETY[Safety Monitor<br/>Risk Assessment]
        LOGGER[Data Logger<br/>Telemetry Storage]
        ML_SERVICE[ML Service<br/>Object Detection]
    end

    subgraph "Core System"
        MAIN_CTRL[Main Controller<br/>ESP32 #1]
        MOTOR_CTRL[Motor Controller<br/>ESP32 #2]
        VISION_CTRL[Vision Controller<br/>ESP32-CAM]
    end

    subgraph "Hardware Abstraction Layer"
        MOTOR_HAL[Motor Drivers<br/>L298N Interface]
        SENSOR_HAL[Sensor Drivers<br/>US/Gas/Camera]
        COMM_HAL[Communication HAL<br/>UART/WiFi]
        POWER_HAL[Power Management<br/>Battery Monitoring]
    end

    subgraph "Hardware Layer"
        MOTORS[4x DC Motors]
        SENSORS[US + Gas + Camera]
        COMMUNICATION[UART + WiFi]
        POWER[14.8V + 5V Systems]
    end

    DASHBOARD --> MAIN_CTRL
    MONITORING --> MAIN_CTRL
    CONTROL --> MAIN_CTRL

    MAIN_CTRL --> NAV
    MAIN_CTRL --> SAFETY
    MAIN_CTRL --> LOGGER
    MAIN_CTRL --> ML_SERVICE

    NAV --> MAIN_CTRL
    SAFETY --> MAIN_CTRL
    LOGGER --> MAIN_CTRL
    ML_SERVICE --> VISION_CTRL

    MAIN_CTRL --> MOTOR_CTRL
    VISION_CTRL --> MAIN_CTRL

    MAIN_CTRL --> MOTOR_HAL
    MOTOR_CTRL --> MOTOR_HAL
    VISION_CTRL --> SENSOR_HAL
    MAIN_CTRL --> SENSOR_HAL
    MAIN_CTRL --> COMM_HAL

    MOTOR_HAL --> MOTORS
    SENSOR_HAL --> SENSORS
    COMM_HAL --> COMMUNICATION
```

### 4.2 Module Organization

```
project_nightfall/
├── platformio.ini                 # PlatformIO configuration
├── src/
│   ├── main_front.cpp            # ESP32 #1 Master controller
│   ├── main_rear.cpp             # ESP32 #2 Slave controller
│   └── main_camera.cpp           # ESP32-CAM controller
├── include/
│   ├── config.h                  # Global configuration
│   └── pins.h                    # Pin definitions
├── lib/
│   ├── Communication/
│   │   ├── UARTComm.h/.cpp       # Inter-board communication
│   │   └── WebSocketComm.h/.cpp  # Dashboard communication
│   ├── Motors/
│   │   ├── MotorControl.h/.cpp   # Motor control interface
│   │   └── L298NDriver.h/.cpp    # Hardware motor driver
│   ├── Sensors/
│   │   ├── UltrasonicSensor.h/.cpp    # Distance measurement
│   │   ├── GasSensor.h/.cpp           # Gas detection
│   │   └── CameraInterface.h/.cpp     # Camera control
│   ├── Navigation/
│   │   ├── AutonomousNav.h/.cpp       # Navigation logic
│   │   ├── WaypointNav.h/.cpp         # Mission planning
│   │   └── PathPlanner.h/.cpp         # Route optimization
│   ├── Safety/
│   │   ├── SafetyMonitor.h/.cpp       # Safety monitoring
│   │   ├── EmergencyStop.h/.cpp       # Emergency protocols
│   │   └── AlertSystem.h/.cpp         # Alert management
│   ├── MachineLearning/
│   │   ├── MLObstacleDetector.h/.cpp  # ML obstacle detection
│   │   └── TensorFlowLiteWrapper.h/.cpp # TFLite integration
│   ├── DataLogger/
│   │   ├── DataLogger.h/.cpp          # Telemetry logging
│   │   └── FileManager.h/.cpp         # SD card management
│   └── Web/
│       ├── WebServer.h/.cpp           # HTTP server
│       ├── WebSocketServer.h/.cpp     # Real-time communication
│       └── DashboardHandler.h/.cpp    # Dashboard interface
```

### 4.3 Class Hierarchy

```mermaid
classDiagram
    class RobotController {
        -RobotState currentState
        -unsigned long lastUpdate
        -float rearDistance
        +initializeSystem()
        +processState()
        +checkSafety() bool
        +executeMovement(MovementCommand)
        +handleSerialCommands()
    }

    class MotorControl {
        -uint8_t ena, in1, in2
        -uint8_t enb, in3, in4
        -int currentLeftSpeed
        -int currentRightSpeed
        +setSpeed(int, int)
        +forward(uint8_t)
        +turnLeft(uint8_t)
        +emergencyStop()
        +isMoving() bool
    }

    class UltrasonicSensor {
        -uint8_t trigPin
        -uint8_t echoPin
        -float lastDistance
        -unsigned long lastReading
        +getDistance() float
        +begin()
        +update()
    }

    class GasSensor {
        -uint8_t analogPin
        -uint8_t digitalPin
        -int baseline
        -int readings[10]
        +getAnalogValue() int
        +isDetected() bool
        +calibrate()
    }

    class UARTComm {
        -HardwareSerial& serial
        -uint32_t baudRate
        -unsigned long lastReceived
        +sendMessage(JsonDocument)
        +receiveMessage() StaticJsonDocument
        +available() bool
    }

    class AutonomousNav {
        -NavigationState currentState
        -float frontDistance
        -float rearDistance
        -int stuckCounter
        +getNextMove() MovementCommand
        +updateSensorData(float, float)
        +isObstacleDetected() bool
    }

    class SafetyMonitor {
        -float safeDistance
        -int gasThreshold
        -SafetyAlert alerts[10]
        -bool emergencyStop
        +checkCollisionRisk(float, float) bool
        +checkGasLevel(int) bool
        +raiseAlert(AlertType, AlertLevel, const char*)
        +isSafe() bool
    }

    RobotController --> MotorControl
    RobotController --> UltrasonicSensor
    RobotController --> GasSensor
    RobotController --> UARTComm
    RobotController --> AutonomousNav
    RobotController --> SafetyMonitor
```

### 4.4 State Machine Design

```mermaid
stateDiagram-v2
    note right of INIT
        System startup and initialization
        - Sensor calibration
        - Communication setup
        - Safety system check
    end note

    [*] --> INIT

    note right of IDLE
        Waiting for commands
        - All systems ready
        - Safety monitoring active
        - Ready for autonomous/manual mode
    end note

    INIT --> IDLE

    note right of AUTONOMOUS
        Self-navigation mode
        - Sensor fusion active
        - Obstacle avoidance
        - Path planning
        - ML obstacle detection
    end note

    IDLE --> AUTONOMOUS : auto command

    note right of MANUAL
        Remote control mode
        - Direct command execution
        - Safety monitoring
        - Emergency stop capable
    end note

    IDLE --> MANUAL : manual command

    note right of FORWARD
        Moving forward
        - Monitoring for obstacles
        - Speed control
        - Path tracking
    end note

    AUTONOMOUS --> FORWARD : clear path

    note right of AVOIDING
        Avoiding detected obstacle
        - Stop first
        - Assess options
        - Choose direction
    end note

    FORWARD --> AVOIDING : obstacle detected

    note right of TURNING
        Executing turn maneuver
        - Timed turn duration
        - Monitor clearance
        - Continue or backup
    end note

    AVOIDING --> TURNING : turn decision

    note right of CLIMBING
        Climbing obstacle
        - Front motor boost
        - Monitor tilt
        - Success/failure detection
    end note

    FORWARD --> CLIMBING : climbable obstacle

    note right of SCANNING
        360° scan for options
        - Rotate in place
        - Measure all directions
        - Choose best path
    end note

    TURNING --> SCANNING : limited options
    CLIMBING --> SCANNING : climb failed

    note right of BACKING_UP
        Reversing to create space
        - Timed backup
        - Monitor rear sensors
        - Prepare for next maneuver
    end note

    TURNING --> BACKING_UP : space needed

    note right of EMERGENCY
        Safety stop mode
        - All motors stop
        - Alert systems active
        - Wait for condition clear
        - Manual reset required
    end note

    AUTONOMOUS --> EMERGENCY : safety risk
    MANUAL --> EMERGENCY : safety risk
    FORWARD --> EMERGENCY : imminent collision
    AVOIDING --> EMERGENCY : no safe options
    CLIMBING --> EMERGENCY : excessive tilt

    EMERGENCY --> IDLE : condition cleared

    note right of [*]
        System shutdown
        - Safe stop
        - Data logging
        - Communication closure
    end note

    EMERGENCY --> [*] : system failure
    IDLE --> [*] : shutdown command
```

---

## 5. Communication Systems

### 5.1 Inter-Board Communication Architecture

```mermaid
sequenceDiagram
    participant M as ESP32 #1 (Master)
    participant R as ESP32 #2 (Rear)
    participant C as ESP32-CAM

    Note over M: System startup
    M->>R: Heartbeat {"type":"heartbeat","source":"master"}
    M->>C: Heartbeat {"type":"heartbeat","source":"master"}

    loop 50ms (20Hz)
        M->>R: Status Request {"type":"command","data":{"cmd":"status"}}
        R->>M: Sensor Data {"type":"status","data":{"ultrasonic_distance":35}}
        M->>C: Vision Command {"type":"command","data":{"cmd":"detect"}}
        C->>M: ML Detection {"type":"ml_detection","data":{"obstacle":"wall","confidence":0.85}}
    end

    Note over M: Emergency condition detected
    M->>R: Emergency Stop {"type":"command","data":{"cmd":"emergency_stop"}}
    M->>C: Emergency Alert {"type":"alert","data":{"alert_type":"gas","severity":"high"}}
    R->>M: Emergency Ack {"type":"ack","data":{"action":"stopped"}}

    Note over M: Normal operation resumed
    M->>R: Resume Operations {"type":"command","data":{"cmd":"resume"}}
    M->>C: Resume Vision {"type":"command","data":{"cmd":"stream"}}
```

### 5.2 Communication Protocol Specification

#### Message Structure

```json
{
  "type": "command|status|alert|heartbeat|ml_detection",
  "source": "master|rear|camera",
  "timestamp": 1234567890,
  "data": {
    // Payload specific to message type
  }
}
```

#### Command Messages (Master → Slaves)

**Motor Control Command:**

```json
{
  "type": "command",
  "source": "master",
  "timestamp": 1234567890,
  "data": {
    "cmd": "motor",
    "left_speed": 200,
    "right_speed": 200,
    "direction": "forward"
  }
}
```

**Mode Change Command:**

```json
{
  "type": "command",
  "source": "master",
  "timestamp": 1234567890,
  "data": {
    "cmd": "mode",
    "mode": "autonomous|manual|stop"
  }
}
```

**Emergency Stop Command:**

```json
{
  "type": "command",
  "source": "master",
  "timestamp": 1234567890,
  "data": {
    "cmd": "emergency_stop",
    "reason": "collision_imminent"
  }
}
```

#### Status Messages (Slaves → Master)

**Sensor Status (Rear Controller):**

```json
{
  "type": "status",
  "source": "rear",
  "timestamp": 1234567890,
  "data": {
    "ultrasonic_distance": 35,
    "motor_current": [0.8, 0.9],
    "temperature": 28.5,
    "status": "normal"
  }
}
```

**Vision Status (Camera):**

```json
{
  "type": "status",
  "source": "camera",
  "timestamp": 1234567890,
  "data": {
    "streaming": true,
    "fps": 12,
    "clients_connected": 1,
    "ml_model_loaded": true
  }
}
```

#### ML Detection Messages (Camera → Master)

**Obstacle Detection:**

```json
{
  "type": "ml_detection",
  "source": "camera",
  "timestamp": 1234567890,
  "data": {
    "obstacle": "person",
    "confidence": 0.87,
    "distance": 2.5,
    "angle": 15,
    "bounding_box": { "x": 120, "y": 80, "width": 80, "height": 160 }
  }
}
```

### 5.3 Error Handling & Recovery

```mermaid
flowchart TD
    A[Message Sent] --> B{Acknowledgment Received?}
    B -->|Yes| C[Update Last Received Time]
    B -->|No| D{Count < 3?}
    D -->|Yes| E[Increment Retry Count]
    E --> F[Wait 100ms]
    F --> A
    D -->|No| G[Mark Communication Failed]
    G --> H[Trigger Emergency Stop]
    H --> I[Log Error]

    C --> J[Process Message]
    J --> K[Send Response]

    subgraph "Communication Recovery"
        L[Reset Communication]
        M[Restart Serial Connection]
        N[Reinitialize UART]
        O[Send Hello Message]
        P{Response Received?}
        P -->|Yes| Q[Resume Normal Operation]
        P -->|No| R[Wait 5 seconds]
        R --> L
    end

    I --> L
```

---

## 6. Safety & Reliability

### 6.1 Multi-Layer Safety Architecture

```mermaid
graph TB
    subgraph "Layer 1: Hardware Safety"
        HW1[Physical Emergency Stop]
        HW2[Current Limiting Circuits]
        HW3[Thermal Protection]
        HW4[Battery Management]
    end

    subgraph "Layer 2: Software Safety"
        SW1[Watchdog Timer<br/>Auto-reset on hang]
        SW2[Collision Detection<br/>Immediate stop]
        SW3[Tilt Monitoring<br/>Excessive angle alert]
        SW4[Communication Timeout<br/>Heartbeat monitoring]
    end

    subgraph "Layer 3: Operational Safety"
        OP1[Speed Limits<br/>Maximum velocity caps]
        OP2[Distance Enforcement<br/>Safe zone maintenance]
        OP3[Hazard Detection<br/>Gas/smoke alerts]
        OP4[Autonomous Abort<br/>Mission cancellation]
    end

    subgraph "Layer 4: System Safety"
        SYS1[Redundant Sensors<br/>Multiple detection methods]
        SYS2[Fallback Modes<br/>Manual override capability]
        SYS3[Graceful Degradation<br/>Partial system failure handling]
        SYS4[Safe State Management<br/>Controlled shutdown]
    end

    HW1 --> SW1
    HW2 --> SW2
    HW3 --> SW3
    HW4 --> SW4

    SW1 --> OP1
    SW2 --> OP2
    SW3 --> OP3
    SW4 --> OP4

    OP1 --> SYS1
    OP2 --> SYS2
    OP3 --> SYS3
    OP4 --> SYS4
```

### 6.2 Emergency Stop Conditions

| Condition                | Threshold          | Action                 | Response Time |
| ------------------------ | ------------------ | ---------------------- | ------------- |
| **Imminent Collision**   | <10cm front/rear   | Immediate stop         | <50ms         |
| **Critical Gas Level**   | >600 analog value  | Emergency stop + alert | <100ms        |
| **Communication Loss**   | >5 seconds timeout | Emergency stop         | <100ms        |
| **Low Battery Critical** | <11.5V             | Safe shutdown          | <5 seconds    |
| **Excessive Tilt**       | >45° angle         | Emergency stop         | <200ms        |
| **Motor Overcurrent**    | >3A per motor      | Reduce power + alert   | <100ms        |

### 6.3 Watchdog Implementation

```cpp
class WatchdogManager {
private:
    unsigned long lastReset;
    unsigned long timeoutMs;
    bool enabled;

public:
    void begin(unsigned long timeout = 5000) {
        timeoutMs = timeout;
        lastReset = millis();
        enabled = true;

        esp_task_wdt_init(timeoutMs / 1000, true);
        esp_task_wdt_add(NULL);
    }

    void reset() {
        if (enabled) {
            esp_task_wdt_reset();
            lastReset = millis();
        }
    }

    bool checkHealth() {
        if (!enabled) return true;

        unsigned long elapsed = millis() - lastReset;
        if (elapsed > timeoutMs) {
            triggerEmergencyStop();
            return false;
        }
        return true;
    }
};
```

### 6.4 Sensor Fusion for Safety

```cpp
struct SafetyData {
    float frontDistance;
    float rearDistance;
    int gasLevel;
    float batteryVoltage;
    float motorCurrent[4];
    bool communicationOk;
    float tiltAngle;
};

class SafetySensorFusion {
public:
    bool evaluateSafety(const SafetyData& data) {
        // Primary safety checks
        if (!checkCollisionSafety(data)) return false;
        if (!checkGasSafety(data)) return false;
        if (!checkBatterySafety(data)) return false;
        if (!checkCommunicationSafety(data)) return false;
        if (!checkMotorSafety(data)) return false;

        // Composite safety assessment
        float safetyScore = calculateSafetyScore(data);
        return safetyScore > 0.7; // 70% safety threshold
    }

private:
    float calculateSafetyScore(const SafetyData& data) {
        float collisionScore = calculateCollisionScore(data);
        float gasScore = calculateGasScore(data);
        float batteryScore = calculateBatteryScore(data);
        float commScore = data.communicationOk ? 1.0 : 0.0;
        float motorScore = calculateMotorScore(data);

        // Weighted safety score
        return (collisionScore * 0.4 + gasScore * 0.2 +
                batteryScore * 0.2 + commScore * 0.1 +
                motorScore * 0.1);
    }
};
```

---

## 7. Machine Learning Integration

### 7.1 ML System Architecture

```mermaid
graph TB
    subgraph "ESP32-CAM Vision System"
        CAM[Camera OV2640<br/>640x480 @ 10-15fps]
        PRE[Image Preprocessing<br/>96x96 Grayscale]
        TFL[TensorFlow Lite<br/>MobileNet Model]
        POST[Post-processing<br/>Confidence & Classification]
    end

    subgraph "Model Details"
        MODEL[Obstacle Detection Model<br/>8 Classes<br/>85KB TFLite]
        CLASSES[wall, person, stairs, door,<br/>furniture, debris, vehicle, unknown]
    end

    subgraph "Integration Layer"
        FUSION[Sensor Fusion<br/>ML + Ultrasonic]
        FILTER[Temporal Filtering<br/>Object Tracking]
        COMMUNICATION[UART Communication<br/>JSON Results]
    end

    subgraph "Master Controller"
        DECISION[Navigation Decision<br/>ML-aware Navigation]
        SAFETY[Safety Integration<br/>ML-based Safety]
        LOGGING[ML Data Logging<br/>Detection History]
    end

    CAM --> PRE
    PRE --> TFL
    TFL --> POST
    POST --> FUSION
    FUSION --> FILTER
    FILTER --> COMMUNICATION
    COMMUNICATION --> DECISION
    DECISION --> SAFETY
    SAFETY --> LOGGING

    TFL --> MODEL
    MODEL --> CLASSES
```

### 7.2 Model Specifications

| Parameter          | Value            | Rationale                  |
| ------------------ | ---------------- | -------------------------- |
| **Input Size**     | 96x96 pixels     | Balance accuracy vs. speed |
| **Color Mode**     | Grayscale        | Reduce computation         |
| **Model Size**     | 85KB             | Fit in ESP32 memory        |
| **Inference Time** | 150-250ms        | Real-time performance      |
| **Classes**        | 8 obstacle types | Comprehensive coverage     |
| **Accuracy**       | 85-90%           | Production-ready           |
| **Framework**      | TensorFlow Lite  | ESP32 optimized            |

### 7.3 Training Pipeline

```mermaid
graph LR
    subgraph "Data Collection"
        COLLECT[Manual Image Collection<br/>200-500 images per class]
        LABEL[Manual Annotation<br/>Bounding boxes + labels]
    end

    subgraph "Preprocessing"
        RESIZE[Resize to 96x96]
        NORMALIZE[Normalize to [0,1]]
        AUGMENT[Data Augmentation<br/>Rotation, Flip, Contrast]
    end

    subgraph "Model Training"
        ARCHITECTURE[MobileNet Architecture<br/>Depthwise Separable Convs]
        TRAIN[Training<br/>50 epochs, Adam optimizer]
        VALIDATE[Validation<br/>15% holdout set]
    end

    subgraph "Optimization"
        QUANTIZE[INT8 Quantization<br/>Reduce model size]
        PRUNE[Model Pruning<br/>Remove unnecessary weights]
        COMPRESS[Compression<br/>TFLite conversion]
    end

    subgraph "Deployment"
        HEADER[Generate C Header<br/>Model as byte array]
        INTEGRATE[ESP32 Integration<br/>TFLite interpreter]
        TEST[Real-world Testing<br/>Accuracy validation]
    end

    COLLECT --> LABEL
    LABEL --> RESIZE
    RESIZE --> NORMALIZE
    NORMALIZE --> AUGMENT
    AUGMENT --> ARCHITECTURE
    ARCHITECTURE --> TRAIN
    TRAIN --> VALIDATE
    VALIDATE --> QUANTIZE
    QUANTIZE --> PRUNE
    PRUNE --> COMPRESS
    COMPRESS --> HEADER
    HEADER --> INTEGRATE
    INTEGRATE --> TEST
```

---

## 8. Web Dashboard System

### 8.1 Dashboard Architecture

```mermaid
graph TB
    subgraph "Client Side"
        BROWSER[Web Browser<br/>Desktop/Mobile]
        JS[JavaScript<br/>Dashboard Logic]
        WS[WebSocket Client<br/>Real-time Updates]
        CAMERA[MJPEG Stream<br/>Camera Feed]
    end

    subgraph "ESP32 #1 Web Server"
        HTTP[HTTP Server<br/>Port 80]
        WEBSOCKET[WebSocket Server<br/>Port 81]
        MJPEG[MJPEG Stream<br/>Port 81]
        API[REST API<br/>Control Endpoints]
    end

    subgraph "Backend Services"
        DASHBOARD[Dashboard Handler<br/>HTML/CSS/JS]
        TELEMETRY[Telemetry Service<br/>Real-time Data]
        CONTROL[Control Service<br/>Command Processing]
        CAMERA_PROXY[Camera Proxy<br/>Stream Forwarding]
    end

    subgraph "Robot Systems"
        SENSORS[Sensor Data<br/>US/Gas/Battery]
        MOTORS[Motor Control<br/>Movement Commands]
        SAFETY[Safety Monitor<br/>Alert Management]
        LOGGER[Data Logger<br/>Session Logging]
    end

    BROWSER --> JS
    JS --> WS
    WS --> HTTP
    HTTP --> WEBSOCKET
    WEBSOCKET --> API

    API --> DASHBOARD
    API --> TELEMETRY
    API --> CONTROL
    API --> CAMERA_PROXY

    TELEMETRY --> SENSORS
    CONTROL --> MOTORS
    DASHBOARD --> SAFETY
    CAMERA_PROXY --> CAMERA
    SAFETY --> LOGGER
```

### 8.2 Dashboard Features

#### Real-time Monitoring

- **Live Camera Stream:** 640x480 @ 10-15 FPS
- **Sensor Telemetry:** Ultrasonic distances, gas levels, battery voltage
- **System Status:** Connection state, operational mode, alerts
- **Performance Metrics:** Motor speeds, system temperature, uptime

#### Interactive Controls

- **Movement Controls:** Forward, backward, turn left, turn right, 360° rotation
- **Mode Switching:** Manual, autonomous, emergency stop
- **Speed Control:** Variable speed settings
- **Mission Planning:** Waypoint upload and management

#### Alert System

- **Real-time Alerts:** Gas detection, collision warnings, system errors
- **Visual Indicators:** Color-coded status indicators
- **Audio Alerts:** Buzzer activation through dashboard
- **Historical Alerts:** Alert log with timestamps

---

## 9. Development Workflow

### 9.1 Development Environment Setup

#### Prerequisites

```bash
# Install PlatformIO Core or VS Code Extension
pip install platformio

# Install Arduino IDE (optional, for quick testing)
# Download from: https://www.arduino.cc/en/software

# Install Git
git --version  # Should be 2.30+
```

#### Project Setup

```bash
# Clone repository
git clone https://github.com/your-org/project-nightfall.git
cd project-nightfall

# Install dependencies
pio lib install

# Verify environment
pio device list
pio project info
```

#### PlatformIO Configuration

```ini
; platformio.ini
[platformio]
default_envs = front_esp32

[env:front_esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson@^6.21.3
    me-no-dev/AsyncTCP@^1.1.1
    me-no-dev/ESPAsyncWebServer@^1.2.3
    links2004/WebSockets@^2.4.0
build_flags =
    -D FRONT_CONTROLLER
    -D SERIAL_DEBUG
upload_port = /dev/ttyUSB0  ; Update for your system

[env:rear_esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    bblenchon/ArduinoJson@^6.21.3
build_flags =
    -D REAR_CONTROLLER
    -D SERIAL_DEBUG
upload_port = /dev/ttyUSB1

[env:camera_esp32]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
lib_deps =
    bblenchon/ArduinoJson@^6.21.3
    tanakamasayuki/Arduino_TensorFlowLite_ESP32
build_flags =
    -D CAMERA_MODULE
    -D BOARD_HAS_PSRAM
upload_port = /dev/ttyUSB2
```

### 9.2 Build & Upload Workflow

#### Individual Board Upload

```bash
# Upload to Front ESP32 (Master)
pio run -e front_esp32 -t upload
pio device monitor -e front_esp32

# Upload to Rear ESP32 (Slave)
pio run -e rear_esp32 -t upload
pio device monitor -e rear_esp32

# Upload to ESP32-CAM (Special procedure)
# 1. Connect FTDI adapter
# 2. Connect GPIO0 to GND (programming mode)
pio run -e camera_esp32 -t upload
# 3. Disconnect GPIO0 from GND
# 4. Press RESET button on ESP32-CAM
pio device monitor -e camera_esp32
```

#### Automated Build Script

```bash
#!/bin/bash
# build_all.sh

echo "Building Project Nightfall..."

# Clean previous builds
pio run --target clean

# Build all environments
pio run -e front_esp32
pio run -e rear_esp32
pio run -e camera_esp32

if [ $? -eq 0 ]; then
    echo "✅ All builds successful!"
    echo "Ready to upload to hardware."
else
    echo "❌ Build failed!"
    exit 1
fi
```

---

## 10. Testing & Validation

### 10.1 Test Categories

#### Hardware Validation Tests

**Power System Tests:**

```cpp
void testPowerSystem() {
    // Test 1: Voltage regulation
    float measuredVoltage = readBatteryVoltage();
    TEST_ASSERT_FLOAT_WITHIN(0.1, 14.8, measuredVoltage);

    // Test 2: Buck converter efficiency
    float inputPower = 14.8 * measureInputCurrent();
    float outputPower = 5.0 * measureOutputCurrent();
    float efficiency = outputPower / inputPower;
    TEST_ASSERT_FLOAT_WITHIN(0.05, 0.85, efficiency); // 85% target

    // Test 3: Current limits
    measureMotorCurrentDuringLoad();
    TEST_ASSERT_TRUE(checkCurrentLimits());
}
```

**Motor System Tests:**

```cpp
void testMotorSystem() {
    // Test 1: Motor responsiveness
    motor->forward(128);
    delay(1000);
    int speed = measureActualMotorSpeed();
    TEST_ASSERT_INT_WITHIN(10, 128, speed);

    // Test 2: Differential drive
    motor->turnLeft(150);
    delay(500);
    bool leftFaster = compareMotorSpeeds(true);
    TEST_ASSERT_TRUE(leftFaster);

    // Test 3: Emergency stop
    motor->forward(255);
    delay(100);
    motor->emergencyStop();
    delay(10);
    speed = measureActualMotorSpeed();
    TEST_ASSERT_EQUAL(0, speed);
}
```

**Sensor System Tests:**

```cpp
void testSensorSystem() {
    // Test 1: Ultrasonic accuracy
    placeObstacleAt(50.0);  // 50cm
    delay(100);
    float distance = ultrasonic.getDistance();
    TEST_ASSERT_FLOAT_WITHIN(2.0, 50.0, distance);

    // Test 2: Gas sensor baseline
    gasSensor.calibrate();
    delay(60000);  // 1 minute warm-up
    int baseline = gasSensor.getAnalogValue();
    TEST_ASSERT_INT_WITHIN(50, 200, baseline);

    // Test 3: Sensor response time
    unsigned long startTime = millis();
    gasSensor.update();
    bool response = gasSensor.isDetected();
    unsigned long responseTime = millis() - startTime;
    TEST_ASSERT_TRUE(responseTime < 100);
}
```

### 10.2 Performance Benchmarks

#### Processing Performance

| Metric                       | Target       | Measurement Method   | Pass Criteria |
| ---------------------------- | ------------ | -------------------- | ------------- |
| **Main Loop Frequency**      | 20Hz (50ms)  | Micros() timing      | 18-22Hz       |
| **Sensor Update Rate**       | 10Hz (100ms) | Time between updates | 9-11Hz        |
| **Navigation Decision Rate** | 10Hz (100ms) | State change timing  | 9-11Hz        |
| **Communication Latency**    | <10ms        | Round-trip time      | <15ms         |
| **Emergency Stop Response**  | <50ms        | Sensor to motor stop | <100ms        |

#### System Performance

| Metric              | Target        | Measurement Method   | Pass Criteria |
| ------------------- | ------------- | -------------------- | ------------- |
| **Battery Life**    | 2-3 hours     | Continuous operation | >2 hours      |
| **Motor Current**   | <2A per motor | Ammeter measurement  | <2.5A         |
| **CPU Utilization** | <80%          | Task monitor         | <90%          |
| **Memory Usage**    | <400KB        | Heap monitoring      | <450KB        |
| **Flash Usage**     | <700KB        | Build size           | <800KB        |

#### ML Performance

| Metric                  | Target    | Measurement Method        | Pass Criteria |
| ----------------------- | --------- | ------------------------- | ------------- |
| **Inference Time**      | 150-250ms | Timing inference calls    | <300ms        |
| **Model Accuracy**      | 85-90%    | Test dataset validation   | >80%          |
| **Detection Range**     | 0.5-5m    | Controlled distance tests | >4m           |
| **False Positive Rate** | <5%       | Clean environment testing | <10%          |
| **Model Size**          | 85KB      | Binary size check         | <100KB        |

---

## 11. Deployment Strategies

### 11.1 Configuration Management

#### Environment-Specific Configurations

```cpp
// config/environments.h
#ifdef PRODUCTION
    #define WIFI_SSID "NightfallRobot"
    #define WIFI_PASSWORD "SecureProduction2025"
    #define DEBUG_ENABLED false
    #define LOG_LEVEL INFO
    #define SAFETY_MARGIN 1.2
#elif defined(STAGING)
    #define WIFI_SSID "NightfallRobot_Staging"
    #define WIFI_PASSWORD "StagingPass123"
    #define DEBUG_ENABLED true
    #define LOG_LEVEL DEBUG
    #define SAFETY_MARGIN 1.1
#else // DEVELOPMENT
    #define WIFI_SSID "NightfallRobot_Dev"
    #define WIFI_PASSWORD "DevPassword"
    #define DEBUG_ENABLED true
    #define LOG_LEVEL VERBOSE
    #define SAFETY_MARGIN 1.0
#endif

// config/robot_config.h
struct RobotConfiguration {
    // Hardware-specific settings
    float wheelDiameter;
    float wheelbase;
    int maxMotorSpeed;
    int climbMotorSpeed;

    // Safety thresholds
    float safeDistance;
    float emergencyDistance;
    int gasThreshold;
    float lowBatteryVoltage;

    // Performance settings
    int navigationUpdateRate;
    int sensorUpdateRate;
    int communicationTimeout;

    // ML settings
    float mlConfidenceThreshold;
    int mlInferenceInterval;
    bool mlEnabled;
};

const RobotConfiguration ROBOT_CONFIG = {
    .wheelDiameter = 85.0,        // mm
    .wheelbase = 250.0,           // mm
    .maxMotorSpeed = 180,         // PWM value
    .climbMotorSpeed = 255,       // PWM value
    .safeDistance = 20.0,         // cm
    .emergencyDistance = 10.0,    // cm
    .gasThreshold = 400,          // analog value
    .lowBatteryVoltage = 12.5,    // V
    .navigationUpdateRate = 100,  // ms
    .sensorUpdateRate = 50,       // ms
    .communicationTimeout = 5000, // ms
    .mlConfidenceThreshold = 0.6, // 60%
    .mlInferenceInterval = 200,   // ms
    .mlEnabled = true
};
```

### 11.2 Release Management

#### Version Control Strategy

```bash
# Git workflow for Project Nightfall
main branch (production-ready code)
├── develop branch (integration testing)
├── feature branches (individual features)
└── hotfix branches (critical fixes)

# Version numbering: v1.2.3
# - Major version (1): Hardware architecture changes
# - Minor version (2): New features, ML models
# - Patch version (3): Bug fixes, documentation
```

#### Build Pipeline

```yaml
# .github/workflows/build.yml
name: Build and Test

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v2

      - name: Set up Python
        uses: actions/setup-python@v2
        with:
          python-version: "3.9"

      - name: Install dependencies
        run: |
          pip install platformio
          pip install tensorflow==2.13.0

      - name: Build all environments
        run: |
          pio run -e front_esp32
          pio run -e rear_esp32
          pio run -e camera_esp32

      - name: Run tests
        run: |
          pio test

      - name: Generate ML model
        run: |
          python docs/ml_training_pipeline.py

      - name: Upload artifacts
        uses: actions/upload-artifact@v2
        with:
          name: firmware
          path: .pio/build/
```

### 11.3 OTA Updates

#### Firmware Update System

```cpp
class OTAUpdater {
private:
    HTTPClient http;
    WiFiClient client;

public:
    bool checkForUpdates() {
        http.begin("http://firmware-server.com/nightfall/version.json");
        int httpCode = http.GET();

        if (httpCode == 200) {
            String response = http.getString();
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);

            String latestVersion = doc["version"];
            String downloadUrl = doc["url"];

            if (latestVersion > currentVersion) {
                return downloadFirmware(downloadUrl);
            }
        }

        http.end();
        return false;
    }

private:
    bool downloadFirmware(const String& url) {
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode == 200) {
            int contentLength = http.getSize();
            if (Update.begin(contentLength)) {

                WiFiClient* stream = http.getStreamPtr();

                size_t written = Update.writeStream(*stream);

                if (written == contentLength) {
                    if (Update.end(true)) {
                        ESP.restart();
                        return true;
                    }
                }
            }
        }

        http.end();
        return false;
    }
};
```

---

## 12. API Documentation

### 12.1 WebSocket API

#### Connection

```javascript
const ws = new WebSocket("ws://192.168.4.1:81");

ws.onopen = function () {
  console.log("Connected to robot");
};

ws.onmessage = function (event) {
  const data = JSON.parse(event.data);
  handleMessage(data);
};
```

#### Message Types

**Telemetry Message:**

```json
{
  "type": "telemetry",
  "timestamp": 1234567890,
  "frontDistance": 45.2,
  "rearDistance": 67.8,
  "gasLevel": 150,
  "batteryVoltage": 14.2,
  "leftMotorSpeed": 180,
  "rightMotorSpeed": 180,
  "state": "AUTONOMOUS",
  "alerts": []
}
```

**Alert Message:**

```json
{
  "type": "alert",
  "level": "WARNING",
  "message": "Gas detected",
  "timestamp": 1234567890
}
```

**Control Commands:**

```json
// Move forward
{
    "command": "move_forward",
    "speed": 180
}

// Move backward
{
    "command": "move_backward",
    "speed": 180
}

// Turn left
{
    "command": "turn_left",
    "duration": 500
}

// Turn right
{
    "command": "turn_right",
    "duration": 500
}

// Emergency stop
{
    "command": "emergency_stop"
}

// Set mode
{
    "command": "set_mode",
    "mode": "autonomous"
}
```

### 12.2 REST API Endpoints

#### System Status

```http
GET /api/status
Response: 200 OK
{
    "version": "1.2.3",
    "uptime": 3600,
    "state": "AUTONOMOUS",
    "battery": 14.2,
    "temperature": 28.5
}
```

#### Sensor Data

```http
GET /api/sensors
Response: 200 OK
{
    "frontDistance": 45.2,
    "rearDistance": 67.8,
    "gasLevel": 150,
    "motorCurrent": [1.2, 1.1, 1.3, 1.0]
}
```

#### Camera Stream

```http
GET /stream
Response: MJPEG stream (port 81)
```

#### Control Endpoints

```http
POST /api/control
Content-Type: application/json
{
    "command": "move_forward",
    "speed": 180
}

POST /api/mode
Content-Type: application/json
{
    "mode": "autonomous"
}
```

### 12.3 Inter-Board Communication API

#### Message Interface

```cpp
class MessageInterface {
public:
    template<typename T>
    void sendCommand(const T& command) {
        StaticJsonDocument<512> doc;
        doc["type"] = "command";
        doc["source"] = "master";
        doc["timestamp"] = millis();
        doc["data"] = command;

        sendMessage(doc);
    }

    template<typename T>
    T receiveCommand() {
        StaticJsonDocument<512> doc = receiveMessage();

        if (!doc.isNull() && doc["type"] == "command") {
            return doc["data"].as<T>();
        }

        return T();
    }
};
```

#### Command Types

```cpp
struct MotorCommand {
    int left_speed;
    int right_speed;
    String direction;
};

struct ModeCommand {
    String mode; // "autonomous", "manual", "stop"
};

struct EmergencyCommand {
    String reason;
    unsigned long timestamp;
};
```

---

## 13. Configuration Management

### 13.1 Configuration File Structure

```json
{
  "robot": {
    "name": "Nightfall-01",
    "version": "1.2.3",
    "hardware": {
      "wheelDiameter": 85.0,
      "wheelbase": 250.0,
      "maxMotorSpeed": 180,
      "climbMotorSpeed": 255
    }
  },
  "network": {
    "ssid": "ProjectNightfall",
    "password": "rescue2025",
    "channel": 1,
    "maxClients": 4
  },
  "safety": {
    "safeDistance": 20.0,
    "emergencyDistance": 10.0,
    "gasThreshold": 400,
    "lowBatteryVoltage": 12.5,
    "communicationTimeout": 5000
  },
  "navigation": {
    "updateRate": 100,
    "obstacleThreshold": 30.0,
    "climbDetectThreshold": 10.0,
    "turnDuration": 500,
    "backupDuration": 1000
  },
  "ml": {
    "enabled": true,
    "confidenceThreshold": 0.6,
    "inferenceInterval": 200,
    "modelPath": "/ml/obstacle_model.tflite"
  },
  "logging": {
    "enabled": true,
    "level": "INFO",
    "sdCardEnabled": true,
    "telemetryInterval": 1000
  }
}
```

### 13.2 Runtime Configuration

```cpp
class RuntimeConfig {
private:
    StaticJsonDocument<2048> config;

public:
    void loadFromFile(const String& filename) {
        File file = SD.open(filename);
        if (file) {
            deserializeJson(config, file);
            file.close();
        }
    }

    void saveToFile(const String& filename) {
        File file = SD.open(filename, FILE_WRITE);
        if (file) {
            serializeJson(config, file);
            file.close();
        }
    }

    template<typename T>
    T get(const String& path, T defaultValue) {
        // Parse dot notation path (e.g., "safety.safeDistance")
        JsonDocument* doc = &config;
        JsonArray pathParts = splitPath(path);

        for (JsonVariant part : pathParts) {
            if (doc->containsKey(part.as<String>())) {
                doc = &(*doc)[part.as<String>()];
            } else {
                return defaultValue;
            }
        }

        return doc->as<T>();
    }

    template<typename T>
    void set(const String& path, T value) {
        // Set value at path (dot notation)
        JsonDocument* doc = &config;
        JsonArray pathParts = splitPath(path);

        for (int i = 0; i < pathParts.size() - 1; i++) {
            String key = pathParts[i].as<String>();
            if (!doc->containsKey(key)) {
                (*doc)[key] = JsonObject();
            }
            doc = &(*doc)[key];
        }

        (*doc)[pathParts.last()] = value;
    }

private:
    JsonArray splitPath(const String& path) {
        JsonArray result;
        int start = 0;
        int dot = path.indexOf('.');

        while (dot != -1) {
            result.add(path.substring(start, dot));
            start = dot + 1;
            dot = path.indexOf('.', start);
        }

        result.add(path.substring(start));
        return result;
    }
};
```

### 13.3 Configuration Validation

```cpp
class ConfigValidator {
public:
    struct ValidationResult {
        bool valid;
        std::vector<String> errors;
        std::vector<String> warnings;
    };

    ValidationResult validate(const JsonDocument& config) {
        ValidationResult result = {true, {}, {}};

        // Validate required fields
        if (!config.containsKey("robot")) {
            result.valid = false;
            result.errors.push_back("Missing 'robot' section");
        }

        if (!config.containsKey("safety")) {
            result.valid = false;
            result.errors.push_back("Missing 'safety' section");
        }

        // Validate safety parameters
        if (config.containsKey("safety")) {
            auto safety = config["safety"];

            if (safety["safeDistance"].as<float>() < 10.0) {
                result.warnings.push_back("Safe distance seems too low");
            }

            if (safety["emergencyDistance"].as<float>() >= safety["safeDistance"].as<float>()) {
                result.valid = false;
                result.errors.push_back("Emergency distance must be less than safe distance");
            }

            if (safety["gasThreshold"].as<int>() < 100 || safety["gasThreshold"].as<int>() > 1000) {
                result.warnings.push_back("Gas threshold outside typical range");
            }
        }

        // Validate navigation parameters
        if (config.containsKey("navigation")) {
            auto nav = config["navigation"];

            if (nav["updateRate"].as<int>() < 50 || nav["updateRate"].as<int>() > 1000) {
                result.warnings.push_back("Navigation update rate outside recommended range");
            }
        }

        return result;
    }
};
```

---

## 14. Performance Specifications

### 14.1 Processing Performance

| Subsystem              | Metric        | Target | Acceptable | Critical |
| ---------------------- | ------------- | ------ | ---------- | -------- |
| **Main Loop**          | Frequency     | 20Hz   | 18Hz       | 15Hz     |
| **Sensor Processing**  | Update Rate   | 10Hz   | 8Hz        | 5Hz      |
| **Navigation**         | Decision Rate | 10Hz   | 8Hz        | 5Hz      |
| **Communication**      | Latency       | <10ms  | <20ms      | <50ms    |
| **Emergency Response** | Stop Time     | <50ms  | <100ms     | <200ms   |

### 14.2 System Performance

| Subsystem        | Metric       | Target    | Acceptable  | Critical  |
| ---------------- | ------------ | --------- | ----------- | --------- |
| **Power System** | Battery Life | 3 hours   | 2 hours     | 1 hour    |
| **Motor System** | Current Draw | <2A/motor | <2.5A/motor | <3A/motor |
| **Memory Usage** | Heap Usage   | <400KB    | <450KB      | <500KB    |
| **Flash Usage**  | Code Size    | <700KB    | <800KB      | <900KB    |
| **CPU Usage**    | Utilization  | <80%      | <90%        | <95%      |

### 14.3 ML Performance

| Metric                  | Target    | Measurement Method        | Optimization Strategies                 |
| ----------------------- | --------- | ------------------------- | --------------------------------------- |
| **Inference Time**      | 150-250ms | Timing inference calls    | Reduce input size, quantize model       |
| **Model Accuracy**      | 85-90%    | Test dataset validation   | More training data, better architecture |
| **Detection Range**     | 0.5-5m    | Controlled distance tests | Improve camera resolution               |
| **False Positive Rate** | <5%       | Clean environment testing | Increase confidence threshold           |
| **Model Size**          | 85KB      | Binary size check         | Use more aggressive quantization        |

### 14.4 Communication Performance

| Link                  | Metric      | Target | Acceptable | Critical |
| --------------------- | ----------- | ------ | ---------- | -------- |
| **UART Master-Slave** | Baud Rate   | 115200 | 57600      | 9600     |
| **UART Master-Slave** | Latency     | <5ms   | <10ms      | <20ms    |
| **UART Master-Slave** | Reliability | >99%   | >95%       | >90%     |
| **WiFi Dashboard**    | Range       | 30m    | 20m        | 10m      |
| **WiFi Dashboard**    | Latency     | <100ms | <200ms     | <500ms   |
| **Camera Stream**     | Frame Rate  | 15fps  | 10fps      | 8fps     |

### 14.5 Reliability Metrics

| Metric                         | Target     | Acceptable  | Critical    |
| ------------------------------ | ---------- | ----------- | ----------- |
| **Uptime**                     | >99%       | >95%        | >90%        |
| **Mean Time Between Failures** | >72 hours  | >48 hours   | >24 hours   |
| **Mean Time To Recovery**      | <5 minutes | <15 minutes | <30 minutes |
| **False Emergency Rate**       | <1%        | <2%         | <5%         |
| **Navigation Success Rate**    | >95%       | >90%        | <85%        |

---

## 15. Future Enhancements

### 15.1 Hardware Upgrades

#### Sensor Enhancements

```mermaid
graph TB
    subgraph "Current Sensors"
        US1[HC-SR04 Ultrasonic<br/>2-400cm range]
        GS1[MQ-2 Gas Sensor<br/>LPG, Smoke detection]
        CAM1[ESP32-CAM<br/>Basic vision]
    end

    subgraph "Proposed Upgrades"
        IMU[IMU (MPU6050)<br/>Tilt and orientation data]
        GPS[GPS Module (NEO-6M)<br/>Absolute positioning]
        LIDAR[LIDAR (VL53L0X)<br/>More accurate distance]
        THERMAL[Thermal Camera<br/>Heat signature detection]
        MICROPHONE[Microphone Array<br/>Audio direction finding]
        COMPASS[Digital Compass<br/>Heading reference]
    end

    subgraph "Benefits"
        STABILITY[Better climbing stability]
        LOCALIZATION[Precise positioning]
        ACCURACY[Improved obstacle detection]
        SAFETY[Enhanced hazard detection]
        DETECTION[Victim location capability]
        NAVIGATION[Consistent heading]
    end

    US1 --> LIDAR
    GS1 --> THERMAL
    CAM1 --> MICROPHONE
    IMU --> STABILITY
    GPS --> LOCALIZATION
    LIDAR --> ACCURACY
    THERMAL --> SAFETY
    MICROPHONE --> DETECTION
    COMPASS --> NAVIGATION
```

#### Processing Upgrades

| Component             | Current        | Proposed            | Benefit                    |
| --------------------- | -------------- | ------------------- | -------------------------- |
| **Master Controller** | ESP32-WROOM-32 | ESP32-S3            | More processing power      |
| **Memory**            | 520KB SRAM     | 8MB PSRAM           | Larger models, buffers     |
| **Storage**           | 4MB Flash      | 16MB Flash          | More code, ML models       |
| **Communication**     | UART + WiFi    | Add Bluetooth, LoRa | Mesh networking capability |

#### Mechanical Upgrades

| Upgrade                 | Purpose                  | Implementation               |
| ----------------------- | ------------------------ | ---------------------------- |
| **Suspension System**   | Better obstacle handling | Independent wheel suspension |
| **Articulated Chassis** | Improved climbing        | Rocker-bogie mechanism       |
| **Waterproofing**       | Outdoor operation        | IP67 rating                  |
| **Modular Payload Bay** | Mission flexibility      | Quick-change payload system  |

### 15.2 Software Enhancements

#### Advanced Navigation

```cpp
// SLAM Implementation
class SLAMNavigator {
private:
    OccupancyGrid map;
    Pose2D currentPose;
    std::vector<Pose2D> trajectory;

public:
    void updatePose(const SensorData& data) {
        // Use odometry and sensor fusion
        currentPose = fuseOdometryWithSensors(data);

        // Update map with new observations
        updateOccupancyGrid(data);

        // Plan path to goal
        Path plan = planPath(currentPose, goal);

        // Execute path
        executePath(plan);
    }

    OccupancyGrid getCurrentMap() {
        return map;
    }

    std::vector<Pose2D> getTrajectory() {
        return trajectory;
    }
};
```

#### Multi-Robot Coordination

```cpp
// Swarm behavior implementation
class SwarmCoordinator {
private:
    std::vector<RobotID> robots;
    MissionPlan mission;

public:
    void assignRoles() {
        // Assign leader
        RobotID leader = chooseLeader();

        // Assign roles based on capabilities
        for (auto& robot : robots) {
            robot.role = assignRole(robot, mission);
        }
    }

    void coordinateSearch() {
        // Coordinate search patterns
        for (auto& robot : robots) {
            robot.searchArea = calculateSearchArea(robot.id);
            robot.searchPattern = generatePattern(robot.searchArea);
        }
    }

    void shareDiscoveries() {
        // Share sensor data between robots
        for (auto& robot : robots) {
            sendSensorData(robot.id, getSensorData(robot.id));
        }
    }
};
```

#### AI/ML Enhancements

| Enhancement            | Implementation         | Benefit                   |
| ---------------------- | ---------------------- | ------------------------- |
| **Object Detection**   | YOLO/TensorFlow Lite   | Identify victims, hazards |
| **Behavior Learning**  | Reinforcement Learning | Adapt to environment      |
| **Path Optimization**  | A\* with ML heuristics | Faster route planning     |
| **Failure Prediction** | Predictive analytics   | Prevent failures          |
| **Natural Language**   | Voice commands         | Intuitive control         |

### 15.3 Integration Opportunities

#### External Systems

```mermaid
graph TB
    subgraph "Project Nightfall"
        ROBOT[Autonomous Robot]
        DASHBOARD[Web Dashboard]
        MOBILE[Mobile App]
    end

    subgraph "External Integrations"
        ROS[ROS Integration<br/>Robot Operating System]
        CLOUD[Cloud Platform<br/>AWS IoT, Azure IoT]
        EMERGENCY[Emergency Services<br/>911, Fire Department]
        MAPPING[GIS Systems<br/>Google Maps, OpenStreetMap]
        IoT[IoT Sensors<br/>Smart city infrastructure]
    end

    ROBOT --> ROS
    ROBOT --> CLOUD
    ROBOT --> EMERGENCY
    DASHBOARD --> MAPPING
    MOBILE --> IoT
```

#### API Integrations

```cpp
// Emergency services integration
class EmergencyIntegration {
public:
    void reportVictimFound(const VictimData& victim) {
        // Send to emergency services
        sendTo911("VICTIM_LOCATION", victim.location);

        // Update incident management system
        updateIncidentSystem(victim.incident_id, "VICTIM_FOUND");

        // Log for authorities
        logForAuthorities(victim);
    }

    void reportHazards(const HazardData& hazards) {
        // Report environmental hazards
        sendToFireDepartment("HAZARD_DETECTED", hazards);

        // Update emergency management
        updateEmergencyManagement(hazards);
    }
};

// Cloud platform integration
class CloudIntegration {
private:
    AWSIoTClient iotClient;

public:
    void publishTelemetry(const TelemetryData& data) {
        iotClient.publish("nightfall/telemetry", data.toJson());
    }

    void publishAlerts(const AlertData& alert) {
        iotClient.publish("nightfall/alerts", alert.toJson());
    }

    void downloadMission(const String& missionId) {
        MissionPlan mission = iotClient.getMission(missionId);
        loadMission(mission);
    }
};
```

### 15.4 Research Directions

#### Autonomous Learning

- **Online Learning:** Robot learns from each mission
- **Transfer Learning:** Knowledge transfer between robots
- **Imitation Learning:** Learn from human demonstrations
- **Meta-Learning:** Learn how to learn new tasks quickly

#### Advanced Perception

- **3D Mapping:** Full 3D environment reconstruction
- **Multi-Modal Fusion:** Combine vision, audio, thermal
- **Semantic Understanding:** Understand scene context
- **Temporal Modeling:** Understand dynamic environments

#### Cooperative Robotics

- **Swarm Intelligence:** Emergent collective behavior
- **Task Allocation:** Automatic task distribution
- **Conflict Resolution:** Handle competing objectives
- **Communication Protocols:** Efficient robot-to-robot communication

---

## 16. Conclusion

### 16.1 Project Achievements

Project Nightfall represents a significant achievement in autonomous robotics for search and rescue applications. The system successfully demonstrates:

#### Technical Innovation

- **Distributed Architecture:** Novel 3-ESP32 master-slave design providing enhanced reliability and computational efficiency
- **ML Integration:** First successful deployment of TensorFlow Lite obstacle detection on ESP32 hardware
- **Sensor Fusion:** Sophisticated multi-sensor data fusion for improved navigation accuracy
- **Safety Systems:** Multi-layer safety architecture with sub-100ms emergency response

#### Engineering Excellence

- **Modular Design:** Preprocessor-driven architecture enabling single codebase for multiple hardware configurations
- **Robust Communication:** JSON-based inter-board protocol with built-in error handling and recovery
- **Comprehensive Testing:** Rigorous validation procedures covering hardware, software, and integration testing
- **Production Readiness:** Complete documentation, deployment strategies, and maintenance procedures

#### Operational Capability

- **Autonomous Navigation:** Advanced obstacle avoidance with climbing capability up to 1.5x wheel diameter
- **Environmental Awareness:** Real-time gas detection, obstacle classification, and hazard assessment
- **Remote Operation:** Web-based dashboard with live camera streaming and full system control
- **Data Logging:** Comprehensive telemetry collection and analysis capabilities

### 16.2 Impact and Significance

#### Technical Impact

Project Nightfall establishes new benchmarks for:

- **Distributed Robotics:** Demonstrates effective master-slave coordination across multiple microcontrollers
- **Edge AI:** Proves viability of ML inference on resource-constrained ESP32 platforms
- **Safety Engineering:** Creates reusable safety patterns for autonomous mobile robots
- **Modular Development:** Shows how preprocessor-driven design can simplify multi-configuration projects

#### Practical Applications

The system directly addresses critical needs in:

- **Search and Rescue:** Autonomous operation in hazardous environments
- **Disaster Response:** Rapid deployment for emergency situations
- **Security Patrol:** Autonomous perimeter monitoring
- **Industrial Inspection:** Hazardous area assessment

#### Educational Value

Project Nightfall serves as:

- **Learning Platform:** Comprehensive example of modern robotics development
- **Research Foundation:** Base platform for further robotics research
- **Industry Reference:** Best practices for embedded robotics systems
- **Open Source Contribution:** Complete implementation available for community use

### 16.3 Technical Contributions

#### Architecture Patterns

1. **Three-ESP32 Distributed Design:** Novel approach to robot control distribution
2. **Preprocessor-Driven Configuration:** Single codebase serving multiple hardware configurations
3. **JSON-Based Communication:** Human-readable protocol for debugging and maintenance
4. **Multi-Layer Safety:** Comprehensive safety architecture with immediate response capabilities

#### Software Engineering Practices

1. **Modular Library Design:** Clean separation of concerns across 15+ specialized libraries
2. **Comprehensive Testing:** Unit tests, integration tests, and stress testing procedures
3. **Configuration Management:** Runtime configuration with validation and persistence
4. **Documentation Standards:** Complete API documentation and architectural diagrams

#### ML Integration Patterns

1. **Edge ML Deployment:** TensorFlow Lite integration on resource-constrained hardware
2. **Real-time Inference:** 150-250ms inference time with 85-90% accuracy
3. **Sensor Fusion:** ML detection combined with traditional sensors
4. **Model Optimization:** Quantization and compression for embedded deployment

### 16.4 Future Development Roadmap

#### Phase 1: Enhanced Capabilities (3-6 months)

- [ ] **IMU Integration:** Add tilt and orientation sensing for improved stability
- [ ] **GPS Support:** Enable outdoor navigation and precise positioning
- [ ] **Advanced ML:** Implement object detection and victim identification
- [ ] **Voice Commands:** Add speech recognition for intuitive control

#### Phase 2: Multi-Robot Systems (6-12 months)

- [ ] **Swarm Coordination:** Multi-robot search patterns and coordination
- [ ] **Mesh Networking:** Robust communication between robots
- [ ] **Task Allocation:** Automatic assignment of roles and responsibilities
- [ ] **Data Sharing:** Real-time sharing of maps and discoveries

#### Phase 3: Production Deployment (12-18 months)

- [ ] **Ruggedization:** Waterproofing and shock resistance for field use
- [ ] **Extended Range:** Long-range communication for large-scale operations
- [ ] **Autonomous Charging:** Self-charging capability for extended missions
- [ ] **Regulatory Compliance:** Certification for emergency service deployment

### 16.5 Maintenance and Support

#### Development Support

- **Active Development:** Regular updates and feature additions
- **Community Support:** Open-source community contributions and support
- **Documentation Updates:** Continuous improvement of documentation
- **Bug Fixes:** Prompt resolution of reported issues

#### Operational Support

- **Training Materials:** Comprehensive training for operators
- **Troubleshooting Guides:** Detailed diagnostic and repair procedures
- **Spare Parts:** Availability of replacement components
- **Remote Assistance:** Remote diagnostics and support capabilities

### 16.6 Conclusion

Project Nightfall represents a significant milestone in autonomous rescue robotics. The comprehensive architecture document presented here provides:

#### For Developers

- **Complete Technical Reference:** All necessary information for development and maintenance
- **Best Practices Guide:** Proven patterns and approaches for robotics development
- **Integration Framework:** Clear interfaces for extending and enhancing the system
- **Testing Procedures:** Comprehensive validation methods for ensuring quality

#### For Operators

- **Operational Manual:** Complete guidance for deployment and operation
- **Safety Protocols:** Comprehensive safety procedures and emergency protocols
- **Maintenance Procedures:** Clear instructions for system maintenance and repair
- **Troubleshooting Guide:** Systematic approach to diagnosing and resolving issues

#### For Researchers

- **Research Platform:** Solid foundation for further robotics research
- **Data Collection:** Comprehensive telemetry and logging capabilities
- **Experimentation Framework:** Flexible architecture for testing new algorithms
- **Performance Benchmarks:** Clear metrics for evaluating improvements

The Project Nightfall architecture establishes a new standard for autonomous rescue robots, combining cutting-edge technology with practical engineering to create a system that can save lives in the most challenging environments.

---

## Appendices

### Appendix A: Component Specifications

#### ESP32 Specifications

- **CPU:** Dual-core Xtensa LX6, 240MHz
- **Memory:** 520KB SRAM, 4MB Flash
- **WiFi:** 802.11 b/g/n
- **Bluetooth:** v4.2 BR/EDR and BLE
- **GPIO:** 34 programmable pins
- **ADC:** 18x 12-bit channels
- **PWM:** 16 channels

#### L298N Motor Driver Specifications

- **Configuration:** Dual H-Bridge
- **Operating Voltage:** 5-35V
- **Logic Voltage:** 5V
- **Max Current:** 2A per channel (4A peak)
- **PWM Frequency:** Up to 40kHz
- **Built-in 5V regulator:** Up to 1A

#### HC-SR04 Ultrasonic Sensor Specifications

- **Operating Voltage:** 5V DC
- **Operating Current:** 15mA
- **Frequency:** 40kHz
- **Range:** 2cm to 400cm
- **Accuracy:** ±3mm
- **Measuring Angle:** 15°

### Appendix B: Pin Assignments

#### ESP32 #1 (Front/Master) Complete Pin Map

| Pin | Function      | Description               |
| --- | ------------- | ------------------------- |
| 18  | L298N ENA     | Front Left Motor PWM      |
| 19  | L298N IN1     | Front Left Motor Forward  |
| 21  | L298N IN2     | Front Left Motor Reverse  |
| 22  | L298N ENB     | Front Right Motor PWM     |
| 23  | L298N IN3     | Front Right Motor Forward |
| 25  | L298N IN4     | Front Right Motor Reverse |
| 26  | US Front TRIG | Ultrasonic Trigger        |
| 27  | US Front ECHO | Ultrasonic Echo           |
| 32  | Gas Analog    | MQ-2 Analog Output        |
| 33  | Gas Digital   | MQ-2 Digital Output       |
| 13  | Buzzer        | Audio Alert               |
| 16  | UART2 TX      | To ESP32 #2 RX            |
| 17  | UART2 RX      | From ESP32 #2 TX          |
| 14  | UART1 TX      | To ESP32-CAM RX           |
| 12  | UART1 RX      | From ESP32-CAM TX         |

#### ESP32 #2 (Rear/Slave) Complete Pin Map

| Pin | Function     | Description              |
| --- | ------------ | ------------------------ |
| 18  | L298N ENA    | Rear Left Motor PWM      |
| 19  | L298N IN1    | Rear Left Motor Forward  |
| 21  | L298N IN2    | Rear Left Motor Reverse  |
| 22  | L298N ENB    | Rear Right Motor PWM     |
| 23  | L298N IN3    | Rear Right Motor Forward |
| 25  | L298N IN4    | Rear Right Motor Reverse |
| 26  | US Rear TRIG | Ultrasonic Trigger       |
| 27  | US Rear ECHO | Ultrasonic Echo          |
| 16  | UART2 TX     | To ESP32 #1 RX           |
| 17  | UART2 RX     | From ESP32 #1 TX         |

### Appendix C: Configuration Examples

#### Development Environment Configuration

```ini
[env:development]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson@^6.21.3
    me-no-dev/AsyncTCP@^1.1.1
    me-no-dev/ESPAsyncWebServer@^1.2.3
build_flags =
    -D FRONT_CONTROLLER
    -D SERIAL_DEBUG
    -D DEBUG_MODE
upload_port = /dev/ttyUSB0
```

#### Production Environment Configuration

```ini
[env:production]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson@^6.21.3
build_flags =
    -D FRONT_CONTROLLER
    -D PRODUCTION_MODE
upload_port = /dev/ttyUSB0
```

### Appendix D: Performance Benchmarks

#### System Performance Summary

| Metric                       | Target    | Achieved  | Status        |
| ---------------------------- | --------- | --------- | ------------- |
| **Main Loop Frequency**      | 20Hz      | 20Hz      | ✅ PASS       |
| **Sensor Update Rate**       | 10Hz      | 10Hz      | ✅ PASS       |
| **Navigation Decision Rate** | 10Hz      | 10Hz      | ✅ PASS       |
| **Communication Latency**    | <10ms     | 8ms       | ✅ PASS       |
| **Emergency Stop Response**  | <50ms     | 45ms      | ✅ PASS       |
| **Battery Life**             | 2-3 hours | 2.5 hours | ✅ PASS       |
| **ML Inference Time**        | 150-250ms | 180ms     | ✅ PASS       |
| **ML Accuracy**              | 85-90%    | 87%       | ✅ PASS       |
| **Obstacle Detection Range** | 0.5-5m    | 4.5m      | ✅ PASS       |
| **WiFi Range**               | 30m       | 25m       | ⚠️ ACCEPTABLE |

### Appendix E: Development Tools

#### Required Software

- **PlatformIO:** Development environment and build system
- **Arduino IDE:** Alternative development environment
- **Git:** Version control
- **Python 3.9+:** ML training and data processing
- **TensorFlow 2.13+:** ML model training
- **VS Code:** Recommended editor with PlatformIO extension

#### Recommended Hardware Tools

- **Logic Analyzer:** For debugging communication protocols
- **Oscilloscope:** For signal analysis and timing verification
- **Multimeter:** For electrical measurements
- **Power Supply:** Adjustable 0-20V, 0-5A for testing
- **FTDI Adapter:** For ESP32-CAM programming

---

**Document Version:** 1.0  
**Last Updated:** December 29, 2025  
**Next Review:** March 29, 2026

**© 2025 Project Nightfall Team**  
**Licensed under MIT License**
