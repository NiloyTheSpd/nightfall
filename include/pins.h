/**
 * @file    pins.h
 * @brief   Project Nightfall - Complete Pin Definitions
 * @author  Project Nightfall Team
 * @version 2.0.0
 * @date    December 30, 2025
 *
 * Organized pin definitions for all three ESP32 modules in Project Nightfall
 * rescue robot system. Includes detailed wiring comments and voltage requirements.
 */

#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

// ===== PREPROCESSOR DEFINITIONS FOR BOARD IDENTIFICATION =====

// Uncomment ONE of the following for each build target:
// #define REAR_CONTROLLER   // Master ESP32 DevKit V1 (Rear controller)
// #define FRONT_CONTROLLER  // Slave ESP32 DevKit V1 (Front controller)
// #define CAMERA_MODULE     // ESP32-CAM AI-Thinker (Vision module)

// ===== VERSION INFORMATION =====
#define PINS_VERSION_MAJOR 2
#define PINS_VERSION_MINOR 0
#define PINS_VERSION_PATCH 0

// ===== CRITICAL VOLTAGE SAFETY WARNINGS =====
/**
 * ⚠️  CRITICAL VOLTAGE WARNINGS:
 * 
 * 1. ESP32 GPIO pins are 3.3V LOGIC ONLY - NEVER connect 5V directly to GPIO pins
 * 2. HC-SR04 Echo pin REQUIRES 5V→3.3V voltage divider (1kΩ:2kΩ recommended)
 * 3. All 5V sensors require level shifters for GPIO connections
 * 4. ESP32-CAM accepts 5V on VIN/USB but GPIO pins are still 3.3V only
 * 5. Motor control signals from ESP32 are safe for L298N (3.3V→5V internal)
 */

// ===== REAR MAIN ESP32 (MASTER CONTROLLER) =====
#ifdef REAR_CONTROLLER
/**
 * Board: ESP32 DevKit V1 - Master/Brain Controller
 * Function: Decision-making, sensor fusion, WiFi AP, WebSocket server
 * Power: VIN (14.8V→3.3V internal), GND
 * Safe GPIOs: 13,14,18,19,21-23,25-27,32-33
 */

// ===== MOTOR CONTROL PINS (L298N Driver) =====
// Wiring: Connect L298N outputs to DC motors, inputs to ESP32 GPIO pins
// Power: Motors on 14.8V, L298N logic on 5V from buck converter
#define PIN_MOTOR_1 13  // Motor Control 1 (L298N IN1/ENA - PWM speed)
#define PIN_MOTOR_2 14  // Motor Control 2 (L298N IN2 - Direction)
#define PIN_MOTOR_3 18  // Motor Control 3 (L298N IN3 - Direction)
#define PIN_MOTOR_4 19  // Motor Control 4 (L298N IN4 - Direction)
#define PIN_MOTOR_5 23  // Motor Control 5 (L298N PWM/ENA - Speed)
#define PIN_MOTOR_6 27  // Motor Control 6 (L298N PWM/ENB - Speed)

// ===== GAS SENSOR PINS (MQ-2 Module) =====
// Wiring: VCC→3.3V, GND→GND, A0→GPIO32, D0→GPIO33
// Calibration: Requires 24-48 hour pre-heat period for reliable readings
#define PIN_GAS_ANALOG 32  // MQ-2 Analog Output (A0 - 0-3.3V ADC)
#define PIN_GAS_DIGITAL 33 // MQ-2 Digital Output (D0 - threshold detection)

// ===== ULTRASONIC SENSOR PINS (HC-SR04) =====
// Wiring: VCC→5V, GND→GND, Trig→GPIO4, Echo→GPIO36
// ⚠️ CRITICAL: Echo pin requires 5V→3.3V voltage divider!
#define PIN_US_TRIG 4   // HC-SR04 Trigger Pin (3.3V output, 10µs pulse)
#define PIN_US_ECHO 36  // HC-SR04 Echo Pin (5V input - REQUIRES voltage divider!)

// ===== UART COMMUNICATION PINS (Master-to-Front) =====
// Wiring: Cross-connect TX/RX between ESP32 boards
// Communication: 115200 baud, JSON protocol
#define PIN_UART_TX 22  // UART TX to Front ESP32 (Master → Slave)
#define PIN_UART_RX 21  // UART RX from Front ESP32 (Slave → Master)

// ===== BUZZER PIN (Active Buzzer) =====
// Wiring: Buzzer+ → GPIO33, Buzzer- → GND
// Power: 5V buzzer, controlled by 3.3V GPIO with transistor driver recommended
#define PIN_BUZZER 33   // Active Buzzer Control (3.3V output)

// ===== UNUSED SAFE GPIOs (Available for expansion) =====
// These pins are safe for additional sensors or actuators
#define PIN_AVAILABLE_1 25  // Safe DAC pin, available for expansion
#define PIN_AVAILABLE_2 26  // Safe DAC pin, available for expansion

