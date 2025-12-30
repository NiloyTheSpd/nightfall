/**
 * @file    main_front_enhanced.cpp
 * @brief   Project Nightfall - Enhanced Front ESP32 (Motor Slave)
 * @author  Project Nightfall Team
 * @version 3.0.0
 * @date    December 30, 2025
 *
 * Enhanced motor slave controller for the manual six-motor rescue robot. Handles:
 * - Listens on UART for motor commands from Rear ESP32
 * - Controls 4 motors (2 motor driver pairs) for front and center wheels
 * - Emergency stop if no UART data received for 1000ms
 * - Heartbeat communication back to Rear ESP32
 * - Non-blocking operation with millis() timers
 * - Robust JSON parsing with ArduinoJson v7
 * - Individual motor testing and diagnostics
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// Include our libraries
#include "config.h"
#include "pins.h"

// Define controller type for conditional compilation
#define FRONT_CONTROLLER

// System state
bool systemReady = false;
unsigned long lastUARTUpdate = 0;
unsigned long lastHeartbeat = 0;
bool emergencyStop = false;
unsigned long emergencyTimestamp = 0;

// Enhanced motor control state - Four motors total
int frontLeftMotorSpeed = 0;   // Motor 1
int frontRightMotorSpeed = 0;  // Motor 2
int centerLeftMotorSpeed = 0;  // Motor 3
int centerRightMotorSpeed = 0; // Motor 4

// Target speeds for smooth control
int targetFrontLeftSpeed = 0;
int targetFrontRightSpeed = 0;
int targetCenterLeftSpeed = 0;
int targetCenterRightSpeed = 0;

// Motor driver pin assignments per pin.md
// Motor Driver 1 (Front Left/Right) - GPIO13,14,18,19
#define MOTOR1_LEFT_PWM 13
#define MOTOR1_LEFT_IN1 14
#define MOTOR1_LEFT_IN2 18
#define MOTOR1_RIGHT_PWM 19
#define MOTOR1_RIGHT_IN1 23
#define MOTOR1_RIGHT_IN2 25

// Motor Driver 2 (Center Left/Right) - GPIO21,23,25,26,27
#define MOTOR2_LEFT_PWM 21
#define MOTOR2_LEFT_IN1 23
#define MOTOR2_LEFT_IN2 25
#define MOTOR2_RIGHT_PWM 26
#define MOTOR2_RIGHT_IN1 27
#define MOTOR2_RIGHT_IN2 13 // Reusing GPIO13 for additional control

// Function declarations
void setup();
void loop();
void initializeHardware();
void handleMainLoop();
void listenForUARTCommands();
void processMotorCommand(const String &command);
void updateMotorControl();
void checkUARTTimeout();
void handleEmergencyStop();
void resetEmergencyStop();
void sendHeartbeat();
void updateMotorDriver1(int leftSpeed, int rightSpeed);
void updateMotorDriver2(int leftSpeed, int rightSpeed);
void stopAllMotors();
void testMotorMovement();
void testIndividualMotor(int motorNumber);
void sendMotorStatus();
void handleSerialCommands();

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL ENHANCED FRONT ESP32║");
    DEBUG_PRINTLN("║         Four-Motor Slave Controller       ║");
    DEBUG_PRINTLN("║              Version 3.0.0                ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Initialize UART communication with Rear ESP32
    Serial2.begin(UART_BAUDRATE);

    systemReady = true;
    lastUARTUpdate = millis();
    lastHeartbeat = millis();

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ ENHANCED FRONT ESP32 Motor Slave Ready!");
    DEBUG_PRINTLN("Listening for UART commands on Serial2");
    DEBUG_PRINTLN("Controlling 4 motors: Front Pair + Center Pair");
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
    DEBUG_PRINTLN("Initializing nine-motor control hardware...");

    // Initialize Motor Driver 1 pins (Front motors) - GPIO13,14,18,19,23,25
    pinMode(MOTOR1_LEFT_PWM, OUTPUT);  // GPIO13
    pinMode(MOTOR1_LEFT_IN1, OUTPUT);  // GPIO14
    pinMode(MOTOR1_LEFT_IN2, OUTPUT);  // GPIO18
    pinMode(MOTOR1_RIGHT_PWM, OUTPUT); // GPIO19
    pinMode(MOTOR1_RIGHT_IN1, OUTPUT); // GPIO23
    pinMode(MOTOR1_RIGHT_IN2, OUTPUT); // GPIO25

    // Initialize Motor Driver 2 pins (Center motors) - GPIO21,23,25,26,27
    pinMode(MOTOR2_LEFT_PWM, OUTPUT);  // GPIO21
    pinMode(MOTOR2_LEFT_IN1, OUTPUT);  // GPIO23
    pinMode(MOTOR2_LEFT_IN2, OUTPUT);  // GPIO25
    pinMode(MOTOR2_RIGHT_PWM, OUTPUT); // GPIO26
    pinMode(MOTOR2_RIGHT_IN1, OUTPUT); // GPIO27
    pinMode(MOTOR2_RIGHT_IN2, OUTPUT); // GPIO13 (reused)

    // Initialize UART Slave pins - Wiring: RX22<-TX22(Rear), TX23->RX21(Rear)
    pinMode(PIN_UART_RX, INPUT);  // GPIO22 - UART RX from Rear ESP32 Master
    pinMode(PIN_UART_TX, OUTPUT); // GPIO23 - UART TX to Rear ESP32 Master

    // Stop all motors initially
    stopAllMotors();

    DEBUG_PRINTLN("Nine-motor control hardware initialized");
    DEBUG_PRINTLN("Motor Driver 1: GPIO13,14,18,19,23,25 (Front Motors)");
    DEBUG_PRINTLN("Motor Driver 2: GPIO21,23,25,26,27 (Center Motors)");
    DEBUG_PRINTLN("UART: GPIO22 (RX), GPIO23 (TX) to Rear ESP32");
}

void handleMainLoop()
{
    unsigned long now = millis();

    // Listen for UART commands from Rear ESP32
    listenForUARTCommands();

    // Check for UART timeout (emergency stop if no data for 1000ms)
    checkUARTTimeout();

    // Update motor control
    updateMotorControl();

    // Send periodic heartbeat
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL)
    {
        sendHeartbeat();
        lastHeartbeat = now;
    }

    // Handle serial commands for debugging
    handleSerialCommands();
}

void listenForUARTCommands()
{
    // Check for data from Rear ESP32
    if (Serial2.available())
    {
        String command = Serial2.readStringUntil('\n');
        command.trim();

        if (command.length() > 0)
        {
            lastUARTUpdate = millis();

            DEBUG_PRINT("UART Command received: ");
            DEBUG_PRINTLN(command);

            processMotorCommand(command);
        }
    }
}

void processMotorCommand(const String &command)
{
    // Parse JSON command: {"L": val, "R": val, "CL": val, "CR": val}
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, command);

    if (error)
    {
        DEBUG_PRINTLN("JSON parse error in motor command");
        return;
    }

    // Extract motor speeds
    if (doc.containsKey("L") && doc.containsKey("R"))
    {
        targetFrontLeftSpeed = doc["L"];
        targetFrontRightSpeed = doc["R"];

        // Handle center motors if present
        if (doc.containsKey("CL"))
        {
            targetCenterLeftSpeed = doc["CL"];
        }
        if (doc.containsKey("CR"))
        {
            targetCenterRightSpeed = doc["CR"];
        }

        // Constrain motor speeds to valid range
        targetFrontLeftSpeed = constrain(targetFrontLeftSpeed, -255, 255);
        targetFrontRightSpeed = constrain(targetFrontRightSpeed, -255, 255);
        targetCenterLeftSpeed = constrain(targetCenterLeftSpeed, -255, 255);
        targetCenterRightSpeed = constrain(targetCenterRightSpeed, -255, 255);

        DEBUG_PRINT("Motor speeds updated - Front L: ");
        DEBUG_PRINT(targetFrontLeftSpeed);
        DEBUG_PRINT(", Front R: ");
        DEBUG_PRINT(targetFrontRightSpeed);
        DEBUG_PRINT(", Center L: ");
        DEBUG_PRINT(targetCenterLeftSpeed);
        DEBUG_PRINT(", Center R: ");
        DEBUG_PRINTLN(targetCenterRightSpeed);
    }
    else if (doc.containsKey("cmd"))
    {
        String cmd = doc["cmd"];

        if (cmd == "emergency_stop")
        {
            DEBUG_PRINTLN("Emergency stop command received");
            handleEmergencyStop();
        }
        else if (cmd == "emergency_reset")
        {
            DEBUG_PRINTLN("Emergency reset command received");
            resetEmergencyStop();
        }
        else if (cmd == "stop")
        {
            DEBUG_PRINTLN("Stop command received");
            targetFrontLeftSpeed = 0;
            targetFrontRightSpeed = 0;
            targetCenterLeftSpeed = 0;
            targetCenterRightSpeed = 0;
        }
        else if (cmd == "test")
        {
            DEBUG_PRINTLN("Test motor movement command received");
            testMotorMovement();
        }
        else if (cmd == "test_motor_1")
        {
            DEBUG_PRINTLN("Testing motor 1 (Front Left)");
            testIndividualMotor(1);
        }
        else if (cmd == "test_motor_2")
        {
            DEBUG_PRINTLN("Testing motor 2 (Front Right)");
            testIndividualMotor(2);
        }
        else if (cmd == "test_motor_3")
        {
            DEBUG_PRINTLN("Testing motor 3 (Center Left)");
            testIndividualMotor(3);
        }
        else if (cmd == "test_motor_4")
        {
            DEBUG_PRINTLN("Testing motor 4 (Center Right)");
            testIndividualMotor(4);
        }
        else if (cmd == "status")
        {
            DEBUG_PRINTLN("Sending motor status");
            sendMotorStatus();
        }
    }
}

void updateMotorControl()
{
    if (emergencyStop)
    {
        // In emergency stop, ensure all motors are stopped
        stopAllMotors();
        return;
    }

    // Apply target speeds to motors
    frontLeftMotorSpeed = targetFrontLeftSpeed;
    frontRightMotorSpeed = targetFrontRightSpeed;
    centerLeftMotorSpeed = targetCenterLeftSpeed;
    centerRightMotorSpeed = targetCenterRightSpeed;

    // Update both motor drivers
    updateMotorDriver1(frontLeftMotorSpeed, frontRightMotorSpeed);
    updateMotorDriver2(centerLeftMotorSpeed, centerRightMotorSpeed);
}

void updateMotorDriver1(int leftSpeed, int rightSpeed)
{
    // Motor Driver 1 - Front Left motor
    if (leftSpeed >= 0)
    {
        analogWrite(MOTOR1_LEFT_PWM, leftSpeed);
        digitalWrite(MOTOR1_LEFT_IN1, HIGH);
        digitalWrite(MOTOR1_LEFT_IN2, LOW);
    }
    else
    {
        analogWrite(MOTOR1_LEFT_PWM, abs(leftSpeed));
        digitalWrite(MOTOR1_LEFT_IN1, LOW);
        digitalWrite(MOTOR1_LEFT_IN2, HIGH);
    }

    // Motor Driver 1 - Front Right motor
    if (rightSpeed >= 0)
    {
        analogWrite(MOTOR1_RIGHT_PWM, rightSpeed);
        digitalWrite(MOTOR1_RIGHT_IN1, HIGH);
        digitalWrite(MOTOR1_RIGHT_IN2, LOW);
    }
    else
    {
        analogWrite(MOTOR1_RIGHT_PWM, abs(rightSpeed));
        digitalWrite(MOTOR1_RIGHT_IN1, LOW);
        digitalWrite(MOTOR1_RIGHT_IN2, HIGH);
    }
}

void updateMotorDriver2(int leftSpeed, int rightSpeed)
{
    // Motor Driver 2 - Center Left motor
    if (leftSpeed >= 0)
    {
        analogWrite(MOTOR2_LEFT_PWM, leftSpeed);
        digitalWrite(MOTOR2_LEFT_IN1, HIGH);
        digitalWrite(MOTOR2_LEFT_IN2, LOW);
    }
    else
    {
        analogWrite(MOTOR2_LEFT_PWM, abs(leftSpeed));
        digitalWrite(MOTOR2_LEFT_IN1, LOW);
        digitalWrite(MOTOR2_LEFT_IN2, HIGH);
    }

    // Motor Driver 2 - Center Right motor
    if (rightSpeed >= 0)
    {
        analogWrite(MOTOR2_RIGHT_PWM, rightSpeed);
        digitalWrite(MOTOR2_RIGHT_IN1, HIGH);
        digitalWrite(MOTOR2_RIGHT_IN2, LOW);
    }
    else
    {
        analogWrite(MOTOR2_RIGHT_PWM, abs(rightSpeed));
        digitalWrite(MOTOR2_RIGHT_IN1, LOW);
        digitalWrite(MOTOR2_RIGHT_IN2, HIGH);
    }
}

void checkUARTTimeout()
{
    unsigned long now = millis();

    // If no UART data received for 1000ms, trigger emergency stop
    if (!emergencyStop && (now - lastUARTUpdate > EMERGENCY_TIMEOUT))
    {
        DEBUG_PRINTLN("⚠️ UART timeout - No data for 1000ms, entering emergency stop");
        handleEmergencyStop();
    }
}

void handleEmergencyStop()
{
    if (!emergencyStop)
    {
        emergencyStop = true;
        emergencyTimestamp = millis();
        targetFrontLeftSpeed = 0;
        targetFrontRightSpeed = 0;
        targetCenterLeftSpeed = 0;
        targetCenterRightSpeed = 0;

        // Immediately stop all motors
        stopAllMotors();

        DEBUG_PRINTLN("🚨 FRONT ESP32 EMERGENCY STOP ACTIVATED!");
    }
}

void resetEmergencyStop()
{
    if (emergencyStop)
    {
        emergencyStop = false;
        emergencyTimestamp = 0;
        targetFrontLeftSpeed = 0;
        targetFrontRightSpeed = 0;
        targetCenterLeftSpeed = 0;
        targetCenterRightSpeed = 0;

        DEBUG_PRINTLN("Front ESP32 emergency stop reset - system resumed");
    }
}

void stopAllMotors()
{
    // Stop Motor Driver 1 (Front motors)
    analogWrite(MOTOR1_LEFT_PWM, 0);
    analogWrite(MOTOR1_RIGHT_PWM, 0);
    digitalWrite(MOTOR1_LEFT_IN1, LOW);
    digitalWrite(MOTOR1_LEFT_IN2, LOW);
    digitalWrite(MOTOR1_RIGHT_IN1, LOW);
    digitalWrite(MOTOR1_RIGHT_IN2, LOW);

    // Stop Motor Driver 2 (Center motors)
    analogWrite(MOTOR2_LEFT_PWM, 0);
    analogWrite(MOTOR2_RIGHT_PWM, 0);
    digitalWrite(MOTOR2_LEFT_IN1, LOW);
    digitalWrite(MOTOR2_LEFT_IN2, LOW);
    digitalWrite(MOTOR2_RIGHT_IN1, LOW);
    digitalWrite(MOTOR2_RIGHT_IN2, LOW);
}

void sendHeartbeat()
{
    // Send status back to Rear ESP32 via Serial2
    StaticJsonDocument<256> heartbeatDoc;
    heartbeatDoc["type"] = "heartbeat";
    heartbeatDoc["source"] = "front";
    heartbeatDoc["timestamp"] = millis();
    heartbeatDoc["emergency"] = emergencyStop;
    heartbeatDoc["leftSpeed"] = frontLeftMotorSpeed;
    heartbeatDoc["rightSpeed"] = frontRightMotorSpeed;
    heartbeatDoc["centerLeftSpeed"] = centerLeftMotorSpeed;
    heartbeatDoc["centerRightSpeed"] = centerRightMotorSpeed;
    heartbeatDoc["uptime"] = millis();
    heartbeatDoc["motorsActive"] = (frontLeftMotorSpeed != 0 || frontRightMotorSpeed != 0 ||
                                    centerLeftMotorSpeed != 0 || centerRightMotorSpeed != 0);

    String heartbeat;
    serializeJson(heartbeatDoc, heartbeat);
    Serial2.print(heartbeat);
    Serial2.print("\n");

    DEBUG_PRINT("Heartbeat sent - Emergency: ");
    DEBUG_PRINT(emergencyStop ? "YES" : "NO");
    DEBUG_PRINT(", Front L: ");
    DEBUG_PRINT(frontLeftMotorSpeed);
    DEBUG_PRINT(", Front R: ");
    DEBUG_PRINT(frontRightMotorSpeed);
    DEBUG_PRINT(", Center L: ");
    DEBUG_PRINT(centerLeftMotorSpeed);
    DEBUG_PRINT(", Center R: ");
    DEBUG_PRINTLN(centerRightMotorSpeed);
}

void testMotorMovement()
{
    DEBUG_PRINTLN("Testing four-motor movement...");

    // Test forward movement - all motors
    targetFrontLeftSpeed = 100;
    targetFrontRightSpeed = 100;
    targetCenterLeftSpeed = 100;
    targetCenterRightSpeed = 100;
    delay(2000);

    // Test turning - differential speed
    targetFrontLeftSpeed = -50;
    targetFrontRightSpeed = 100;
    targetCenterLeftSpeed = -50;
    targetCenterRightSpeed = 100;
    delay(1500);

    // Test reverse
    targetFrontLeftSpeed = -100;
    targetFrontRightSpeed = -100;
    targetCenterLeftSpeed = -100;
    targetCenterRightSpeed = -100;
    delay(2000);

    // Stop
    targetFrontLeftSpeed = 0;
    targetFrontRightSpeed = 0;
    targetCenterLeftSpeed = 0;
    targetCenterRightSpeed = 0;

    DEBUG_PRINTLN("Four-motor test complete");
}

void testIndividualMotor(int motorNumber)
{
    DEBUG_PRINT("Testing individual motor ");
    DEBUG_PRINTLN(motorNumber);

    // Reset all motors
    targetFrontLeftSpeed = 0;
    targetFrontRightSpeed = 0;
    targetCenterLeftSpeed = 0;
    targetCenterRightSpeed = 0;

    delay(500);

    // Test specified motor
    switch (motorNumber)
    {
    case 1: // Front Left
        targetFrontLeftSpeed = 100;
        break;
    case 2: // Front Right
        targetFrontRightSpeed = 100;
        break;
    case 3: // Center Left
        targetCenterLeftSpeed = 100;
        break;
    case 4: // Center Right
        targetCenterRightSpeed = 100;
        break;
    default:
        DEBUG_PRINTLN("Invalid motor number");
        return;
    }

    delay(2000);

    // Stop test motor
    switch (motorNumber)
    {
    case 1:
        targetFrontLeftSpeed = 0;
        break;
    case 2:
        targetFrontRightSpeed = 0;
        break;
    case 3:
        targetCenterLeftSpeed = 0;
        break;
    case 4:
        targetCenterRightSpeed = 0;
        break;
    }

    DEBUG_PRINTLN("Individual motor test complete");
}

void sendMotorStatus()
{
    DEBUG_PRINTLN("=== FRONT ESP32 MOTOR STATUS ===");
    DEBUG_PRINT("Uptime: ");
    DEBUG_PRINT(millis() / 1000);
    DEBUG_PRINTLN(" seconds");
    DEBUG_PRINT("Emergency Stop: ");
    DEBUG_PRINTLN(emergencyStop ? "YES" : "NO");
    DEBUG_PRINT("Target Front Left Speed: ");
    DEBUG_PRINTLN(targetFrontLeftSpeed);
    DEBUG_PRINT("Target Front Right Speed: ");
    DEBUG_PRINTLN(targetFrontRightSpeed);
    DEBUG_PRINT("Target Center Left Speed: ");
    DEBUG_PRINTLN(targetCenterLeftSpeed);
    DEBUG_PRINT("Target Center Right Speed: ");
    DEBUG_PRINTLN(targetCenterRightSpeed);
    DEBUG_PRINT("Current Front Left Speed: ");
    DEBUG_PRINTLN(frontLeftMotorSpeed);
    DEBUG_PRINT("Current Front Right Speed: ");
    DEBUG_PRINTLN(frontRightMotorSpeed);
    DEBUG_PRINT("Current Center Left Speed: ");
    DEBUG_PRINTLN(centerLeftMotorSpeed);
    DEBUG_PRINT("Current Center Right Speed: ");
    DEBUG_PRINTLN(centerRightMotorSpeed);
    DEBUG_PRINT("Last UART Update: ");
    DEBUG_PRINT((millis() - lastUARTUpdate));
    DEBUG_PRINTLN(" ms ago");
    DEBUG_PRINTLN("================================");
}

// Serial command handler for debugging
void handleSerialCommands()
{
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "status")
        {
            sendMotorStatus();
        }
        else if (command == "emergency")
        {
            handleEmergencyStop();
        }
        else if (command == "reset")
        {
            resetEmergencyStop();
        }
        else if (command == "test")
        {
            testMotorMovement();
        }
        else if (command == "stop")
        {
            targetFrontLeftSpeed = 0;
            targetFrontRightSpeed = 0;
            targetCenterLeftSpeed = 0;
            targetCenterRightSpeed = 0;
            Serial.println("All motors stopped");
        }
        else if (command == "forward")
        {
            targetFrontLeftSpeed = 150;
            targetFrontRightSpeed = 150;
            targetCenterLeftSpeed = 150;
            targetCenterRightSpeed = 150;
            Serial.println("Moving forward with all motors");
        }
        else if (command.startsWith("motor "))
        {
            int motorNum = command.substring(6).toInt();
            testIndividualMotor(motorNum);
        }
        else if (command == "help")
        {
            Serial.println("Available commands:");
            Serial.println("  status  - Show system status");
            Serial.println("  emergency - Emergency stop");
            Serial.println("  reset - Reset emergency stop");
            Serial.println("  stop - Stop all motors");
            Serial.println("  forward - Move forward");
            Serial.println("  test - Test all motors");
            Serial.println("  motor N - Test individual motor (1-4)");
            Serial.println("  help - Show this help");
        }
    }
}