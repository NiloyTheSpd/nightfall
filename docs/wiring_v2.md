# Test Version 2 - Complete Wiring Documentation

**Version:** 2.0.0  
**Date:** December 29, 2025  
**Project:** Project Nightfall Autonomous Rescue Robot  
**Test Version:** 2 - Unified WiFi Networking Architecture  
**Classification:** Hardware Assembly Documentation

---

## Table of Contents

1. [Hardware Overview](#1-hardware-overview)
2. [System Architecture](#2-system-architecture)
3. [Power Distribution System](#3-power-distribution-system)
4. [Rear ESP32 (Master Controller)](#4-rear-esp32-master-controller)
5. [Front ESP32 (Motor Controller)](#5-front-esp32-motor-controller)
6. [ESP32-CAM (Vision Module)](#6-esp32-cam-vision-module)
7. [Motor Driver Wiring](#7-motor-driver-wiring)
8. [Sensor Connections](#8-sensor-connections)
9. [UART Communication Network](#9-uart-communication-network)
10. [WiFi Network Topology](#10-wifi-network-topology)
11. [Electrical Specifications](#11-electrical-specifications)
12. [Wire Gauge and Connectors](#12-wire-gauge-and-connectors)
13. [Safety Considerations](#13-safety-considerations)
14. [EMI Protection](#14-emi-protection)
15. [Assembly Instructions](#15-assembly-instructions)
16. [Testing and Verification](#16-testing-and-verification)
17. [Troubleshooting Guide](#17-troubleshooting-guide)
18. [Bill of Materials](#18-bill-of-materials)
19. [Tools Required](#19-tools-required)

---

## 1. Hardware Overview

### 1.1 System Architecture - Test Version 2

```
┌─────────────────────────────────────────────────────────────────┐
│                    PROJECT NIGHTFALL ROBOT                      │
│                    Test Version 2 Architecture                   │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │  REAR ESP32 │    │ FRONT ESP32 │    │  ESP32-CAM  │        │
│  │   (Master)  │◄──►│  (Motor)    │◄──►│   (Vision)  │        │
│  │ 192.168.4.1 │    │ 192.168.4.2 │    │ 192.168.4.3 │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
│         │                  │                  │               │
│         │                  │                  │               │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │  Sensors    │    │  Motors     │    │   Camera    │        │
│  │ - Ultrasonic│    │ - 4x DC     │    │   OV2640    │        │
│  │ - Gas       │    │ - L298N x2  │    │   Flash     │        │
│  │ - Buzzer    │    │ - Wheels    │    │   WiFi      │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
├─────────────────────────────────────────────────────────────────┤
│                    POWER DISTRIBUTION                           │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │14.8V Battery│────►LM2596 Buck │────►5V Rail    │        │
│  │  4S Li-ion  │    │   12V→5V    │    │  3A Max     │        │
│  │  2500mAh    │    │    3A       │    │            │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
│         │                  │                  │               │
│         └──────────────────┴──────────────────┘               │
│                    Common Ground                               │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 Core Components (Updated for 6-Motor System)

| Component              | Model               | Quantity   | Function             | Voltage | Current        |
| ---------------------- | ------------------- | ---------- | -------------------- | ------- | -------------- |
| **ESP32 DevKitC**      | ESP32-WROOM-32      | 2          | Master/Motor Control | 5V      | 500mA          |
| **ESP32-CAM**          | AI-Thinker          | 1          | Vision Processing    | 5V      | 300mA          |
| **L298N Motor Driver** | L298N Dual H-Bridge | 3          | Motor Control        | 12V/5V  | 2A per channel |
| **DC Motors**          | 12V 500 RPM         | 6          | Propulsion           | 12V     | 1.5A each      |
| **Li-ion Battery**     | 18650 3.7V          | 4 (series) | Power Source         | 14.8V   | 2500mAh        |
| **Buck Converter**     | LM2596 DC-DC        | 1          | 12V→5V Conversion    | 12V/5V  | 3A             |
| **HC-SR04 Ultrasonic** | HC-SR04             | 2          | Distance Sensing     | 5V      | 15mA           |
| **MQ-2 Gas Sensor**    | MQ-2 Module         | 1          | Gas Detection        | 5V      | 133mA          |
| **Active Buzzer**      | 5V 70dB             | 1          | Audio Alert          | 5V      | 30mA           |
| **Robot Wheels**       | 85mm Plastic Tire   | 6          | Mobility             | -       | -              |

---

## 2. System Architecture

### 2.1 Three-Board Configuration

**Critical Preprocessor Defines for Board Behavior:**

- `FRONT_CONTROLLER` - Front ESP32 with motor execution logic
- `REAR_CONTROLLER` - Rear ESP32 with decision-making and sensor fusion
- `CAMERA_MODULE` - ESP32-CAM for vision processing

### 2.2 Communication Pattern

```
UART Cross-Connections (Test Version 2):
┌─────────────┐              ┌─────────────┐              ┌─────────────┐
│ REAR ESP32  │              │FRONT ESP32  │              │ ESP32-CAM   │
│   (Master)  │              │   (Motor)   │              │   (Vision)  │
├─────────────┤              ├─────────────┤              ├─────────────┤
│ Serial2     │◄────────────►│ Serial2     │              │             │
│ GPIO16(TX)  │   115200     │ GPIO17(RX)  │              │             │
│ GPIO17(RX)  │     Baud     │ GPIO16(TX)  │              │             │
├─────────────┤              ├─────────────┤              ├─────────────┤
│ Serial1     │              │             │              │ Serial1     │
│ GPIO14(TX)  │              │             │◄────────────►│ GPIO15(RX)  │
│ GPIO12(RX)  │              │             │   115200     │ GPIO14(TX)  │
│             │              │             │     Baud     │             │
└─────────────┘              └─────────────┘              └─────────────┘
```

### 2.3 WiFi Network Topology

```
WiFi Access Point: "ProjectNightfall" (192.168.4.0/24)
┌─────────────┐
│ REAR ESP32  │◄─── Access Point (192.168.4.1)
│   (Master)  │
├─────────────┤
│   ┌──────┐  │
│   │ Web  │  │◄─── Dashboard Clients (192.168.4.10-50)
│   │Socket│  │    Port 8888
│   └──────┘  │
└─────────────┘
       │WiFi
       ▼
┌─────────────┐
│ ESP32-CAM   │◄─── Client (192.168.4.3)
│   (Vision)  │    WebSocket Connection
├─────────────┤
│   ┌──────┐  │
│   │ ML   │  │◄─── Image Processing
│   │Inference│ │    Object Detection
│   └──────┘  │
└─────────────┘
```

---

## 3. Power Distribution System

### 3.1 Overall Power Distribution Diagram (Updated for 6-Motor System)

```
                    14.8V BATTERY PACK (4S Li-ion)
                           ┌─────────────┐
                           │    ┌───┐    │
                           │    │+  │────┼─ Main Power Switch
                           │    └───┘    │
                           │    ┌───┐    │
                           │    │-  │────┼─ Common Ground
                           │    └───┘    │
                           └─────────────┘
                                   │
                           ┌───────▼───────┐
                           │ 10A Main Fuse │
                           └───────┬───────┘
                                   │
                    ┌──────────────┼──────────────┐
                    │              │              │
            ┌───────▼───────┐ ┌────▼─────┐ ┌────▼─────┐
            │  L298N #1     │ │L298N #2  │ │L298N #3  │
            │ Motor Driver  │ │Motor     │ │Motor     │
            │  Front Motors │ │Driver    │ │Driver    │
            │               │ │Aux Motors│ │Rear      │
            └───────────────┘ └──────────┘ │Motors    │
                                          └──────────┘
                           ┌──────────────┬──────────────┐
                           │ LM2596 Buck  │
                           │ 14.8V → 5V   │
                           │   3A Max     │
                           └───────┬──────┘
                                   │
                    ┌──────────────┼──────────────┐
                    │              │              │
            ┌───────▼───────┐ ┌───▼───┐   ┌───────▼───────┐
            │ REAR ESP32    │ │Front  │   │ ESP32-CAM     │
            │ Master Ctrl   │ │ESP32  │   │ Vision        │
            │ + 2 Motors    │ │Motor  │   │ ~300mA        │
            │ ~600mA        │ │Ctrl   │   └───────────────┘
            └───────────────┘ │~500mA │
                              └───────┘
                    ┌─────────┼─────────┐
                    │         │         │
            ┌───────▼─┐  ┌───▼───┐  ┌──▼────┐
            │Sensors │  │Buzzer │  │ Status│
            │~150mA  │  │~30mA  │  │ LEDs  │
            └────────┘  └───────┘  └───────┘
```

### 3.2 Power Specifications (Updated for 6-Motor System)

| Voltage Rail | Source         | Max Current     | Components            | Wire Gauge |
| ------------ | -------------- | --------------- | --------------------- | ---------- |
| **14.8V**    | Battery Pack   | 15A (fused)     | L298N Drivers, Motors | 14 AWG     |
| **5V**       | LM2596 Buck    | 3A              | ESP32s, Sensors, CAM  | 18 AWG     |
| **3.3V**     | ESP32 Internal | 200mA per board | GPIO Logic            | Internal   |

### 3.3 Current Consumption Analysis (Updated for 6-Motor System)

```
Power Consumption Breakdown (6-Motor System):
┌─────────────────────┬─────────┬──────────┬──────────┬──────────┐
│ Component           │ Voltage │ Idle     │ Active   │ Peak     │
├─────────────────────┼─────────┼──────────┼──────────┼──────────┤
│ Rear ESP32          │ 5V      │ 80mA     │ 250mA    │ 350mA    │
│ Front ESP32         │ 5V      │ 80mA     │ 200mA    │ 300mA    │
│ ESP32-CAM           │ 5V      │ 120mA    │ 250mA    │ 400mA    │
│ L298N #1            │ 14.8V   │ 0mA      │ 50mA     │ 100mA    │
│ L298N #2            │ 14.8V   │ 0mA      │ 50mA     │ 100mA    │
│ L298N #3            │ 14.8V   │ 0mA      │ 50mA     │ 100mA    │
│ Motors (6x)         │ 14.8V   │ 0mA      │ 1.2A     │ 2A each  │
│ Ultrasonic (2x)     │ 5V      │ 15mA     │ 15mA     │ 20mA     │
│ Gas Sensor          │ 5V      │ 133mA    │ 133mA    │ 150mA    │
│ Buzzer              │ 5V      │ 0mA      │ 30mA     │ 50mA     │
├─────────────────────┼─────────┼──────────┼──────────┼──────────┤
│ TOTAL 5V RAIL       │ 5V      │ 428mA    │ 928mA    │ 1.37A    │
│ TOTAL 14.8V RAIL    │ 14.8V   │ 150mA    │ 5.7A     │ 12.9A    │
│ TOTAL SYSTEM POWER  │ -       │ 8.2W     │ 87.8W    │ 200W     │
└─────────────────────┴─────────┴──────────┴──────────┴──────────┘
```

### 3.4 Battery Runtime Calculations

```
Battery Capacity: 2500mAh @ 14.8V = 37Wh

Operating Modes:
┌─────────────────────┬──────────┬──────────┬──────────┬──────────┐
│ Mode                │ Current  │ Power    │ Runtime  │ Notes    │
├─────────────────────┼──────────┼──────────┼──────────┼──────────┤
│ Idle (Standby)      │ 0.53A    │ 7.8W     │ 4.7 hrs  │ WiFi on  │
│ Normal Navigation   │ 3.93A    │ 58.2W    │ 38 mins  │ Motors   │
│ Climbing/Obstacles  │ 8.83A    │ 130.7W   │ 17 mins  │ Max load │
│ Emergency Stop      │ 0.53A    │ 7.8W     │ 4.7 hrs  │ Motors   │
│                     │          │          │          │ stopped  │
└─────────────────────┴──────────┴──────────┴──────────┴──────────┘

Safety Margin: 20% remaining capacity
Recommended Max Mission Time: 30 minutes active operation
```

---

## 4. Rear ESP32 (Master Controller)

### 4.1 Rear ESP32 Pinout Diagram

```
                    REAR ESP32 (MASTER CONTROLLER)
                    ESP32-WROOM-32 DevKitC
    ┌─────────────────────────────────────────────────────────────┐
    │  USB    EN    3V3   IO0  IO2  IO4  IO5  IO18  IO19 IO21    │
    │  D-   RST    3V3   GND  IO14 IO27  IO26  IO25  IO23 IO22   │
    │  D+          IO0   GND  IO12  IO13  SD2  SD3  CMD  SD1     │
    │        5V    GND   IO0   3V3  SD1  CMD  SD0  IO1   IO3     │
    │  VIN   GND   GND   GND   3V3  3V3  IO2   IO0  GND   GND    │
    └─────────────────────────────────────────────────────────────┘
    │USB│EN│3V3│IO0│IO2│IO4│IO5│IO18│IO19│IO21│      ┌─────────────┐
    │D- │RST│3V3│GND│IO14│IO27│IO26│IO25│IO23│IO22  │  WiFi ANT   │
    │D+ │   │IO0│GND│IO12│IO13│SD2 │SD3 │CMD │SD1   │             │
    │   │5V │GND│IO0│3V3 │SD1 │CMD │SD0 │IO1 │IO3   └─────────────┘
    │VIN│GND│GND│GND│3V3 │3V3 │IO2 │IO0 │GND │GND   │             │
    └─────────────────────────────────────────────────────────────┘
```

### 4.2 Rear ESP32 Pin Assignment Table (Updated for 6-Motor System)

| GPIO Pin    | Function       | Connection          | Wire Color | Purpose                     |
| ----------- | -------------- | ------------------- | ---------- | --------------------------- |
| **VIN**     | Power Input    | 5V from LM2596      | Red (5V)   | Primary Power (3.7-15V)     |
| **GND**     | Ground         | System Ground       | Black      | Common Ground               |
| **GPIO 16** | UART TX        | To Front ESP32 RX   | Yellow     | Master→Slave Communication  |
| **GPIO 17** | UART RX        | From Front ESP32 TX | White      | Slave→Master Communication  |
| **GPIO 14** | UART TX        | To ESP32-CAM RX     | Yellow     | Master→Camera Communication |
| **GPIO 12** | UART RX        | From ESP32-CAM TX   | White      | Camera→Master Communication |
| **GPIO 26** | PWM Output     | L298N #3 ENA        | Orange     | Rear Left Motor Speed       |
| **GPIO 27** | Digital Output | L298N #3 IN1        | Yellow     | Rear Left Motor Forward     |
| **GPIO 25** | Digital Output | L298N #3 IN2        | Blue       | Rear Left Motor Reverse     |
| **GPIO 23** | PWM Output     | L298N #3 ENB        | Orange     | Rear Right Motor Speed      |
| **GPIO 22** | Digital Output | L298N #3 IN3        | Yellow     | Rear Right Motor Forward    |
| **GPIO 21** | Digital Output | L298N #3 IN4        | Blue       | Rear Right Motor Reverse    |
| **GPIO 18** | Digital Output | Front US Trig       | Green      | Front Ultrasonic Trigger    |
| **GPIO 19** | Digital Input  | Front US Echo       | Purple     | Front Ultrasonic Echo       |
| **GPIO 32** | Digital Output | Rear US Trig        | Green      | Rear Ultrasonic Trigger     |
| **GPIO 33** | Digital Input  | Rear US Echo        | Purple     | Rear Ultrasonic Echo        |
| **GPIO 15** | Analog Input   | Gas Sensor Analog   | Brown      | Gas Level Reading (0-4095)  |
| **GPIO 4**  | Digital Input  | Gas Sensor Digital  | Gray       | Gas Threshold Detection     |
| **GPIO 5**  | Digital Output | Buzzer Control      | Red        | Audio Alert (5V, ~30mA)     |
| **GPIO 2**  | Digital Output | Status LED          | Blue       | System Status Indicator     |
| **GPIO 0**  | Digital Output | Error LED           | Yellow     | Error/Warning Indicator     |

### 4.3 Rear ESP32 Connections Schematic (Updated for 6-Motor System)

```
Rear ESP32 Master Controller Connections (6-Motor System):
┌─────────────────────────────────────────────────────────────────┐
│ REAR ESP32 (MASTER + 2 MOTORS)                                  │
├─────────────────────────────────────────────────────────────────┤
│ POWER:                                                          │
│   VIN  ◄─────── 5V Rail (18 AWG Red) ─────┐                   │
│   GND  ◄─────── Common Ground (18 AWG Black)                   │
├─────────────────────────────────────────────────────────────────┤
│ UART TO FRONT ESP32:                                            │
│   GPIO16(TX) ──► White ──── GPIO17(RX)  ◄─── Front ESP32      │
│   GPIO17(RX) ◄─── Yellow ──── GPIO16(TX) ───►                  │
│   GND         ◄─────── Common Ground ──────                    │
├─────────────────────────────────────────────────────────────────┤
│ UART TO ESP32-CAM:                                              │
│   GPIO14(TX) ──► Yellow ──── GPIO15(RX) ◄─── ESP32-CAM        │
│   GPIO12(RX) ◄─── White ──── GPIO14(TX) ───►                   │
│   GND         ◄─────── Common Ground ──────                    │
├─────────────────────────────────────────────────────────────────┤
│ L298N #3 (Rear Motors - Controlled by Rear ESP32):             │
│   GPIO26(ENA) ───── Orange ───── ENA ─┐                       │
│   GPIO27(IN1) ───── Yellow ───── IN1 ─┼─► Rear Left Motor     │
│   GPIO25(IN2) ───── Blue  ───── IN2 ─┘    (from rear)         │
│                                           ┌─ Rear Left Motor   │
│   GPIO23(ENB) ───── Orange ───── ENB ─┐  │   (12V DC Gear)    │
│   GPIO22(IN3) ───── Yellow ───── IN3 ─┼─► Rear Right Motor   │
│   GPIO21(IN4) ───── Blue  ───── IN4 ─┘  │   (12V DC Gear)     │
│                                           └─ Rear Right Motor  │
├─────────────────────────────────────────────────────────────────┤
│ SENSORS:                                                        │
│   GPIO18 ─────── Green ────── HC-SR04 Trig (Front)             │
│   GPIO19 ◄────── Purple ───── HC-SR04 Echo (Front)             │
│   GPIO32 ─────── Green ────── HC-SR04 Trig (Rear)              │
│   GPIO33 ◄────── Purple ───── HC-SR04 Echo (Rear)              │
│   GPIO15 ─────── Brown ────── MQ-2 Analog                      │
│   GPIO4  ◄────── Gray ─────── MQ-2 Digital                     │
├─────────────────────────────────────────────────────────────────┤
│ ALERTS:                                                         │
│   GPIO5  ─────── Red ─────── Active Buzzer (+)                 │
│   GPIO2  ─────── Blue ────── Status LED (220Ω)                 │
│   GPIO0  ─────── Yellow ─── Error LED (220Ω)                   │
└─────────────────────────────────────────────────────────────────┘
```

### 4.4 Rear ESP32 Motor Control Logic

```
Rear ESP32 Motor Control (L298N #3):
┌─────────────────────────────────────────────────────────────────┐
│ REAR MOTOR CONTROL ASSIGNMENTS                                  │
├─────────────────────────────────────────────────────────────────┤
│ Motor 5 (Rear Left):                                           │
│   GPIO26 (PWM) - Speed Control (0-255)                        │
│   GPIO27 (Digital) - Forward Direction                        │
│   GPIO25 (Digital) - Reverse Direction                        │
├─────────────────────────────────────────────────────────────────┤
│ Motor 6 (Rear Right):                                          │
│   GPIO23 (PWM) - Speed Control (0-255)                        │
│   GPIO22 (Digital) - Forward Direction                        │
│   GPIO21 (Digital) - Reverse Direction                        │
├─────────────────────────────────────────────────────────────────┤
│ Safety Features:                                               │
│   - Independent motor control for each rear wheel             │
│   - Differential drive support for rear steering              │
│   - Emergency stop capability                                 │
│   - Current monitoring and overload protection                │
└─────────────────────────────────────────────────────────────────┘
```

### 4.4 Critical GPIO Safety Notes

**⚠️ ESP32 VOLTAGE SAFETY - CRITICAL:**

- GPIO pins are **3.3V ONLY** - NOT 5V tolerant
- **NEVER** connect 5V directly to GPIO pins
- Use level shifters for 5V signals if needed
- ESP32 accepts 5V on VIN/USB pins (built-in regulator)
- Always verify voltage levels before connection

---

## 5. Front ESP32 (Motor Controller)

### 5.1 Front ESP32 Pinout Diagram

```
                   FRONT ESP32 (MOTOR CONTROLLER)
                   ESP32-WROOM-32 DevKitC
    ┌─────────────────────────────────────────────────────────────┐
    │  USB    EN    3V3   IO0  IO2  IO4  IO5  IO18  IO19 IO21    │
    │  D-   RST    3V3   GND  IO14 IO27  IO26  IO25  IO23 IO22   │
    │  D+          IO0   GND  IO12  IO13  SD2  SD3  CMD  SD1     │
    │        5V    GND   IO0   3V3  SD1  CMD  SD0  IO1   IO3     │
    │  VIN   GND   GND   GND   3V3  3V3  IO2   IO0  GND   GND    │
    └─────────────────────────────────────────────────────────────┘
    │USB│EN│3V3│IO0│IO2│IO4│IO5│IO18│IO19│IO21│      ┌─────────────┐
    │D- │RST│3V3│GND│IO14│IO27│IO26│IO25│IO23│IO22  │  WiFi ANT   │
    │D+ │   │IO0│GND│IO12│IO13│SD2 │SD3 │CMD │SD1   │             │
    │   │5V │GND│IO0│3V3 │SD1 │CMD │SD0 │IO1 │IO3   └─────────────┘
    │VIN│GND│GND│GND│3V3 │3V3 │IO2 │IO0 │GND │GND   │             │
    └─────────────────────────────────────────────────────────────┘
```

### 5.2 Front ESP32 Pin Assignment Table (Updated for 4-Motor System)

| GPIO Pin    | Function       | Connection         | Wire Color | Purpose                   |
| ----------- | -------------- | ------------------ | ---------- | ------------------------- |
| **VIN**     | Power Input    | 5V from LM2596     | Red (5V)   | Primary Power (3.7-15V)   |
| **GND**     | Ground         | System Ground      | Black      | Common Ground             |
| **GPIO 13** | PWM Output     | L298N #1 ENA       | Orange     | Front Left Motor Speed    |
| **GPIO 23** | Digital Output | L298N #1 IN1       | Yellow     | Front Left Motor Forward  |
| **GPIO 22** | Digital Output | L298N #1 IN2       | Blue       | Front Left Motor Reverse  |
| **GPIO 25** | PWM Output     | L298N #1 ENB       | Orange     | Front Right Motor Speed   |
| **GPIO 26** | Digital Output | L298N #1 IN3       | Yellow     | Front Right Motor Forward |
| **GPIO 27** | Digital Output | L298N #1 IN4       | Blue       | Front Right Motor Reverse |
| **GPIO 14** | PWM Output     | L298N #2 ENA       | Orange     | Auxiliary Left Motor      |
| **GPIO 32** | Digital Output | L298N #2 IN1       | Yellow     | Auxiliary Left Forward    |
| **GPIO 33** | Digital Output | L298N #2 IN2       | Blue       | Auxiliary Left Reverse    |
| **GPIO 15** | PWM Output     | L298N #2 ENB       | Orange     | Auxiliary Right Motor     |
| **GPIO 19** | Digital Output | L298N #2 IN3       | Yellow     | Auxiliary Right Forward   |
| **GPIO 21** | Digital Output | L298N #2 IN4       | Blue       | Auxiliary Right Reverse   |
| **GPIO 16** | UART RX        | From Rear ESP32 TX | White      | Master→Slave Commands     |
| **GPIO 17** | UART TX        | To Rear ESP32 RX   | Yellow     | Slave→Master Status       |

### 5.3 Front ESP32 Motor Control Schematic (Updated for 4-Motor System)

```
Front ESP32 Motor Controller Connections (4-Motor System):
┌─────────────────────────────────────────────────────────────────┐
│ FRONT ESP32 (MOTOR CONTROLLER - 4 MOTORS)                      │
├─────────────────────────────────────────────────────────────────┤
│ POWER:                                                          │
│   VIN  ◄─────── 5V Rail (18 AWG Red) ─────┐                   │
│   GND  ◄─────── Common Ground (18 AWG Black)                   │
├─────────────────────────────────────────────────────────────────┤
│ UART TO REAR ESP32:                                             │
│   GPIO16(RX) ◄─────── White ──────── GPIO16(TX) ──── Rear ESP32│
│   GPIO17(TX) ──────── Yellow ──────── GPIO17(RX) ◄─────        │
│   GND         ◄─────── Common Ground ──────                    │
├─────────────────────────────────────────────────────────────────┤
│ L298N #1 (Front Main Motors):                                  │
│   GPIO13(ENA) ───── Orange ───── ENA ─┐                       │
│   GPIO23(IN1) ───── Yellow ───── IN1 ─┼─► Front Left Motor    │
│   GPIO22(IN2) ───── Blue  ───── IN2 ─┘                       │
│                                           ┌─ Front Left Motor  │
│   GPIO25(ENB) ───── Orange ───── ENB ─┐  │   (12V DC Gear)    │
│   GPIO26(IN3) ───── Yellow ───── IN3 ─┼─► Front Right Motor   │
│   GPIO27(IN4) ───── Blue  ───── IN4 ─┘  │   (12V DC Gear)     │
│                                           └─ Front Right Motor │
├─────────────────────────────────────────────────────────────────┤
│ L298N #2 (Auxiliary Motors):                                   │
│   GPIO14(ENA) ───── Orange ───── ENA ─┐                       │
│   GPIO32(IN1) ───── Yellow ───── IN1 ─┼─► Auxiliary Left      │
│   GPIO33(IN2) ───── Blue  ───── IN2 ─┘                       │
│                                           ┌─ Auxiliary Left    │
│   GPIO15(ENB) ───── Orange ───── ENB ─┐  │   (12V DC Gear)    │
│   GPIO19(IN3) ───── Yellow ───── IN3 ─┼─► Auxiliary Right    │
│   GPIO21(IN4) ───── Blue  ───── IN4 ─┘  │   (12V DC Gear)     │
│                                           └─ Auxiliary Right   │
├─────────────────────────────────────────────────────────────────┤
│ L298N POWER:                                                    │
│   VIN  ◄─────── 14.8V Battery (16 AWG Red)                     │
│   GND  ◄─────── Common Ground (16 AWG Black)                   │
│   5V   ◄─────── 5V Rail (18 AWG Red) ─ Logic Supply            │
└─────────────────────────────────────────────────────────────────┘
```

### 5.4 Front ESP32 Motor Driver Control Logic

```
Front ESP32 Motor Control (L298N #1 and #2):
┌─────────────────────────────────────────────────────────────────┐
│ FRONT MOTOR CONTROL ASSIGNMENTS                                 │
├─────────────────────────────────────────────────────────────────┤
│ L298N #1 (Front Main Motors):                                  │
│   Motor 1 (Front Left):                                        │
│     GPIO13 (PWM) - Speed Control (0-255)                      │
│     GPIO23 (Digital) - Forward Direction                      │
│     GPIO22 (Digital) - Reverse Direction                      │
│   Motor 2 (Front Right):                                       │
│     GPIO25 (PWM) - Speed Control (0-255)                      │
│     GPIO26 (Digital) - Forward Direction                      │
│     GPIO27 (Digital) - Reverse Direction                      │
├─────────────────────────────────────────────────────────────────┤
│ L298N #2 (Auxiliary Motors):                                   │
│   Motor 3 (Auxiliary Left):                                    │
│     GPIO14 (PWM) - Speed Control (0-255)                      │
│     GPIO32 (Digital) - Forward Direction                      │
│     GPIO33 (Digital) - Reverse Direction                      │
│   Motor 4 (Auxiliary Right):                                   │
│     GPIO15 (PWM) - Speed Control (0-255)                      │
│     GPIO19 (Digital) - Forward Direction                      │
│     GPIO21 (Digital) - Reverse Direction                      │
├─────────────────────────────────────────────────────────────────┤
│ 6-Wheel Configuration:                                         │
│   - Motor 1: Front Left Wheel                                 │
│   - Motor 2: Front Right Wheel                                │
│   - Motor 3: Middle Left Wheel                                │
│   - Motor 4: Middle Right Wheel                               │
│   - Motor 5: Rear Left Wheel (controlled by Rear ESP32)       │
│   - Motor 6: Rear Right Wheel (controlled by Rear ESP32)      │
├─────────────────────────────────────────────────────────────────┤
│ Safety Features:                                               │
│   - Independent motor control for each wheel                  │
│   - Differential drive support                                │
│   - Emergency stop capability                                 │
│   - Current monitoring and overload protection                │
└─────────────────────────────────────────────────────────────────┘
```

---

## 6. ESP32-CAM (Vision Module)

### 6.1 ESP32-CAM Pinout Diagram

```
                      ESP32-CAM (VISION MODULE)
                      AI-Thinker ESP32-CAM
    ┌─────────────────────────────────────────────────────────────┐
    │  RST   IO0   IO1   IO2   IO3   IO4   IO5   IO6   IO7       │
    │  GND   GND   IO9   IO10  IO11  IO12  IO13  GND   3V3       │
    │  IO16  IO15  IO13  IO12  IO14  IO2   GND   IO4   GND       │
    │  5V    GND   GND   GND   GND   GND   GND   GND   GND       │
    └─────────────────────────────────────────────────────────────┘
    │RST│IO0│IO1│IO2│IO3│IO4│IO5│IO6│IO7│    ┌─────────────────┐
    │GND│GND│IO9│IO10│IO11│IO12│IO13│GND│3V3 │  Camera OV2640  │
    │IO16│IO15│IO13│IO12│IO14│IO2 │GND│IO4 │    │               │
    │5V │GND│GND│GND│GND│GND│GND│GND │    │  ┌─────────────┐  │
    └─────────────────────────────────────┘  │   WiFi ANT   │  │
                                           └─────────────┘  │
                                           │               │
                                           └─────────────────┘
```

### 6.2 ESP32-CAM Pin Assignment Table

| GPIO Pin    | Function       | Connection         | Wire Color | Purpose                  |
| ----------- | -------------- | ------------------ | ---------- | ------------------------ |
| **5V**      | Power Input    | 5V from LM2596     | Red (5V)   | Primary Power (3.7-15V)  |
| **GND**     | Ground         | System Ground      | Black      | Common Ground            |
| **GPIO 14** | UART TX        | To Rear ESP32 RX   | Yellow     | Camera→Master Data       |
| **GPIO 15** | UART RX        | From Rear ESP32 TX | White      | Master→Camera Commands   |
| **GPIO 4**  | Digital Output | Flash LED          | Red        | Camera Flash Control     |
| **GPIO 33** | Digital Output | Status LED         | Green      | System Status Indicator  |
| **GPIO 0**  | Boot/Program   | GND during upload  | -          | Programming Mode Control |

### 6.3 ESP32-CAM Connections Schematic

```
ESP32-CAM Vision Module Connections:
┌─────────────────────────────────────────────────────────────────┐
│ ESP32-CAM (VISION MODULE)                                       │
├─────────────────────────────────────────────────────────────────┤
│ POWER:                                                          │
│   5V   ◄─────── 5V Rail (18 AWG Red) ─────┐                   │
│   GND  ◄─────── Common Ground (18 AWG Black)                   │
├─────────────────────────────────────────────────────────────────┤
│ UART TO REAR ESP32:                                             │
│   GPIO14(TX) ────── Yellow ──────── GPIO12(RX) ◄─── Rear ESP32 │
│   GPIO15(RX) ◄─────── White ──────── GPIO14(TX) ──────         │
│   GND         ◄─────── Common Ground ──────                    │
├─────────────────────────────────────────────────────────────────┤
│ INDICATORS:                                                     │
│   GPIO4  ──────── Red ──────── Flash LED (+)                   │
│            ◄─────── 220Ω ──────── 3.3V                         │
│   GPIO33 ──────── Green ──────── Status LED (+)                │
│            ◄─────── 220Ω ──────── 3.3V                         │
├─────────────────────────────────────────────────────────────────┤
│ PROGRAMMING:                                                    │
│   GPIO0 ─── GND during upload (programming mode)               │
│   GPIO0 ─── FLOATING during normal operation                   │
│   RST  ──── Reset Button (optional)                            │
├─────────────────────────────────────────────────────────────────┤
│ INTERNAL CAMERA:                                                │
│   OV2640 Camera Module (built-in)                              │
│   - Resolution: 640x480 to 1600x1200                           │
│   - Frame Rate: 10-15 FPS                                      │
│   - Interface: Parallel to internal GPIO                       │
│   - Power: Internal 3.3V regulation                           │
└─────────────────────────────────────────────────────────────────┘
```

### 6.4 Camera Operation Requirements

**⚠️ ESP32-CAM PROGRAMMING SEQUENCE - CRITICAL:**

1. **During Upload (Programming Mode):**

   - Connect GPIO0 to GND
   - Press reset button
   - Upload firmware via Serial
   - Monitor upload progress

2. **During Normal Operation:**

   - Disconnect GPIO0 from GND (leave floating)
   - Press reset button
   - Camera should start streaming

3. **WiFi Antenna Requirements:**
   - Built-in ceramic antenna (limited range)
   - External 2.4GHz antenna recommended for >25m range
   - Position antenna away from metal structures

---

## 7. Motor Driver Wiring

### 7.1 L298N Dual H-Bridge Driver

```
                     L298N DUAL H-BRIDGE MOTOR DRIVER
    ┌─────────────────────────────────────────────────────────────┐
    │                        L298N MODULE                         │
    ├─────────────────────────────────────────────────────────────┤
    │ OUT1 │ OUT2 │ OUT3 │ OUT4 │ +12V │ GND │ ENA │ ENB │ IN1-4 │
    └─────────────────────────────────────────────────────────────┘
    │Motor1│Motor1│Motor2│Motor2│ Power│ GND │PWM A│PWM B│Control│
    │   +  │   -  │   +  │   -  │Input │     │     │     │Logic  │
    └─────────────────────────────────────────────────────────────┘
```

### 7.2 Motor Driver Pin Functions

| Pin           | Function          | Connection     | Description               |
| ------------- | ----------------- | -------------- | ------------------------- |
| **OUT1/OUT2** | Motor 1 Output    | Motor 1 Wires  | Motor 1 direction control |
| **OUT3/OUT4** | Motor 2 Output    | Motor 2 Wires  | Motor 2 direction control |
| **+12V**      | Motor Power       | 14.8V Battery  | Motor driver power supply |
| **GND**       | Ground            | Common Ground  | System ground reference   |
| **5V**        | Logic Power       | 5V Rail        | Logic circuit power       |
| **ENA**       | Enable PWM A      | GPIO (PWM)     | Motor 1 speed control     |
| **ENB**       | Enable PWM B      | GPIO (PWM)     | Motor 2 speed control     |
| **IN1-IN4**   | Direction Control | GPIO (Digital) | Motor direction logic     |

### 7.3 L298N #1 Wiring (Front Main Motors)

```
L298N #1 - Front Main Motor Driver:
┌─────────────────────────────────────────────────────────────────┐
│ L298N #1 (FRONT MAIN MOTORS)                                    │
├─────────────────────────────────────────────────────────────────┤
│ POWER CONNECTIONS:                                              │
│   +12V ◄──────── 14.8V Battery (16 AWG Red)                    │
│   GND  ◄──────── Common Ground (16 AWG Black)                  │
│   5V   ◄──────── 5V Rail (18 AWG Red)                          │
├─────────────────────────────────────────────────────────────────┤
│ MOTOR OUTPUTS:                                                  │
│   OUT1 ───────── Motor 1 (Front Left) (+)                      │
│   OUT2 ───────── Motor 1 (Front Left) (-)                      │
│                                           ┌─ Motor 1: Front    │
│   OUT3 ───────── Motor 2 (Front Right) (+) │   Left Wheel      │
│   OUT4 ───────── Motor 2 (Front Right) (-) │   (12V 500 RPM)   │
│                                           └─                   │
├─────────────────────────────────────────────────────────────────┤
│ CONTROL SIGNALS (FROM FRONT ESP32):                             │
│   ENA  ◄────────── GPIO13 (PWM) ──────── Orange                │
│   ENB  ◄────────── GPIO25 (PWM) ──────── Orange                │
│   IN1  ◄────────── GPIO23 (Digital) ──── Yellow               │
│   IN2  ◄────────── GPIO22 (Digital) ──── Blue                 │
│   IN3  ◄────────── GPIO26 (Digital) ──── Yellow               │
│   IN4  ◄────────── GPIO27 (Digital) ──── Blue                 │
└─────────────────────────────────────────────────────────────────┘
```

### 7.4 L298N #2 Wiring (Auxiliary Motors)

```
L298N #2 - Auxiliary Motor Driver:
┌─────────────────────────────────────────────────────────────────┐
│ L298N #2 (AUXILIARY MOTORS)                                     │
├─────────────────────────────────────────────────────────────────┤
│ POWER CONNECTIONS:                                              │
│   +12V ◄──────── 14.8V Battery (16 AWG Red)                    │
│   GND  ◄──────── Common Ground (16 AWG Black)                  │
│   5V   ◄──────── 5V Rail (18 AWG Red)                          │
├─────────────────────────────────────────────────────────────────┤
│ MOTOR OUTPUTS:                                                  │
│   OUT1 ───────── Motor 3 (Auxiliary Left) (+)                  │
│   OUT2 ───────── Motor 3 (Auxiliary Left) (-)                  │
│                                           ┌─ Motor 3: Middle   │
│   OUT3 ───────── Motor 4 (Auxiliary Right) (+)│   Left Wheel    │
│   OUT4 ───────── Motor 4 (Auxiliary Right) (-)│   (12V 500 RPM) │
│                                           └─                   │
├─────────────────────────────────────────────────────────────────┤
│ CONTROL SIGNALS (FROM FRONT ESP32):                             │
│   ENA  ◄────────── GPIO14 (PWM) ──────── Orange                │
│   ENB  ◄────────── GPIO15 (PWM) ──────── Orange                │
│   IN1  ◄────────── GPIO32 (Digital) ──── Yellow               │
│   IN2  ◄────────── GPIO33 (Digital) ──── Blue                 │
│   IN3  ◄────────── GPIO19 (Digital) ──── Yellow               │
│   IN4  ◄────────── GPIO21 (Digital) ──── Blue                 │
└─────────────────────────────────────────────────────────────────┘
```

### 7.5 L298N #3 Wiring (Rear Motors)

```
L298N #3 - Rear Motor Driver:
┌─────────────────────────────────────────────────────────────────┐
│ L298N #3 (REAR MOTORS - CONTROLLED BY REAR ESP32)              │
├─────────────────────────────────────────────────────────────────┤
│ POWER CONNECTIONS:                                              │
│   +12V ◄──────── 14.8V Battery (16 AWG Red)                    │
│   GND  ◄──────── Common Ground (16 AWG Black)                  │
│   5V   ◄──────── 5V Rail (18 AWG Red)                          │
├─────────────────────────────────────────────────────────────────┤
│ MOTOR OUTPUTS:                                                  │
│   OUT1 ───────── Motor 5 (Rear Left) (+)                       │
│   OUT2 ───────── Motor 5 (Rear Left) (-)                       │
│                                           ┌─ Motor 5: Rear     │
│   OUT3 ───────── Motor 6 (Rear Right) (+)│   Left Wheel       │
│   OUT4 ───────── Motor 6 (Rear Right) (-)│   (12V 500 RPM)    │
│                                           └─                   │
├─────────────────────────────────────────────────────────────────┤
│ CONTROL SIGNALS (FROM REAR ESP32):                              │
│   ENA  ◄────────── GPIO26 (PWM) ──────── Orange                │
│   ENB  ◄────────── GPIO23 (PWM) ──────── Orange                │
│   IN1  ◄────────── GPIO27 (Digital) ──── Yellow               │
│   IN2  ◄────────── GPIO25 (Digital) ──── Blue                 │
│   IN3  ◄────────── GPIO22 (Digital) ──── Yellow               │
│   IN4  ◄────────── GPIO21 (Digital) ──── Blue                 │
└─────────────────────────────────────────────────────────────────┘
```

### 7.5 Motor Connection Details

```
Motor Wiring Specifications:
┌─────────────────────────────────────────────────────────────────┐
│ DC GEAR MOTOR (12V 500 RPM) - Qty 4                           │
├─────────────────────────────────────────────────────────────────┤
│ Physical Connection:                                            │
│   - 2-pin JST-XH connector or screw terminals                  │
│   - 18-20 AWG wire (motor power)                               │
│   - Color coding: Red (+), Black (-)                           │
│                                                                 │
│ Electrical Specifications:                                      │
│   - Voltage: 12V nominal                                       │
│   - Current: 1.5A typical, 2A peak                             │
│   - Power: 18W nominal                                         │
│   - Speed: 500 RPM @ 12V                                       │
│   - Torque: ~3kg-cm                                            │
│                                                                 │
│ Mounting:                                                      │
│   - M3 mounting screws                                         │
│   - Rubber grommets for vibration damping                      │
│   - Secure wire routing to prevent strain                      │
└─────────────────────────────────────────────────────────────────┘
```

### 7.6 Motor Driver Protection

```
Protection Features Built into L298N:
┌─────────────────────────────────────────────────────────────────┐
│ OVERCURRENT PROTECTION:                                         │
│   - 2A per channel continuous                                  │
│   - 4A peak per channel (1 second)                             │
│   - Built-in thermal shutdown at 150°C                         │
│                                                                 │
│ RECOMMENDED EXTERNAL PROTECTION:                                │
│   - 3A fuse per L298N module (optional)                        │
│   - Flyback diodes for inductive loads (built-in)              │
│   - Proper heat sinking for continuous operation               │
│                                                                 │
│ THERMAL MANAGEMENT:                                             │
│   - L298N operates at 85°C max ambient                         │
│   - Heat sink recommended for >70°C ambient                    │
│   - Fan cooling for high duty cycle operation                  │
│   - Thermal paste for heat transfer                            │
└─────────────────────────────────────────────────────────────────┘
```

---

## 8. Sensor Connections

### 8.1 Ultrasonic Sensors (HC-SR04)

```
HC-SR04 Ultrasonic Distance Sensor:
┌─────────────────────────────────────────────────────────────────┐
│ HC-SR04 ULTRASONIC SENSOR                                       │
├─────────────────────────────────────────────────────────────────┤
│ Physical Layout:                                                │
│    [VCC] [Trig] [Echo] [GND]                                    │
│     │      │      │      │                                      │
│     │      │      │      └─ System Ground                      │
│     │      │      └─ Echo Response (Digital)                   │
│     │      └─ Trigger Input (Digital)                          │
│     └─ 5V Power Supply                                          │
│                                                                 │
│ Connection Details:                                             │
│   - VCC: 5V (15mA typical)                                     │
│   - GND: System ground                                         │
│   - Trig: 10µs pulse triggers measurement                      │
│   - Echo: High pulse (38µs per cm)                             │
└─────────────────────────────────────────────────────────────────┘
```

#### 8.1.1 Front Ultrasonic Sensor (GPIO 26/27)

```
Front Ultrasonic Sensor Connection:
┌─────────────────────────────────────────────────────────────────┐
│ FRONT HC-SR04 (Rear ESP32 GPIO 26/27)                         │
├─────────────────────────────────────────────────────────────────┤
│ CONNECTIONS:                                                    │
│   VCC  ◄────────── 5V Rail (22 AWG Red)                        │
│   GND  ◄────────── Common Ground (22 AWG Black)                │
│   Trig ◄────────── GPIO26 (22 AWG Green)                       │
│   Echo ──────────► GPIO27 (22 AWG Purple)                      │
├─────────────────────────────────────────────────────────────────┤
│ MOUNTING:                                                       │
│   - Position: Front of robot, 10cm above ground               │
│   - Orientation: Facing forward, clear line of sight           │
│   - Protection: Heat shrink tubing for weather sealing         │
│   - Mounting: M3 screws with rubber grommets                   │
├─────────────────────────────────────────────────────────────────┤
│ OPERATION:                                                      │
│   - Trigger: 10µs high pulse on GPIO26                        │
│   - Echo: High pulse width proportional to distance            │
│   - Range: 2-400cm, ±3mm accuracy                              │
│   - Frequency: Max 10Hz measurement rate                       │
└─────────────────────────────────────────────────────────────────┘
```

#### 8.1.2 Rear Ultrasonic Sensor (GPIO 18/19)

```
Rear Ultrasonic Sensor Connection:
┌─────────────────────────────────────────────────────────────────┐
│ REAR HC-SR04 (Rear ESP32 GPIO 18/19)                          │
├─────────────────────────────────────────────────────────────────┤
│ CONNECTIONS:                                                    │
│   VCC  ◄────────── 5V Rail (22 AWG Red)                        │
│   GND  ◄────────── Common Ground (22 AWG Black)                │
│   Trig ◄────────── GPIO18 (22 AWG Green)                       │
│   Echo ──────────► GPIO19 (22 AWG Purple)                      │
├─────────────────────────────────────────────────────────────────┤
│ MOUNTING:                                                       │
│   - Position: Rear of robot, 10cm above ground                │
│   - Orientation: Facing backward, clear line of sight          │
│   - Protection: Heat shrink tubing for weather sealing         │
│   - Mounting: M3 screws with rubber grommets                   │
├─────────────────────────────────────────────────────────────────┤
│ OPERATION:                                                      │
│   - Trigger: 10µs high pulse on GPIO18                        │
│   - Echo: High pulse width proportional to distance            │
│   - Range: 2-400cm, ±3mm accuracy                              │
│   - Frequency: Max 10Hz measurement rate                       │
└─────────────────────────────────────────────────────────────────┘
```

### 8.2 Gas Sensor (MQ-2)

```
MQ-2 Gas/Smoke Detection Sensor:
┌─────────────────────────────────────────────────────────────────┐
│ MQ-2 GAS SENSOR MODULE                                          │
├─────────────────────────────────────────────────────────────────┤
│ Physical Layout:                                                │
│    [VCC] [GND] [DO] [AO]                                        │
│     │      │     │     │                                      │
│     │      │     │     └─ Analog Output (0-5V)                │
│     │      │     └─ Digital Output (0/1)                      │
│     │      └─ System Ground                                   │
│     └─ 5V Power Supply                                         │
│                                                                 │
│ Connection Details:                                             │
│   - VCC: 5V (133mA typical)                                    │
│   - GND: System ground                                         │
│   - AO: Analog output (0-5V, 0-1023 ADC)                       │
│   - DO: Digital threshold output                               │
└─────────────────────────────────────────────────────────────────┘
```

#### 8.2.1 MQ-2 Gas Sensor Connection

```
MQ-2 Gas Sensor Connection (Rear ESP32 GPIO 32/33):
┌─────────────────────────────────────────────────────────────────┐
│ MQ-2 GAS SENSOR (Rear ESP32 GPIO 32/33)                       │
├─────────────────────────────────────────────────────────────────┤
│ CONNECTIONS:                                                    │
│   VCC  ◄────────── 5V Rail (22 AWG Red)                        │
│   GND  ◄────────── Common Ground (22 AWG Black)                │
│   AO   ──────────► GPIO32 (22 AWG Brown) - Analog Input        │
│   DO   ──────────► GPIO33 (22 AWG Gray) - Digital Input        │
├─────────────────────────────────────────────────────────────────┤
│ MOUNTING:                                                       │
│   - Position: Ventilated location away from motor heat         │
│   - Height: 5-10cm above ground for gas detection              │
│   - Protection: Ventilated enclosure, no direct water exposure │
│   - Access: Maintainable for calibration                       │
├─────────────────────────────────────────────────────────────────┤
│ CALIBRATION REQUIREMENTS:                                       │
│   - Pre-heat: 24-48 hours before reliable readings             │
│   - Baseline: Record clean air reading                         │
│   - Test: Exposure to known gas source for verification        │
│   - Threshold: Digital output at ~400 ADC analog value          │
├─────────────────────────────────────────────────────────────────┤
│ DETECTION CAPABILITIES:                                         │
│   - LPG, Propane, Butane                                       │
│   - Methane, Natural Gas                                       │
│   - Smoke, Hydrogen                                            │
│   - Alcohol, Benzene                                           │
│   - Sensitivity: Adjustable via potentiometer                 │
└─────────────────────────────────────────────────────────────────┘
```

### 8.3 Audio Alert System

```
Active Buzzer Audio Alert:
┌─────────────────────────────────────────────────────────────────┐
│ ACTIVE BUZZER (5V 70dB)                                        │
├─────────────────────────────────────────────────────────────────┤
│ Physical Specifications:                                        │
│   - Type: Active Buzzer (internal oscillator)                  │
│   - Voltage: 5V nominal (3.3-5V range)                        │
│   - Current: 30mA typical                                      │
│   - Sound Level: 70dB @ 10cm                                   │
│   - Frequency: ~4kHz                                           │
└─────────────────────────────────────────────────────────────────┘
```

#### 8.3.1 Buzzer Connection

```
Buzzer Connection (Rear ESP32 GPIO 13):
┌─────────────────────────────────────────────────────────────────┐
│ ACTIVE BUZZER (Rear ESP32 GPIO 13)                            │
├─────────────────────────────────────────────────────────────────┤
│ CONNECTIONS:                                                    │
│   Positive ◄──────── GPIO13 (22 AWG Red)                       │
│   Negative ◄──────── Common Ground (22 AWG Black)              │
├─────────────────────────────────────────────────────────────────┤
│ OPERATION MODES:                                                │
│   - Emergency: Continuous tone (0.5s on/off)                  │
│   - Warning: Intermittent tone (2s on/0.5s off)               │
│   - Alert: Short pulse (0.1s on/0.9s off)                     │
│   - Success: Ascending tone sequence                           │
├─────────────────────────────────────────────────────────────────┤
│ MOUNTING:                                                       │
│   - Position: Audible from all robot positions                │
│   - Protection: Waterproof housing or covers                   │
│   - Vibration: Secure mounting to prevent loose connections    │
│   - Access: Maintainable for replacement                       │
└─────────────────────────────────────────────────────────────────┘
```

---

## 9. UART Communication Network

### 9.1 UART Communication Architecture

```
UART Communication Network - Test Version 2:
┌─────────────────────────────────────────────────────────────────┐
│ UART COMMUNICATION TOPOLOGY                                     │
├─────────────────────────────────────────────────────────────────┤
│ ┌─────────────┐     Serial2     ┌─────────────┐               │
│ │ REAR ESP32  │◄────────────────│FRONT ESP32  │               │
│ │   (Master)  │   115200 baud   │   (Motor)   │               │
│ │ 192.168.4.1 │                 │ 192.168.4.2 │               │
│ └─────────────┘                 └─────────────┘               │
│        │                                                        │
│        │ Serial1                                                 │
│        │ 115200 baud                                             │
│        ▼                                                        │
│ ┌─────────────┐                                                 │
│ │ ESP32-CAM   │                                                 │
│ │   (Vision)  │                                                 │
│ │ 192.168.4.3 │                                                 │
│ └─────────────┘                                                 │
├─────────────────────────────────────────────────────────────────┤
│ MESSAGE TYPES:                                                  │
│   - Motor Commands: JSON {"cmd":"forward","speed":180}         │
│   - Status Updates: JSON {"status":"ok","motors":"running"}    │
│   - Heartbeats: JSON {"hb":true,"uptime":1234}                 │
│   - Telemetry: JSON {"distance":25.4,"gas":120,"temp":23.1}    │
│   - Emergency: JSON {"emergency":true,"reason":"obstacle"}     │
└─────────────────────────────────────────────────────────────────┘
```

### 9.2 UART Wiring Details

```
UART Wiring Specifications:
┌─────────────────────────────────────────────────────────────────┐
│ REAR ↔ FRONT ESP32 (Serial2)                                   │
├─────────────────────────────────────────────────────────────────┤
│ Physical Connections:                                           │
│   Rear GPIO16(TX) ──── White ──── GPIO17(RX) ◄─── Front ESP32 │
│   Rear GPIO17(RX) ◄─── Yellow ──── GPIO16(TX) ──── Front      │
│   Rear GND         ◄─────── Common Ground ──────               │
│                                                                 │
│ Signal Specifications:                                          │
│   - Baud Rate: 115200 (8N1: 8 data, No parity, 1 stop)        │
│   - Voltage: 3.3V logic level                                  │
│   - Wire: 22-24 AWG twisted pair (recommended)                 │
│   - Length: <1m for reliable operation                         │
│   - Termination: None required for short runs                  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ REAR ↔ ESP32-CAM (Serial1)                                     │
├─────────────────────────────────────────────────────────────────┤
│ Physical Connections:                                           │
│   Rear GPIO14(TX) ──── Yellow ──── GPIO15(RX) ◄─── ESP32-CAM  │
│   Rear GPIO12(RX) ◄───── White ────── GPIO14(TX) ──────       │
│   Rear GND         ◄─────── Common Ground ──────               │
│                                                                 │
│ Signal Specifications:                                          │
│   - Baud Rate: 115200 (8N1: 8 data, No parity, 1 stop)        │
│   - Voltage: 3.3V logic level                                  │
│   - Wire: 22-24 AWG twisted pair (recommended)                 │
│   - Length: <1m for reliable operation                         │
│   - Termination: None required for short runs                  │
└─────────────────────────────────────────────────────────────────┘
```

### 9.3 Communication Protocol

```
JSON Message Protocol Examples:
┌─────────────────────────────────────────────────────────────────┐
│ MOTOR COMMAND (Rear → Front):                                   │
│ {                                                               │
│   "cmd": "forward",                                            │
│   "speed": 180,                                                │
│   "duration": 5000                                             │
│ }                                                               │
├─────────────────────────────────────────────────────────────────┤
│ STATUS RESPONSE (Front → Rear):                                 │
│ {                                                               │
│   "status": "executing",                                       │
│   "motors": "forward",                                         │
│   "speed": 180,                                                │
│   "current": 1.2,                                              │
│   "uptime": 1250                                               │
│ }                                                               │
├─────────────────────────────────────────────────────────────────┤
│ HEARTBEAT (Both Directions):                                    │
│ {                                                               │
│   "hb": true,                                                  │
│   "uptime": 1250,                                              │
│   "voltage": 14.8,                                             │
│   "temperature": 23.1                                          │
│ }                                                               │
├─────────────────────────────────────────────────────────────────┤
│ TELEMETRY (Rear ESP32):                                         │
│ {                                                               │
│   "front_distance": 45.2,                                      │
│   "rear_distance": 120.5,                                      │
│   "gas_level": 150,                                            │
│   "battery_voltage": 14.6,                                     │
│   "system_state": "autonomous"                                 │
│ }                                                               │
└─────────────────────────────────────────────────────────────────┘
```

### 9.4 Error Handling and Recovery

```
Communication Error Handling:
┌─────────────────────────────────────────────────────────────────┐
│ TIMEOUT HANDLING:                                               │
│   - Watchdog Timeout: 5 seconds                                │
│   - Heartbeat Interval: 1 second                               │
│   - Retry Attempts: 3 per message                              │
│   - Recovery Action: Emergency stop, restart communication     │
├─────────────────────────────────────────────────────────────────┤
│ ERROR CODES:                                                    │
│   - 0x01: Unknown command                                      │
│   - 0x02: Invalid parameters                                   │
│   - 0x03: Hardware error                                       │
│   - 0x04: Communication timeout                                │
│   - 0x05: System overload                                      │
│   - 0xFF: Emergency stop                                       │
├─────────────────────────────────────────────────────────────────┤
│ RECOVERY PROCEDURES:                                            │
│   1. Stop all motors immediately                               │
│   2. Send emergency alert to all nodes                         │
│   3. Attempt communication restart                             │
│   4. Enter safe mode if communication fails                    │
│   5. Manual intervention required                              │
└─────────────────────────────────────────────────────────────────┘
```

---

## 10. WiFi Network Topology

### 10.1 WiFi Access Point Configuration

```
WiFi Network: "ProjectNightfall" (192.168.4.0/24)
┌─────────────────────────────────────────────────────────────────┐
│ ACCESS POINT (Rear ESP32 - Master Controller)                 │
├─────────────────────────────────────────────────────────────────┤
│ Network Configuration:                                          │
│   SSID: ProjectNightfall                                       │
│   Password: rescue2025                                         │
│   Security: WPA2-PSK                                           │
│   Channel: 1 (2.4GHz)                                         │
│   IP Range: 192.168.4.0/24                                    │
│   Gateway: 192.168.4.1                                        │
│   DHCP Range: 192.168.4.10-192.168.4.100                      │
├─────────────────────────────────────────────────────────────────┤
│ Connected Devices:                                              │
│   - Rear ESP32: 192.168.4.1 (Gateway)                         │
│   - ESP32-CAM: 192.168.4.3 (Static)                           │
│   - Front ESP32: 192.168.4.2 (Static)                         │
│   - Dashboard Clients: 192.168.4.10-50 (DHCP)                 │
└─────────────────────────────────────────────────────────────────┘
```

### 10.2 WebSocket Server Implementation

```
WebSocket Server (Port 8888):
┌─────────────────────────────────────────────────────────────────┐
│ REAR ESP32 WEB SERVER                                           │
├─────────────────────────────────────────────────────────────────┤
│ HTTP Server:                                                    │
│   - Port: 80                                                   │
│   - Static Files: /index.html, /dashboard.js                   │
│   - API Endpoints: /api/status, /api/control                   │
│   - File System: LittleFS                                      │
├─────────────────────────────────────────────────────────────────┤
│ WebSocket Server:                                               │
│   - Port: 8888                                                 │
│   - Protocol: WebSocket (RFC 6455)                            │
│   - Message Format: JSON                                       │
│   - Max Clients: 4 simultaneous                                │
│   - Heartbeat: 30 seconds                                      │
├─────────────────────────────────────────────────────────────────┤
│ Real-time Updates:                                              │
│   - Telemetry: 500ms interval                                  │
│   - Status: On change                                          │
│   - Alerts: Immediate                                          │
│   - Video Stream: HTTP on port 81                             │
└─────────────────────────────────────────────────────────────────┘
```

### 10.3 ESP32-CAM WiFi Client

```
ESP32-CAM WiFi Client Configuration:
┌─────────────────────────────────────────────────────────────────┐
│ ESP32-CAM CLIENT CONNECTION                                     │
├─────────────────────────────────────────────────────────────────┤
│ WiFi Connection:                                                │
│   SSID: ProjectNightfall                                       │
│   Password: rescue2025                                         │
│   Static IP: 192.168.4.3                                      │
│   Connection: WPA2-PSK                                        │
│   Reconnect: Auto (30s retry)                                 │
├─────────────────────────────────────────────────────────────────┤
│ WebSocket Client:                                               │
│   Host: 192.168.4.1                                           │
│   Port: 8888                                                  │
│   Protocol: ws://                                             │
│   Message Rate: Heartbeat every 5 seconds                     │
│   Buffer Size: 512 bytes                                      │
├─────────────────────────────────────────────────────────────────┤
│ Camera Streaming:                                               │
│   HTTP Server: Port 81                                        │
│   JPEG Quality: 10 (1-63, lower=better)                      │
│   Resolution: VGA (640x480)                                   │
│   Frame Rate: 10-15 FPS                                       │
│   Format: JPEG                                                │
└─────────────────────────────────────────────────────────────────┘
```

### 10.4 Dashboard Client Access

```
Dashboard Web Interface:
┌─────────────────────────────────────────────────────────────────┐
│ WEB DASHBOARD CLIENTS                                           │
├─────────────────────────────────────────────────────────────────┤
│ Connection Information:                                         │
│   URL: http://192.168.4.1                                     │
│   Network: Connect to "ProjectNightfall"                      │
│   Password: rescue2025                                        │
│   Browser: Chrome, Firefox, Safari, Edge                      │
├─────────────────────────────────────────────────────────────────┤
│ Available Pages:                                                │
│   /           - Main Dashboard                                │
│   /stream     - Camera Live View                              │
│   /api/status - System Status JSON                            │
│   /api/control - Manual Control Interface                     │
│   /config     - Configuration Settings                        │
├─────────────────────────────────────────────────────────────────┤
│ Features:                                                       │
│   - Real-time telemetry display                               │
│   - Manual motor control                                      │
│   - Camera stream viewer                                      │
│   - System diagnostics                                        │
│   - Emergency stop button                                     │
│   - Configuration management                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 11. Electrical Specifications

### 11.1 Voltage Level Specifications

```
Voltage Level Matrix:
┌─────────────────┬──────────┬──────────┬────────────┬──────────┐
│ Component       │ Nominal  │ Min      │ Max        │ Tolerance│
├─────────────────┼──────────┼──────────┼────────────┼──────────┤
│ Battery Pack    │ 14.8V    │ 12.0V    │ 16.8V      │ ±5%      │
│ Motor Voltage   │ 12.0V    │ 11.0V    │ 14.0V      │ ±10%     │
│ Logic 5V Rail   │ 5.0V     │ 4.75V    │ 5.25V      │ ±5%      │
│ ESP32 GPIO      │ 3.3V     │ 2.8V     │ 3.6V       │ ±10%     │
│ UART Signals    │ 3.3V     │ 2.8V     │ 3.6V       │ ±10%     │
│ Sensor Inputs   │ 5.0V     │ 4.5V     │ 5.5V       │ ±10%     │
│ Analog Signals  │ 0-5V     │ 0V       │ 5.0V       │ -        │
└─────────────────┴──────────┴──────────┴────────────┴──────────┘
```

### 11.2 Current Rating Specifications

```
Current Consumption by Component:
┌─────────────────┬─────────┬─────────┬─────────┬─────────┬───────┐
│ Component       │ Standby │ Normal  │ Peak    │ Duration│ Notes │
├─────────────────┼─────────┼─────────┼─────────┼─────────┼───────┤
│ Rear ESP32      │ 80mA    │ 200mA   │ 300mA   │ 10s     │ WiFi  │
│ Front ESP32     │ 80mA    │ 200mA   │ 300mA   │ 10s     │ Motors│
│ ESP32-CAM       │ 120mA   │ 250mA   │ 400mA   │ 5s      │ Camera│
│ L298N #1        │ 0mA     │ 50mA    │ 100mA   │ 1s      │ Logic │
│ L298N #2        │ 0mA     │ 50mA    │ 100mA   │ 1s      │ Logic │
│ Motor FL        │ 0mA     │ 1.5A    │ 2.0A    │ 30s     │ 12V   │
│ Motor FR        │ 0mA     │ 1.5A    │ 2.0A    │ 30s     │ 12V   │
│ Motor RL        │ 0mA     │ 1.5A    │ 2.0A    │ 30s     │ 12V   │
│ Motor RR        │ 0mA     │ 1.5A    │ 2.0A    │ 30s     │ 12V   │
│ US Front        │ 15mA    │ 15mA    │ 20mA    │ 1s      │ Pulse │
│ US Rear         │ 15mA    │ 15mA    │ 20mA    │ 1s      │ Pulse │
│ Gas Sensor      │ 133mA   │ 133mA   │ 150mA   │ Always  │ Warmup│
│ Buzzer          │ 0mA     │ 30mA    │ 50mA    │ 1s      │ Tone  │
├─────────────────┼─────────┼─────────┼─────────┼─────────┼───────┤
│ TOTAL 5V RAIL   │ 428mA   │ 878mA   │ 1.32A   │ -       │       │
│ TOTAL 14.8V     │ 100mA   │ 6.8A    │ 8.4A    │ -       │ Motors│
└─────────────────┴─────────┴─────────┴─────────┴─────────┴───────┘
```

### 11.3 Power Dissipation Analysis

```
Power Consumption Breakdown:
┌─────────────────────────────────────────────────────────────────┐
│ POWER BUDGET ANALYSIS                                           │
├─────────────────────────────────────────────────────────────────┤
│ 5V Rail Power:                                                  │
│   Rear ESP32: 200mA × 5V = 1.0W                               │
│   Front ESP32: 200mA × 5V = 1.0W                              │
│   ESP32-CAM: 250mA × 5V = 1.25W                               │
│   Sensors: 200mA × 5V = 1.0W                                  │
│   Total 5V: 4.25W (with 85% efficiency = 5.0W from 12V)       │
├─────────────────────────────────────────────────────────────────┤
│ 14.8V Rail Power:                                               │
│   L298N Logic: 100mA × 14.8V = 1.48W                          │
│   Motors (4x): 6.0A × 14.8V = 88.8W                           │
│   Total 14.8V: 90.28W                                         │
├─────────────────────────────────────────────────────────────────┤
│ Overall System Power:                                           │
│   Input Power: 95.28W                                          │
│   Battery Current: 6.44A @ 14.8V                              │
│   Efficiency: ~85% (buck converter + motor drivers)           │
│   Estimated Runtime: ~23 minutes @ 2500mAh                    │
└─────────────────────────────────────────────────────────────────┘
```

### 11.4 Electrical Safety Ratings

```
Component Safety Ratings:
┌─────────────────┬─────────────┬─────────────┬─────────────────┐
│ Component       │ Max Voltage │ Max Current │ Safety Factor   │
├─────────────────┼─────────────┼─────────────┼─────────────────┤
│ Battery Pack    │ 16.8V       │ 10A         │ 2.0x            │
│ LM2596 Buck     │ 40V         │ 3A          │ 1.5x            │
│ L298N Driver    │ 35V         │ 2A/channel  │ 1.5x            │
│ ESP32 GPIO      │ 3.6V        │ 12mA/pin    │ 3.0x            │
│ Motor Wires     │ 24V         │ 5A          │ 2.0x            │
│ Signal Wires    │ 12V         │ 1A          │ 3.0x            │
└─────────────────┴─────────────┴─────────────┴─────────────────┘
```

---

## 12. Wire Gauge and Connectors

### 12.1 Wire Gauge Specifications

```
Wire Gauge Selection Matrix:
┌─────────────────┬─────────┬─────────┬─────────┬─────────────────┐
│ Application     │ Gauge   │ Current │ Voltage │ Color Code      │
├─────────────────┼─────────┼─────────┼─────────┼─────────────────┤
│ Battery to Buck │ 16 AWG  │ 10A     │ 15V     │ Red(+)/Black(-) │
│ Buck to ESP32s  │ 18 AWG  │ 3A      │ 5V      │ Red(5V)/Black   │
│ Motor Power     │ 18 AWG  │ 2A      │ 12V     │ Red(+)/Black(-) │
│ Motor Control   │ 20 AWG  │ 1A      │ 3.3V    │ Orange/Blue     │
│ UART Signals    │ 22 AWG  │ 0.1A    │ 3.3V    │ Yellow/White    │
│ Sensor Power    │ 22 AWG  │ 0.2A    │ 5V      │ Red/Black       │
│ Sensor Signals  │ 24 AWG  │ 0.1A    │ 3.3V    │ Brown/Green     │
│ LEDs/Indicators │ 26 AWG  │ 0.05A   │ 3.3V    │ Various         │
└─────────────────┴─────────┴─────────┴─────────┴─────────────────┘
```

### 12.2 Connector Types and Specifications

```
Connector Type Reference:
┌─────────────────────────────────────────────────────────────────┐
│ POWER CONNECTORS                                                │
├─────────────────────────────────────────────────────────────────┤
│ Battery Pack:                                                   │
│   - Type: XT60 (main power)                                    │
│   - Current: 60A maximum                                       │
│   - Voltage: 600V maximum                                      │
│   - Wire: 12-16 AWG                                            │
│                                                                 │
│ Buck Converter:                                                 │
│   - Type: Screw terminals                                      │
│   - Current: 5A maximum                                        │
│   - Wire: 16-18 AWG                                            │
│   - Strip length: 5-7mm                                        │
├─────────────────────────────────────────────────────────────────┤
│ MOTOR CONNECTORS                                                │
├─────────────────────────────────────────────────────────────────┤
│ DC Motors:                                                      │
│   - Type: JST-XH 2-pin or bullet connectors                   │
│   - Current: 3A maximum                                       │
│   - Wire: 18-20 AWG                                            │
│   - Color: Red(+), Black(-)                                    │
│                                                                 │
│ Motor Drivers:                                                  │
│   - Type: Screw terminals                                      │
│   - Current: 2A per terminal                                   │
│   - Wire: 18-20 AWG                                            │
│   - Strip length: 5-7mm                                        │
├─────────────────────────────────────────────────────────────────┤
│ SIGNAL CONNECTORS                                               │
├─────────────────────────────────────────────────────────────────┤
│ ESP32 Boards:                                                   │
│   - Type: Dupont 2.54mm headers                               │
│   - Current: 1A maximum                                       │
│   - Wire: 22-26 AWG                                            │
│   - Types: Male-to-Male, Male-to-Female                        │
│                                                                 │
│ Sensors:                                                        │
│   - Type: JST-PH 2.0mm or Dupont                             │
│   - Current: 0.5A maximum                                     │
│   - Wire: 24-26 AWG                                            │
│   - Polarization: Keyed connectors                            │
└─────────────────────────────────────────────────────────────────┘
```

### 12.3 Wire Management and Protection

```
Wire Protection and Management:
┌─────────────────────────────────────────────────────────────────┐
│ WIRE PROTECTION                                                 │
├─────────────────────────────────────────────────────────────────┤
│ Heat Shrink Tubing:                                             │
│   - Size: 2mm (22-26 AWG), 4mm (18-20 AWG), 6mm (16 AWG)      │
│   - Material: Polyolefin (2:1 shrink ratio)                   │
│   - Temperature: -55°C to +125°C                              │
│   - Application: All solder joints, wire junctions             │
├─────────────────────────────────────────────────────────────────┤
│ Cable Organization:                                             │
│   - Cable ties: 4" and 8" nylon                              │
│   - Spiral wrap: 1/4" to 1" diameter                         │
│   - Wire ducts: PVC channel for power distribution            │
│   - Cable grommets: For chassis penetrations                  │
├─────────────────────────────────────────────────────────────────┤
│ Strain Relief:                                                  │
│   - Secure wires 2-3cm from terminals                         │
│   - Use cable clamps on chassis                               │
│   - Minimum bend radius: 10x wire diameter                    │
│   - Avoid sharp edges and corners                             │
└─────────────────────────────────────────────────────────────────┘
```

### 12.4 Grounding Implementation

```
Grounding Scheme Implementation:
┌─────────────────────────────────────────────────────────────────┐
│ STAR GROUND CONFIGURATION                                       │
├─────────────────────────────────────────────────────────────────┤
│ Central Ground Point:                                           │
│   - Location: Near battery negative terminal                  │
│   - Connection: Large lug terminal                            │
│   - Wire: 16 AWG black (main ground)                          │
│   - Ring terminal: 6-8mm ID                                   │
├─────────────────────────────────────────────────────────────────┤
│ Ground Connections:                                             │
│   - Battery negative: 16 AWG                                  │
│   - Buck converter: 16 AWG                                    │
│   - ESP32 grounds: 18 AWG (star from central point)           │
│   - L298N grounds: 18 AWG                                     │
│   - Sensor grounds: 22 AWG                                    │
├─────────────────────────────────────────────────────────────────┤
│ Ground Wire Routing:                                            │
│   - Minimize length: <30cm for power grounds
```