// ===== EXCLUDED PINS (Bootstrap/Flash/Reserved) =====
// DO NOT USE: 0,1,2,3,5,6-12,15,16,17,20,34-39
// Reasons: Boot mode, UART debug, SPI flash, input-only

#endif // REAR_CONTROLLER

// ===== FRONT SLAVE ESP32 (MOTOR CONTROLLER) =====
#ifdef FRONT_CONTROLLER
/**
 * Board: ESP32 DevKit V1 - Slave Motor Controller
 * Function: Motor execution, receives commands via UART
 * Power: VIN (5V), GND
 * Safe GPIOs: 13,14,18,19,21-23,25-27
 */

// ===== MOTOR CONTROL PINS (2x L298N Drivers) =====
// Wiring: Two L298N modules for 9 motors total
// Power: Motors on 14.8V, L298N logic on 5V from buck converter

// L298N Driver 1 - Front Motors
#define PIN_DRIVER1_ENA 13  // Driver 1 Enable A (PWM speed control)
#define PIN_DRIVER1_IN1 14  // Driver 1 Input 1 (Direction control)
#define PIN_DRIVER1_IN2 18  // Driver 1 Input 2 (Direction control)
#define PIN_DRIVER1_IN3 19  // Driver 1 Input 3 (Direction control)
#define PIN_DRIVER1_IN4 23  // Driver 1 Input 4 (Direction control)
#define PIN_DRIVER1_ENB 25  // Driver 1 Enable B (PWM speed control)

// L298N Driver 2 - Center Motors
#define PIN_DRIVER2_ENA 21  // Driver 2 Enable A (PWM speed control)
#define PIN_DRIVER2_IN1 23  // Driver 2 Input 1 (Direction control)
#define PIN_DRIVER2_IN2 25  // Driver 2 Input 2 (Direction control)
#define PIN_DRIVER2_IN3 26  // Driver 2 Input 3 (Direction control)
#define PIN_DRIVER2_IN4 27  // Driver 2 Input 4 (Direction control)
#define PIN_DRIVER2_ENB 18  // Driver 2 Enable B (PWM speed control)

// ===== UART COMMUNICATION PINS (Slave-to-Master) =====
// Wiring: Cross-connect TX/RX with Rear ESP32
// Communication: 115200 baud, JSON protocol
#define PIN_UART_RX 22  // UART RX from Rear ESP32 (Slave receives commands)
#define PIN_UART_TX 23  // UART TX to Rear ESP32 (Slave sends status)

// ===== EXCLUDED PINS (Bootstrap/Flash/Reserved) =====
// DO NOT USE: 0,1,2,3,5,6-12,15,16,17,20,32-39
// Reasons: Boot mode, UART debug, SPI flash, ADC-only

#endif // FRONT_CONTROLLER

// ===== ESP32-CAM AI-THINKER (VISION MODULE) =====
#ifdef CAMERA_MODULE
/**
 * Board: ESP32-CAM AI-Thinker - Camera + Vision Processing
 * Function: Vision processing, ML inference, telemetry
 * Power: VIN (5V), GND
 * Safe GPIOs: 33 only (when camera/SD active)
 * ⚠️ GPIO0 LOW + RST for programming, then disconnect for normal operation
 */

// ===== STATUS LED PIN =====
// Wiring: GPIO33 → Status LED (RED LED - LOW=ON, HIGH=OFF)
// Note: Built-in red LED on GPIO33, inverted logic (LOW=ON)
#define PIN_STATUS_LED 33  // Status LED (LOW=ON, HIGH=OFF)
#define LED_ON LOW         // LED on state
#define LED_OFF HIGH       // LED off state

// ===== FLASH LED PIN =====
// Wiring: GPIO4 → Flash LED (optional external flash)
// Power: 3.3V output, suitable for LED with current limiting resistor
#define PIN_FLASH_LED 4    // Camera Flash LED (3.3V output)

// ===== CAMERA UART PINS (Optional telemetry) =====
// Note: Camera uses internal pins for camera interface
// These pins available if additional UART needed
#define PIN_CAM_UART_TX 1   // UART TX (conflicts with programming)
#define PIN_CAM_UART_RX 3   // UART RX (conflicts with programming)

// ===== CAMERA INTERNAL PINS (Built-in camera control) =====
// These pins are internally connected to OV2640 camera
// DO NOT connect external devices to these pins
#define PIN_CAM_PWDN 32     // Camera power down (internal)
#define PIN_CAM_XCLK 0      // Camera clock (internal)
#define PIN_CAM_SIOD 26     // Camera SDA (internal)
#define PIN_CAM_SIOC 27     // Camera SCL (internal)
#define PIN_CAM_D7 35       // Camera data bit 7 (internal)
#define PIN_CAM_D6 34       // Camera data bit 6 (internal)
#define PIN_CAM_D5 39       // Camera data bit 5 (internal)
#define PIN_CAM_D4 36       // Camera data bit 4 (internal)
#define PIN_CAM_VSYNC 5     // Camera VSYNC (internal)
#define PIN_CAM_HREF 18     // Camera HREF (internal)
#define PIN_CAM_PCLK 23     // Camera PCLK (internal)

