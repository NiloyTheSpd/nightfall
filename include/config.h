/**
 * @file    config.h
 * @brief   Project Nightfall - Phase 2 MVP Configuration
 * @author  Project Nightfall Team
 * @version 2.0.0
 * @date    December 29, 2025
 *
 * Central configuration file for Phase 2 MVP Hybrid UART + WiFi Architecture
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== VERSION INFORMATION =====
#define VERSION_MAJOR 2
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_STRING "2.0.0"

// ===== HARDWARE CONFIGURATION =====
#define WHEEL_DIAMETER 85.0 // mm
#define WHEELBASE 250.0     // mm
#define TRACK_WIDTH 180.0   // mm

// ===== MOTOR CONTROL =====
#define MAX_MOTOR_SPEED 180   // PWM value (0-255)
#define CLIMB_MOTOR_SPEED 255 // PWM value for climbing
#define MOTOR_SPEED_RAMP 5    // PWM increment per cycle
#define TURN_DURATION 500     // ms
#define BACKUP_DURATION 1000  // ms

// ===== SAFETY THRESHOLDS =====
#define EMERGENCY_STOP_DISTANCE 20.0  // cm - SAFETY OVERRIDE TRIGGER
#define GAS_THRESHOLD_ANALOG 400      // ADC value - SAFETY OVERRIDE TRIGGER
#define LOW_BATTERY_VOLTAGE 12.5      // V
#define CRITICAL_BATTERY_VOLTAGE 11.5 // V
#define MAX_TILT_ANGLE 45.0           // degrees
#define MAX_MOTOR_CURRENT 2.5         // A

// ===== COMMUNICATION =====
#define UART_BAUDRATE 115200           // baud
#define WATCHDOG_TIMEOUT 5000          // ms
#define HEARTBEAT_INTERVAL 1000        // ms
#define SENSOR_UPDATE_INTERVAL 100     // ms
#define NAVIGATION_UPDATE_INTERVAL 100 // ms
#define TELEMETRY_INTERVAL 500         // ms
#define CAMERA_HEARTBEAT_INTERVAL 5000 // ms
#define EMERGENCY_TIMEOUT 1000         // ms

// ===== SENSOR CONFIGURATION =====
#define GAS_SAMPLE_INTERVAL 100 // ms
#define ULTRASONIC_TIMEOUT 30   // ms (max wait for echo)
#define CAMERA_FRAME_SIZE FRAMESIZE_VGA
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG
#define CAMERA_QUALITY 10 // JPEG quality (1-63, lower = better)

// ===== WEB DASHBOARD =====
#define WEBSOCKET_PORT 8888
#define HTTP_PORT 80
#define MAX_WEBSOCKET_CLIENTS 4
#define DASHBOARD_UPDATE_INTERVAL 100 // ms

// ===== MACHINE LEARNING =====
#define ML_CONFIDENCE_THRESHOLD 0.6 // 60%
#define ML_INFERENCE_INTERVAL 200   // ms
#define ML_IMAGE_SIZE 96            // pixels
#define ML_NUM_CLASSES 8

// ===== POWER MANAGEMENT =====
#define BATTERY_VOLTAGE_DIVIDER 2.0 // Voltage divider ratio
#define POWER_SAVE_ENABLED true
#define LOW_POWER_VOLTAGE 12.0 // V
#define SLEEP_INTERVAL 300000  // ms (5 minutes)

// ===== DATA LOGGING =====
#define LOGGING_ENABLED true
#define TELEMETRY_INTERVAL 500 // ms
#define SD_CARD_ENABLED true
#define MAX_LOG_FILE_SIZE 10485760 // 10MB

// ===== DEBUGGING =====
#ifdef SERIAL_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif

#ifdef VERBOSE_LOGGING
#define VERBOSE_PRINT(x) Serial.print(x)
#define VERBOSE_PRINTLN(x) Serial.println(x)
#else
#define VERBOSE_PRINT(x)
#define VERBOSE_PRINTLN(x)
#endif

// ===== PIN ASSIGNMENTS =====

// REAR MAIN ESP32 (Master Controller)
// Board: ESP32 DevKit V1 - Master/Brain Controller
#ifdef REAR_CONTROLLER
// MQ-2 Gas Sensor - Wiring: VCC->3.3V, GND->GND, A0->GPIO32, D0->GPIO33
#define PIN_GAS_ANALOG 32  // MQ-2 Gas Sensor Analog Output (A0)
#define PIN_GAS_DIGITAL 33 // MQ-2 Gas Sensor Digital Output (D0/Buzzer)

// HC-SR04 Ultrasonic Sensor - Wiring: VCC->5V, GND->GND, Trig->GPIO4, Echo->GPIO36 (with voltage divider!)
#define PIN_US_TRIG 4  // HC-SR04 Trigger Pin (5V output from ESP32)
#define PIN_US_ECHO 36 // HC-SR04 Echo Pin (5V input - REQUIRES 5V->3.3V voltage divider!)

// UART Master - Wiring: TX22->RX22(Front), RX21->TX23(Front)
#define PIN_UART_TX 22 // UART TX to Front ESP32 Slave
#define PIN_UART_RX 21 // UART RX from Front ESP32 Slave

// Motors (L298N Driver) - Wiring: 6 motor control pins
#define PIN_MOTOR_1 13 // Motor 1 Control (L298N IN1/ENA)
#define PIN_MOTOR_2 14 // Motor 2 Control (L298N IN2/ENB)
#define PIN_MOTOR_3 18 // Motor 3 Control (L298N IN3)
#define PIN_MOTOR_4 19 // Motor 4 Control (L298N IN4)
#define PIN_MOTOR_5 23 // Motor 5 Control (L298N PWM/ENA)
#define PIN_MOTOR_6 27 // Motor 6 Control (L298N PWM/ENB)
#endif

// FRONT SLAVE ESP32 (Slave Controller)
// Board: ESP32 DevKit V1 - Motor Slave Controller
#ifdef FRONT_CONTROLLER
// Motors (2x L298N Drivers) - Wiring: 9 motor control pins
#define PIN_MOTOR_1 13 // Motor 1 Control (L298N #1 IN1/ENA)
#define PIN_MOTOR_2 14 // Motor 2 Control (L298N #1 IN2/ENB)
#define PIN_MOTOR_3 18 // Motor 3 Control (L298N #1 IN3)
#define PIN_MOTOR_4 19 // Motor 4 Control (L298N #1 IN4)
#define PIN_MOTOR_5 21 // Motor 5 Control (L298N #2 IN1/ENA)
#define PIN_MOTOR_6 23 // Motor 6 Control (L298N #2 IN2/ENB)
#define PIN_MOTOR_7 25 // Motor 7 Control (L298N #2 IN3)
#define PIN_MOTOR_8 26 // Motor 8 Control (L298N #2 IN4)
#define PIN_MOTOR_9 27 // Motor 9 Control (L298N #2 PWM/ENB)

// UART Slave - Wiring: RX22<-TX22(Rear), TX23->RX21(Rear)
#define PIN_UART_RX 22 // UART RX from Rear ESP32 Master
#define PIN_UART_TX 23 // UART TX to Rear ESP32 Master
#endif

// ESP32-CAM (Vision Module)
// Board: ESP32-CAM AI-Thinker - Camera Module
#ifdef CAMERA_MODULE
// Status LED - Wiring: GPIO33->LED (LOW=ON, HIGH=OFF)
#define PIN_STATUS_LED 33 // Status LED (RED LED - LOW=ON)

// Note: Camera pins are internal to ESP32-CAM module
// No additional pin definitions needed for camera/SD card
#endif

// ===== SYSTEM STATES =====
enum RobotState
{
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_MANUAL,
    STATE_FORWARD,
    STATE_TURNING,
    STATE_BACKING_UP,
    STATE_EMERGENCY,
    STATE_SHUTDOWN
};

// ===== MESSAGE TYPES =====
enum MessageType
{
    MSG_COMMAND = 0,
    MSG_STATUS,
    MSG_HEARTBEAT,
    MSG_ALERT,
    MSG_ML_DETECTION,
    MSG_TELEMETRY
};

// ===== MANUAL CONTROL STATES =====
enum ControlState
{
    CONTROL_INIT = 0,
    CONTROL_READY,
    CONTROL_ACTIVE,
    CONTROL_STOPPED
};

// ===== SENSOR CLASSES =====
enum SensorType
{
    SENSOR_ULTRASONIC_FRONT = 0,
    SENSOR_ULTRASONIC_REAR,
    SENSOR_GAS,
    SENSOR_SMOKE
};

// ===== ALERT TYPES =====
enum AlertType
{
    ALERT_COLLISION = 0,
    ALERT_GAS_DETECTED,
    ALERT_LOW_BATTERY,
    ALERT_OVERHEAT,
    ALERT_COMMUNICATION_LOSS,
    ALERT_TILT_EXCESSIVE,
    ALERT_STUCK,
    ALERT_SENSOR_FAILURE
};

// ===== ALERT LEVELS =====
enum AlertLevel
{
    ALERT_NONE = 0,
    ALERT_INFO,
    ALERT_WARNING,
    ALERT_CRITICAL
};

// ===== MOVEMENT COMMANDS =====
enum MovementCommand
{
    MOVE_STOP = 0,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_TURN_LEFT,
    MOVE_TURN_RIGHT,
    MOVE_CLIMB
};

// ===== TIMING CONSTANTS =====
#define SENSOR_READ_TIMEOUT 30     // ms
#define COMMUNICATION_TIMEOUT 5000 // ms
#define EMERGENCY_RESPONSE_TIME 50 // ms
#define MOTOR_RESPONSE_TIME 10     // ms

// ===== MEMORY LIMITS =====
#define MAX_JSON_DOCUMENT_SIZE 512 // bytes
#define MAX_TELEMETRY_BUFFER 100
#define MAX_ALERT_BUFFER 20
#define MAX_SENSOR_HISTORY 50

// ===== COMPILATION FLAGS =====
#ifdef PRODUCTION_MODE
#define DEBUG_ENABLED false
#define LOG_LEVEL ERROR
#define SAFETY_MARGIN 1.2
#elif defined(STAGING_MODE)
#define DEBUG_ENABLED true
#define LOG_LEVEL WARNING
#define SAFETY_MARGIN 1.1
#else // DEVELOPMENT_MODE
#define DEBUG_ENABLED true
#define LOG_LEVEL DEBUG
#define SAFETY_MARGIN 1.0
#endif

#endif // CONFIG_H