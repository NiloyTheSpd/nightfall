/**
 * @file    main_rear_motor_test.cpp
 * @brief   Project Nightfall - Phase 2 MVP BACK ESP32 (Motor Master - Test Version)
 * @author  Project Nightfall Team
 * @version 2.0.0
 * @date    December 29, 2025
 *
 * Motor master controller for testing. Simplified version with:
 * - Motor control (sends commands to Front ESP32)
 * - UART communication to Front ESP32 (Serial2)
 * - Manual control via Serial monitor
 * - Motor testing sequences
 * - Basic safety monitoring (simplified)
 *
 * NOTE: Web server, WiFi, and sensor components commented out for motor testing
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// Include our libraries
#include "config.h"

// System state
unsigned long lastSensorUpdate = 0;
unsigned long lastMotorUpdate = 0;
bool systemReady = false;

// Safety system (simplified)
bool emergencyStop = false;
unsigned long emergencyTimestamp = 0;

// Motor control state
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;
int targetLeftSpeed = 0;
int targetRightSpeed = 0;

// Test state
unsigned long testStartTime = 0;
bool testActive = false;

// Function declarations
void setup();
void loop();
void initializeHardware();
void handleMainLoop();
void updateMotorControl();
void sendMotorCommand();
void processSerialCommands();
void runMotorTest();
void activateEmergencyStop(const String &reason);
void deactivateEmergencyStop();
void stopAllMotors();

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL BACK ESP32          ║");
    DEBUG_PRINTLN("║        Motor Master (Test Version)        ║");
    DEBUG_PRINTLN("║              Version 2.0.0                ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Initialize UART communication to Front ESP32
    Serial2.begin(UART_BAUDRATE);

    systemReady = true;
    lastMotorUpdate = millis();

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ BACK ESP32 Motor Master Ready!");
    DEBUG_PRINTLN("Motor testing mode - Web server disabled");
    DEBUG_PRINTLN("UART communication ready on Serial2");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Available commands:");
    DEBUG_PRINTLN("  forward    - Move forward");
    DEBUG_PRINTLN("  backward   - Move backward");
    DEBUG_PRINTLN("  left       - Turn left");
    DEBUG_PRINTLN("  right      - Turn right");
    DEBUG_PRINTLN("  stop       - Stop all motors");
    DEBUG_PRINTLN("  test       - Run motor test sequence");
    DEBUG_PRINTLN("  emergency  - Emergency stop");
    DEBUG_PRINTLN("  reset      - Reset emergency stop");
    DEBUG_PRINTLN("  status     - Show system status");
    DEBUG_PRINTLN();
}

// Main loop
void loop()
{
    handleMainLoop();
    yield();
}

void initializeHardware()
{
    DEBUG_PRINTLN("Initializing motor control hardware...");

    // Initialize motor control pins (for local testing/diagnostics)
    pinMode(13, OUTPUT); // Left Motor PWM
    pinMode(23, OUTPUT); // Left Motor IN1
    pinMode(22, OUTPUT); // Left Motor IN2
    pinMode(25, OUTPUT); // Right Motor PWM
    pinMode(26, OUTPUT); // Right Motor IN1
    pinMode(27, OUTPUT); // Right Motor IN2

    // Initialize UART pins
    pinMode(UART_TX, OUTPUT); // UART TX
    pinMode(UART_RX, INPUT);  // UART RX

    // Initialize safety/buzzer pin
    pinMode(4, OUTPUT); // Buzzer (emergency indication)

    // Stop all motors initially
    stopAllMotors();

    DEBUG_PRINTLN("Motor control hardware initialized");
    DEBUG_PRINTLN("Motor pins: PWM=13,25 | IN1=23,26 | IN2=22,27");
    DEBUG_PRINTLN("UART: TX=" + String(UART_TX) + " RX=" + String(UART_RX));
    DEBUG_PRINTLN("Safety: Buzzer on pin 4");
}

void handleMainLoop()
{
    unsigned long now = millis();

    // Process serial commands from user
    processSerialCommands();

    // Update motor control every 50ms
    if (now - lastMotorUpdate >= 50)
    {
        updateMotorControl();
        lastMotorUpdate = now;
    }

    // Run motor test if active
    if (testActive)
    {
        runMotorTest();
    }

    // Check emergency stop timeout (reset buzzer after 5 seconds)
    if (emergencyStop && (now - emergencyTimestamp >= 5000))
    {
        digitalWrite(4, LOW);
    }
}

void updateMotorControl()
{
    // Apply target speeds to motors
    leftMotorSpeed = targetLeftSpeed;
    rightMotorSpeed = targetRightSpeed;

    // Constrain motor speeds
    leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
    rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);

    // Update motor outputs (for local diagnostics)
    if (leftMotorSpeed >= 0)
    {
        analogWrite(13, leftMotorSpeed);
        digitalWrite(23, HIGH);
        digitalWrite(22, LOW);
    }
    else
    {
        analogWrite(13, abs(leftMotorSpeed));
        digitalWrite(23, LOW);
        digitalWrite(22, HIGH);
    }

    if (rightMotorSpeed >= 0)
    {
        analogWrite(25, rightMotorSpeed);
        digitalWrite(26, HIGH);
        digitalWrite(27, LOW);
    }
    else
    {
        analogWrite(25, abs(rightMotorSpeed));
        digitalWrite(26, LOW);
        digitalWrite(27, HIGH);
    }

    // Send motor commands to Front ESP32 via UART
    sendMotorCommand();
}

void sendMotorCommand()
{
    StaticJsonDocument<256> motorDoc;
    motorDoc["L"] = leftMotorSpeed;
    motorDoc["R"] = rightMotorSpeed;

    String motorCommand;
    serializeJson(motorDoc, motorCommand);

    // Send via UART to Front ESP32
    Serial2.print(motorCommand);
    Serial2.print("\n");

    DEBUG_PRINT("Motor command sent: L=");
    DEBUG_PRINT(leftMotorSpeed);
    DEBUG_PRINT(", R=");
    DEBUG_PRINTLN(rightMotorSpeed);
}

void processSerialCommands()
{
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();

        DEBUG_PRINT("Command received: ");
        DEBUG_PRINTLN(command);

        if (command == "forward")
        {
            targetLeftSpeed = 150;
            targetRightSpeed = 150;
            DEBUG_PRINTLN("Moving forward");
        }
        else if (command == "backward")
        {
            targetLeftSpeed = -150;
            targetRightSpeed = -150;
            DEBUG_PRINTLN("Moving backward");
        }
        else if (command == "left")
        {
            targetLeftSpeed = -100;
            targetRightSpeed = 100;
            DEBUG_PRINTLN("Turning left");
        }
        else if (command == "right")
        {
            targetLeftSpeed = 100;
            targetRightSpeed = -100;
            DEBUG_PRINTLN("Turning right");
        }
        else if (command == "stop")
        {
            targetLeftSpeed = 0;
            targetRightSpeed = 0;
            DEBUG_PRINTLN("Stopping all motors");
        }
        else if (command == "emergency")
        {
            activateEmergencyStop("Manual emergency stop");
        }
        else if (command == "reset")
        {
            deactivateEmergencyStop();
        }
        else if (command == "test")
        {
            DEBUG_PRINTLN("Starting motor test sequence...");
            testActive = true;
            testStartTime = millis();
        }
        else if (command == "status")
        {
            Serial.println("=== BACK ESP32 MOTOR MASTER STATUS ===");
            Serial.print("Uptime: ");
            Serial.print(millis() / 1000);
            Serial.println(" seconds");
            Serial.print("Emergency Stop: ");
            Serial.println(emergencyStop ? "YES" : "NO");
            Serial.print("Target Left Speed: ");
            Serial.println(targetLeftSpeed);
            Serial.print("Target Right Speed: ");
            Serial.println(targetRightSpeed);
            Serial.print("Current Left Speed: ");
            Serial.println(leftMotorSpeed);
            Serial.print("Current Right Speed: ");
            Serial.println(rightMotorSpeed);
            Serial.print("Test Active: ");
            Serial.println(testActive ? "YES" : "NO");
            Serial.println("=====================================");
        }
        else
        {
            Serial.println("Unknown command. Type 'help' for available commands.");
        }
    }
}

void runMotorTest()
{
    unsigned long testElapsed = millis() - testStartTime;

    // Test sequence:
    // 0-2s: Forward
    // 2-4s: Turn left
    // 4-6s: Turn right
    // 6-8s: Reverse
    // 8s: Stop

    if (testElapsed < 2000)
    {
        // Forward
        targetLeftSpeed = 120;
        targetRightSpeed = 120;
    }
    else if (testElapsed < 4000)
    {
        // Turn left
        targetLeftSpeed = -80;
        targetRightSpeed = 80;
    }
    else if (testElapsed < 6000)
    {
        // Turn right
        targetLeftSpeed = 80;
        targetRightSpeed = -80;
    }
    else if (testElapsed < 8000)
    {
        // Reverse
        targetLeftSpeed = -100;
        targetRightSpeed = -100;
    }
    else
    {
        // Stop
        targetLeftSpeed = 0;
        targetRightSpeed = 0;
        testActive = false;
        DEBUG_PRINTLN("Motor test sequence completed");
    }
}

void activateEmergencyStop(const String &reason)
{
    if (!emergencyStop)
    {
        emergencyStop = true;
        emergencyTimestamp = millis();
        targetLeftSpeed = 0;
        targetRightSpeed = 0;

        // Activate buzzer
        digitalWrite(4, HIGH);

        DEBUG_PRINTLN("🚨 EMERGENCY STOP ACTIVATED!");
        DEBUG_PRINT("Reason: ");
        DEBUG_PRINTLN(reason);
    }
}

void deactivateEmergencyStop()
{
    if (emergencyStop)
    {
        emergencyStop = false;
        emergencyTimestamp = 0;
        targetLeftSpeed = 0;
        targetRightSpeed = 0;
        digitalWrite(4, LOW);

        DEBUG_PRINTLN("Emergency stop reset - system resumed");
    }
}

void stopAllMotors()
{
    // Stop local motors
    analogWrite(13, 0);
    analogWrite(25, 0);
    digitalWrite(23, LOW);
    digitalWrite(22, LOW);
    digitalWrite(26, LOW);
    digitalWrite(27, LOW);

    // Send stop command to Front ESP32
    StaticJsonDocument<256> motorDoc;
    motorDoc["L"] = 0;
    motorDoc["R"] = 0;

    String motorCommand;
    serializeJson(motorDoc, motorCommand);
    Serial2.print(motorCommand);
    Serial2.print("\n");
}