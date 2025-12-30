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

// ESP32 #1 (Front/Master Controller) - GPIO pins
#ifdef FRONT_CONTROLLER
#define PIN_MOTOR_ENA 18 // Front Left Motor PWM
#define PIN_MOTOR_IN1 19 // Front Left Motor Forward
#define PIN_MOTOR_IN2 21 // Front Left Motor Reverse
#define PIN_MOTOR_ENB 22 // Front Right Motor PWM
#define PIN_MOTOR_IN3 23 // Front Right Motor Forward
#define PIN_MOTOR_IN4 25 // Front Right Motor Reverse

#define PIN_US_TRIG 26 // Front Ultrasonic Trigger
#define PIN_US_ECHO 27 // Front Ultrasonic Echo

#define PIN_GAS_ANALOG 32  // MQ-2 Gas Sensor Analog
#define PIN_GAS_DIGITAL 33 // MQ-2 Gas Sensor Digital

#define PIN_BUZZER 13 // Audio Alert

#define PIN_UART2_TX 16 // UART2 TX to Rear ESP32
#define PIN_UART2_RX 17 // UART2 RX from Rear ESP32
#define PIN_UART1_TX 14 // UART1 TX to Camera
#define PIN_UART1_RX 12 // UART1 RX from Camera
#endif

// ESP32 #2 (Rear/Slave Controller) - GPIO pins
#ifdef REAR_CONTROLLER
#define PIN_MOTOR_ENA 18 // Rear Left Motor PWM
#define PIN_MOTOR_IN1 19 // Rear Left Motor Forward
#define PIN_MOTOR_IN2 21 // Rear Left Motor Reverse
#define PIN_MOTOR_ENB 22 // Rear Right Motor PWM
#define PIN_MOTOR_IN3 23 // Rear Right Motor Forward
#define PIN_MOTOR_IN4 25 // Rear Right Motor Reverse

#define PIN_US_TRIG 26 // Rear Ultrasonic Trigger
#define PIN_US_ECHO 27 // Rear Ultrasonic Echo

#define PIN_UART2_TX 16 // UART2 TX to Front ESP32
#define PIN_UART2_RX 17 // UART2 RX from Front ESP32
#endif

// ESP32-CAM (Vision Module) - GPIO pins
#ifdef CAMERA_MODULE
#define PIN_UART1_TX 14 // UART1 TX to Front ESP32
#define PIN_UART1_RX 15 // UART1 RX from Front ESP32
#define PIN_FLASH_LED 4 // Flash LED control (optional)

// Camera pins are internal to ESP32-CAM
// No additional pin definitions needed
#endif

// ===== SYSTEM STATES =====
enum RobotState
{
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_AUTONOMOUS,
    STATE_MANUAL,
    STATE_FORWARD,
    STATE_AVOIDING,
    STATE_TURNING,
    STATE_CLIMBING,
    STATE_BACKING_UP,
    STATE_SCANNING,
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

// ===== NAVIGATION STATES =====
enum NavigationState
{
    NAV_INIT = 0,
    NAV_FORWARD,
    NAV_AVOID,
    NAV_TURN,
    NAV_CLIMB,
    NAV_BACKUP,
    NAV_SCAN,
    NAV_STUCK
};

// ===== ML CLASSES =====
enum MLObjectClass
{
    ML_WALL = 0,
    ML_PERSON,
    ML_STAIRS,
    ML_DOOR,
    ML_FURNITURE,
    ML_DEBRIS,
    ML_VEHICLE,
    ML_UNKNOWN
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