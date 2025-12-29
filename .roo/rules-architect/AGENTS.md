# Project Architecture Rules (Non-Obvious Only)

**Three-ESP32 Distributed Architecture:** Master-slave pattern with ESP32 #1 (Master decision-making), ESP32 #2 (Slave motor execution), ESP32-CAM (Vision streaming) - NOT a typical single-controller design

**Preprocessor-Driven Architecture:** Board behavior completely controlled by `FRONT_CONTROLLER`, `REAR_CONTROLLER`, `CAMERA_MODULE` defines - single codebase serves three different hardware configurations

**JSON-Based Inter-Board Communication:** All communication between boards uses ArduinoJson at 115200 baud - UART protocol is NOT binary but human-readable JSON messages

**Power Distribution Architecture:** Dual-voltage system - 14.8V battery pack for motors via L298N drivers, 5V regulated via LM2596 buck converter for ESP32s and sensors

**ESP32 Voltage Architecture Constraint:** GPIO pins operate at 3.3V logic only - NOT 5V tolerant. All 5V sensors (HC-SR04, L298N) require level shifting or voltage division for GPIO connections. Power can be 5V via VIN/USB (has onboard regulator), but signals must be 3.3V.

**Sensor Fusion Architecture:** Front ESP32 receives sensor data from rear ESP32 via UART - Master makes navigation decisions based on combined front/rear sensor data

**State Machine Design:** Robot operates in discrete states (INIT/IDLE/AUTONOMOUS/EMERGENCY) with hierarchical transitions - autonomous navigation is sub-state of AUTONOMOUS mode

**Emergency Stop Architecture:** Distributed emergency stop - rear controller will halt motors if master heartbeat timeout occurs (5-second window)

**Modular Library Structure:** Despite Arduino framework, uses C++ class architecture in `lib/` with MotorControl, Sensor classes, and Communication modules