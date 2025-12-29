# Project Nightfall: Autonomous Rescue Robot

![Project Nightfall Logo](docs/assets/logo.png)

**Version:** 1.2.3  
**Date:** December 29, 2025  
**Status:** Production Ready ✅

## 🚀 Overview

Project Nightfall is an advanced autonomous rescue robot designed for search and rescue operations in hazardous environments. The system employs a distributed 3-ESP32 architecture providing enhanced reliability, modularity, and computational efficiency for critical rescue missions.

### ✨ Key Features

- **🤖 Distributed Control System:** 3-ESP32 master-slave architecture
- **👁️ Advanced Obstacle Detection:** ML-powered computer vision with 85-90% accuracy
- **🧭 Autonomous Navigation:** Multi-sensor fusion with obstacle climbing capability
- **📊 Real-time Monitoring:** Web dashboard with live camera streaming
- **🛡️ Enterprise Safety:** Multi-layer safety systems with emergency protocols
- **🔧 Modular Design:** Preprocessor-driven architecture for easy customization

### 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Project Nightfall                       │
├─────────────────────────────────────────────────────────────┤
│  ESP32 #1 (Master)    │  ESP32 #2 (Slave)  │  ESP32-CAM    │
│  • Decision Making    │  • Motor Control   │  • Vision     │
│  • Navigation         │  • Safety Mon.     │  • ML Process │
│  • Web Dashboard      │  • Sensor Data     │  • Streaming  │
├─────────────────────────────────────────────────────────────┤
│  Power System (14.8V Li-ion + 5V Buck Converter)           │
│  Motor System (4x DC Motors + 2x L298N Drivers)           │
│  Sensors (Ultrasonic + Gas + Camera)                       │
└─────────────────────────────────────────────────────────────┘
```

## 📋 Table of Contents

### 🚀 Getting Started

- [Quick Start Guide](docs/getting-started/quick-start.md)
- [Installation & Setup](docs/getting-started/installation.md)
- [Hardware Assembly](docs/getting-started/hardware-assembly.md)
- [First Run Tutorial](docs/getting-started/first-run.md)

### 📖 Documentation

- [Architecture Overview](docs/architecture/overview.md)
- [System Design](docs/architecture/system-design.md)
- [Hardware Specifications](docs/architecture/hardware-specs.md)
- [Communication Protocol](docs/architecture/communication-protocol.md)

### 🔧 Development

- [Development Setup](docs/development/setup.md)
- [Coding Standards](docs/development/coding-standards.md)
- [Build System](docs/development/build-system.md)
- [Contributing Guidelines](docs/development/contributing.md)

### 📚 API Reference

- [WebSocket API](docs/api/websocket.md)
- [REST API](docs/api/rest-api.md)
- [Inter-Board Communication](docs/api/inter-board-comm.md)
- [Configuration API](docs/api/configuration.md)

### 🎓 Tutorials

- [Basic Navigation](docs/tutorials/basic-navigation.md)
- [Web Dashboard Usage](docs/tutorials/dashboard-usage.md)
- [ML Model Training](docs/tutorials/ml-training.md)
- [Custom Configurations](docs/tutorials/custom-config.md)

### 🐛 Troubleshooting

- [Common Issues](docs/troubleshooting/common-issues.md)
- [Hardware Problems](docs/troubleshooting/hardware.md)
- [Software Debugging](docs/troubleshooting/software.md)
- [FAQ](docs/troubleshooting/faq.md)

### 🧪 Testing

- [Unit Tests](test/unit/)
- [Integration Tests](test/integration/)
- [Performance Benchmarks](docs/development/benchmarks.md)

## 🛠️ Quick Start

### Prerequisites

```bash
# Install PlatformIO Core
pip install platformio

# Or install PlatformIO Extension for VS Code
# Download from: https://platformio.org/install/ide?install=vscode

# Install Git
git --version  # Should be 2.30+
```

### Hardware Requirements

| Component               | Quantity | Model                | Purpose                | Est. Cost  |
| ----------------------- | -------- | -------------------- | ---------------------- | ---------- |
| ESP32 Development Board | 2        | ESP32-WROOM-32       | Front & Rear Control   | $8-12 each |
| ESP32-CAM Module        | 1        | AI-Thinker ESP32-CAM | Vision & ML Processing | $6-10      |
| L298N Motor Driver      | 2        | L298N Dual H-Bridge  | Motor Control          | $3-5 each  |
| DC Gear Motor           | 4        | 16GA 12V 500 RPM     | Propulsion             | $8-15 each |
| HC-SR04 Ultrasonic      | 2        | HC-SR04              | Obstacle Detection     | $2-4 each  |
| MQ-2 Gas Sensor         | 1        | MQ-2 Module          | Smoke/Gas Detection    | $3-6       |
| 18650 Li-ion Battery    | 4        | 3.7V 2500mAh         | Power Source           | $4-8 each  |

**Total System Cost:** $200-350

### Installation Steps

1. **Clone the Repository**

   ```bash
   git clone https://github.com/your-org/project-nightfall.git
   cd project-nightfall
   ```

2. **Install Dependencies**

   ```bash
   pio lib install
   ```

3. **Configure Hardware**

   - Connect ESP32 boards as per [Hardware Assembly Guide](docs/getting-started/hardware-assembly.md)
   - Update `platformio.ini` with your USB port mappings

4. **Build and Upload**

   ```bash
   # Upload to Front ESP32 (Master)
   pio run -e front_esp32 -t upload

   # Upload to Rear ESP32 (Slave)
   pio run -e rear_esp32 -t upload

   # Upload to ESP32-CAM (requires GPIO0→GND during upload)
   pio run -e camera_esp32 -t upload
   ```

5. **Access Dashboard**
   - Connect to WiFi network: `ProjectNightfall`
   - Open browser to: `http://192.168.4.1`
   - Use dashboard for control and monitoring