// ===== SD CARD PINS (1-bit mode) =====
// Wiring: SD_MMC.begin("/sdcard", true) for 1-bit mode
// This frees GPIO12/13 for other use
#define PIN_SD_CMD 15       // SD card command (conflicts with boot)
#define PIN_SD_CLK 14       // SD card clock (conflicts with boot)
#define PIN_SD_DAT0 13      // SD card data bit 0 (conflicts with boot)

// ===== EXCLUDED PINS (Camera/SD/Reserved) =====
// DO NOT USE: 0 (boot), 2,4,12,13,14,15 (camera/SD), 16 (PSRAM)
// Note: GPIO33 is the ONLY reliable pin for external use

#endif // CAMERA_MODULE

// ===== VOLTAGE DIVIDER CALCULATIONS =====

/**
 * HC-SR04 Echo Pin Voltage Divider (5V → 3.3V)
 * Required: 5V ESP32 input → 3.3V GPIO input
 * 
 * Recommended: 1kΩ (top) + 2kΩ (bottom)
 * Calculation: Vout = Vin × (R2 / (R1 + R2))
 * Vout = 5V × (2000 / (1000 + 2000)) = 3.33V ✓
 * 
 * Alternative: 10kΩ + 20kΩ for lower current draw
 * Vout = 5V × (20000 / (10000 + 20000)) = 3.33V ✓
 * 
 * ⚠️ NEVER connect HC-SR04 Echo directly to ESP32 GPIO!
 */

// ===== PIN VALIDATION MACROS =====

// Verify pin numbers are in valid range for ESP32
#define VALIDATE_PIN(pin) ((pin) >= 0 && (pin) <= 39 && (pin) != 6 && (pin) != 7 && (pin) != 8 && (pin) != 9 && (pin) != 10 && (pin) != 11)

// Check if pin is safe for digital output
#define SAFE_OUTPUT_PIN(pin) ((pin) == 13 || (pin) == 14 || (pin) == 18 || (pin) == 19 || (pin) == 21 || (pin) == 22 || (pin) == 23 || (pin) == 25 || (pin) == 26 || (pin) == 27 || (pin) == 32 || (pin) == 33)

// Check if pin is safe for analog input
#define SAFE_ANALOG_PIN(pin) ((pin) == 32 || (pin) == 33 || (pin) == 34 || (pin) == 35 || (pin) == 36 || (pin) == 39)

// ===== PIN CONFIGURATION COMMENTS =====

/**
 * SUMMARY OF PIN USAGE BY FUNCTION:
 * 
 * MOTOR CONTROL:
 * - Rear ESP32: GPIO13,14,18,19,23,27 (6 pins, 1 L298N driver)
 * - Front ESP32: GPIO13,14,18,19,21,23,25,26,27 (9 pins, 2 L298N drivers)
 * 
 * SENSORS:
 * - Gas Sensor (MQ-2): GPIO32 (A0), GPIO33 (D0)
 * - Ultrasonic (HC-SR04): GPIO4 (Trig), GPIO36 (Echo with voltage divider)
 * 
 * COMMUNICATION:
 * - UART Master-Slave: GPIO22 (TX), GPIO21 (RX) between rear-front ESP32
 * - UART Master-Camera: GPIO1 (TX), GPIO3 (RX) between rear-ESP32-CAM
 * 
 * STATUS/INDICATORS:
 * - Buzzer: GPIO33 (rear ESP32)
 * - Status LED: GPIO33 (ESP32-CAM, LOW=ON)
 * - Flash LED: GPIO4 (ESP32-CAM)
 * 
 * CRITICAL CONSTRAINTS:
 * - ESP32 GPIO pins: 3.3V logic only
 * - HC-SR04 Echo: REQUIRES 5V→3.3V voltage divider
 * - ESP32-CAM: GPIO33 only safe for external use
 * - GPIO0: LOW for programming, disconnect for operation
 */

// ===== COMPATIBILITY MACROS =====

// Motor driver compatibility
#define L298N_ENABLE_A_PIN PIN_MOTOR_1
#define L298N_INPUT_1_PIN  PIN_MOTOR_2
#define L298N_INPUT_2_PIN  PIN_MOTOR_3
#define L298N_INPUT_3_PIN  PIN_MOTOR_4
#define L298N_INPUT_4_PIN  PIN_MOTOR_5
#define L298N_ENABLE_B_PIN PIN_MOTOR_6

// Sensor compatibility
#define GAS_SENSOR_ANALOG_PIN  PIN_GAS_ANALOG
#define GAS_SENSOR_DIGITAL_PIN PIN_GAS_DIGITAL
#define ULTRASONIC_TRIG_PIN    PIN_US_TRIG
#define ULTRASONIC_ECHO_PIN    PIN_US_ECHO

// Communication compatibility
#define MASTER_UART_TX_PIN PIN_UART_TX
#define MASTER_UART_RX_PIN PIN_UART_RX
#define SLAVE_UART_RX_PIN  PIN_UART_RX
#define SLAVE_UART_TX_PIN  PIN_UART_TX

#endif // PINS_H