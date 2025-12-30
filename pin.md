```markdown
# ESP32 Project Pinout Reference - Project Nightfall Rescue Robot
**Author: CSE Student (Dhaka, BD)** | **Date: Dec 30, 2025** | **Boards: ESP32 DevKit V1 + ESP32-CAM AI-Thinker**

Complete hardware pin mapping for ESP32-based rescue robot with L298N motors, MQ-2 gas sensor, HC-SR04 ultrasonic, buck converter. **ALL 3.3V LOGIC** - use level shifters for 5V sensors if needed.

## 1. ESP32 DevKit V1 (Main Controller - Motors + Sensors)
30-pin board. **Safe GPIOs for project**: 13, 14, 18, 19, 21-23, 25-27, 32-33

| GPIO | Label | Type | Project Safe | Notes |
|------|-------|------|--------------|-------|
| 0 | - | I/O | ⚠️ Avoid | Boot strap (LOW=prog) |
| 1 | TX0 | I/O | No | UART debug |
| 2 | - | I/O | ⚠️ Avoid | Boot strap |
| 3 | RX0 | I/O | No | UART debug |
| 4 | - | I/O | Yes | ADC2_CH0 |
| 5 | - | I/O | ⚠️ Avoid | Boot strap, SPI CS |
| 12 | - | I/O | ⚠️ Avoid | Boot strap |
| **13** | - | I/O | ✅ **MOTOR/SENSOR** | Safe PWM |
| **14** | - | I/O | ✅ **MOTOR** | Safe |
| **18** | - | I/O | ✅ **MOTOR/SENSOR** | Safe, SPI CLK |
| **19** | - | I/O | ✅ **MOTOR** | Safe, SPI MISO |
| **21** | SDA | I/O | ✅ **SENSOR** | Safe I2C |
| **22** | SCL | I/O | ✅ **ULTRASONIC** | Safe I2C |
| **23** | MOSI | I/O | ✅ **MOTOR** | Safe SPI |
| **25** | DAC2 | I/O | ✅ **PWM** | Safe |
| **26** | DAC1 | I/O | ✅ **PWM** | Safe |
| **27** | - | I/O | ✅ **MOTOR** | Safe |
| **32** | - | I/O | ✅ **GAS SENSOR** | ADC1_CH4 safe |
| **33** | - | I/O | ✅ **BUZZER** | ADC1_CH5 safe |
| 34 | - | Input | Yes | ADC1_CH6, no output |
| 35 | - | Input | Yes | ADC1_CH7 |
| 36 | VP | Input | Yes | ADC1_CH0 |
| 39 | VN | Input | Yes | ADC1_CH3 |

**Power**: 5V IN, 3.3V OUT (600mA max), GND x3[page:1]

## 2. ESP32-CAM AI-Thinker (Camera + Vision)
16 exposed pins. **ONLY GPIO33 safe** for extra sensors when camera/SD active.

| GPIO | Label | Default Function | Project Safe | Notes |
|------|-------|------------------|--------------|-------|
| 0 | U0R | XCLK (camera) | ⚠️ Limited | Boot strap LOW=prog |
| 1 | U0T | Serial TX | No | UART debug |
| 2 | - | SD CLK | No | microSD |
| 3 | - | Serial RX | No | UART debug |
| 4 | - | SD CMD/Flash LED | No | Conflicts SD |
| 5 | - | Y5 (camera) | No | Camera |
| 12 | GND | SD DAT3 | No* | Free in 1-bit SD mode |
| 13 | - | SD DAT2 | No* | Free in 1-bit SD mode |
| 14 | - | SD DAT1 | No | microSD |
| 15 | - | SD DAT0 | No | Boot strap |
| 16 | PS | PSRAM CS | ❌ Avoid | Crashes post-camera |
| 32 | PP | PWDN (camera) | No | Camera |
| **33** | - | **RED LED (LOW=ON)** | ✅ **BUZZER/STATUS** | **BEST PIN** |
| 34 | Y9 | D7 (camera) | No | Input-only |
| 35 | Y8 | D6 (camera) | No | Input-only |
| 36 | Y7 | D5 (camera) | No | Input-only |
| 39 | Y6 | D4 (camera) | No | Input-only |

**SD Init**: `SD_MMC.begin("/sdcard", true)` (1-bit mode frees GPIO12/13)[page:2]

## 3. L298N Motor Driver (Dual H-Bridge)
Controls 2 DC motors per module. **ESP32 → L298N: 3.3V logic safe**.

| Pin | Function | ESP32 GPIO Rec. | Notes |
|-----|----------|-----------------|-------|
| ENA | Motor A PWM | GPIO25/26/27 | Speed control (0-255) |
| IN1 | Motor A Dir1 | GPIO13/14 | HIGH=forward |
| IN2 | Motor A Dir2 | GPIO18/19 | LOW=forward |
| IN3 | Motor B Dir1 | GPIO21/22 | HIGH=forward |
| IN4 | Motor B Dir2 | GPIO23/27 | LOW=forward |
| ENB | Motor B PWM | GPIO32/33 | Speed control |

**Power**: 5-35V motor supply (separate from ESP32), 5V logic from buck converter[web:2]

## 4. MQ-2 Gas Sensor (Smoke/LPG)
**3.3V or 5V operation**. Analog + Digital output.

| Pin | Function | ESP32 GPIO Rec. | Notes |
|-----|----------|-----------------|-------|
| VCC | Power | 3.3V/5V (buck) | 150mA max |
| GND | Ground | GND | - |
| A0 | Analog Out | GPIO32/36 | 0-3.3V ADC |
| D0 | Digital Out | GPIO33/4 | >threshold=HIGH |

**Calibration**: Read A0 in clean air = baseline (~100-300 raw)[web:1]

## 5. HC-SR04 Ultrasonic Sensor
**5V only** - use buck converter + level shifter for trig/echo.

| Pin | Function | ESP32 GPIO Rec. | Notes |
|-----|----------|-----------------|-------|
| VCC | 5V | Buck converter | - |
| GND | Ground | GND | - |
| Trig | Trigger (5V) | GPIO22/14* | 10µs HIGH pulse |
| Echo | Echo (5V) | GPIO21/13* | Level shift to 3.3V |

*Use voltage divider (1k:2k) on Echo → 3.3V[memory:12]

## 6. Buck Converter (AMS1117-3.3 or LM2596)
Steps down 5-12V to stable **3.3V/1A** for ESP32 + sensors.

| Pin | Function | Connection | Notes |
|-----|----------|------------|-------|
| VIN+ | Input + | 5-12V source | Battery/USB |
| VIN- | Input - | GND | - |
| VOUT+ | **3.3V OUT** | ESP32 3V3 + Sensor VCC | 1A max total |
| VOUT- | Output GND | Common GND | - |

**Warning**: ESP32 3V3 pin = 600mA max. Use buck for heavy loads.

## Project Nightfall Pin Recommendations
```
ESP32 DevKit V1:
- Motors: 13,14,18,19,23,27 (L298N)
- Gas MQ-2: GPIO32 (A0), GPIO33 (D0)
- Ultrasonic: GPIO21 (Echo*), GPIO22 (Trig*)

ESP32-CAM:
- Status/Buzzer: GPIO33 (LED LOW=ON)
- SD Camera: Default mappings
```

**CRITICAL**: GPIO0 LOW + RST for programming both boards. 5V/1A min for ESP32-CAM stability.[page:1][page:2][memory:12]
```

***