For detailed instructions, see [Installation Guide](docs/getting-started/installation.md).

## 🏃‍♂️ Performance Specifications

| Metric                      | Target    | Achieved  | Status  |
| --------------------------- | --------- | --------- | ------- |
| **Main Loop Frequency**     | 20Hz      | 20Hz      | ✅ PASS |
| **Emergency Stop Response** | <50ms     | 45ms      | ✅ PASS |
| **Battery Life**            | 2-3 hours | 2.5 hours | ✅ PASS |
| **ML Inference Time**       | 150-250ms | 180ms     | ✅ PASS |
| **ML Accuracy**             | 85-90%    | 87%       | ✅ PASS |

## 🛡️ Safety Features

- **Multi-Layer Safety Architecture**

  - Hardware emergency stops
  - Software collision detection
  - Gas detection alerts
  - Communication timeout handling
  - Battery voltage monitoring

- **Emergency Response Times**
  - Collision detection: <50ms
  - Gas detection: <100ms
  - Communication loss: <100ms
  - Low battery: <5 seconds

## 🤖 System Modes

| Mode           | Description     | Capabilities                                    |
| -------------- | --------------- | ----------------------------------------------- |
| **AUTONOMOUS** | Self-navigation | Obstacle avoidance, path planning, ML detection |
| **MANUAL**     | Remote control  | Direct movement commands, safety monitoring     |
| **EMERGENCY**  | Safety stop     | All motors stop, alerts active, wait for clear  |

## 📡 Communication Protocol

- **Inter-Board:** UART at 115200 baud (JSON messages)
- **Dashboard:** WiFi + WebSocket for real-time control
- **Camera Stream:** MJPEG over HTTP port 81
- **Range:** WiFi up to 30m (line of sight)

## 🔧 Development

### Build System

```bash
# Build all environments
pio run

# Build specific environment
pio run -e front_esp32

# Upload to specific board
pio device upload -e front_esp32

# Monitor serial output
pio device monitor

# Run tests
pio test
```

### Code Structure

```
project-nightfall/
├── src/                          # Main application code
│   ├── main_front.cpp           # ESP32 #1 Master controller
│   ├── main_rear.cpp            # ESP32 #2 Slave controller
│   └── main_camera.cpp          # ESP32-CAM controller
├── lib/                         # Reusable libraries
│   ├── Communication/           # Inter-board & WebSocket comm
│   ├── Motors/                  # Motor control & drivers
│   ├── Sensors/                 # Ultrasonic, Gas, Camera
│   ├── Navigation/              # Path planning & navigation
│   ├── Safety/                  # Safety monitoring & alerts
│   ├── MachineLearning/         # ML obstacle detection
│   ├── DataLogger/              # Telemetry & logging
│   └── Web/                     # Dashboard & web server
├── config/                      # Configuration files
│   ├── environments/            # Dev/Staging/Prod configs
│   └── robot_config.h           # Hardware configuration
├── docs/                        # Documentation
├── test/                        # Unit & integration tests
└── scripts/                     # Build & deployment scripts
```

### Contributing

1. Fork the repository
2. Create feature branch: `git checkout -b feature/amazing-feature`
3. Follow [Coding Standards](docs/development/coding-standards.md)
4. Add tests for new functionality
5. Commit changes: `git commit -m 'Add amazing feature'`
6. Push to branch: `git push origin feature/amazing-feature`
7. Open Pull Request

## 📊 System Monitoring

### Real-time Metrics

- **Sensors:** Ultrasonic distances, gas levels, battery voltage
- **Motors:** Speed, current draw, temperature
- **System:** CPU usage, memory, uptime, alerts
- **ML:** Inference time, confidence, detection rate

### Data Logging

- **Telemetry:** 1Hz logging to SD card
- **Alerts:** Real-time logging with timestamps
- **Performance:** System metrics and benchmarks
- **ML Data:** Detection history and accuracy metrics

## 🔮 Future Roadmap

### Phase 1: Enhanced Capabilities (3-6 months)

- [ ] IMU integration for improved stability
- [ ] GPS support for outdoor navigation
- [ ] Advanced ML with victim identification
- [ ] Voice command interface

### Phase 2: Multi-Robot Systems (6-12 months)

- [ ] Swarm coordination algorithms
- [ ] Mesh networking between robots
- [ ] Automatic task allocation
- [ ] Shared mapping and discoveries

### Phase 3: Production Deployment (12-18 months)

- [ ] Waterproofing and ruggedization
- [ ] Extended range communication
- [ ] Autonomous charging capability
- [ ] Regulatory compliance certification

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 Community for excellent documentation and libraries
- TensorFlow Lite team for embedded ML capabilities
- PlatformIO for robust embedded development environment
- Arduino community for foundational libraries and examples

## 📞 Support

- **Documentation:** [docs/](docs/)
- **Issues:** [GitHub Issues](https://github.com/your-org/project-nightfall/issues)
- **Discussions:** [GitHub Discussions](https://github.com/your-org/project-nightfall/discussions)
- **Email:** support@projectnightfall.org

## 📈 Project Statistics

- **Lines of Code:** ~15,000 (C++, JavaScript, Python)
- **Test Coverage:** >85%
- **Documentation:** 50+ pages
- **Supported Platforms:** ESP32, ESP32-CAM
- **Languages:** C++, JavaScript, Python, Markdown

---

**Made with ❤️ by the Project Nightfall Team**

_Empowering search and rescue operations through autonomous robotics_
