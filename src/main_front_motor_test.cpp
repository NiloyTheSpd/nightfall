/**
 * @file    main_front_motor_test.cpp
 * @brief   Project Nightfall - Trial Version 1: MOTOR TESTING FOCUS
 * @author  Project Nightfall Team
 * @version 2.0.0-Trial1
 * @date    December 29, 2025
 *
 * FRONT ESP32 - MOTOR SLAVE CONTROLLER (Motor Testing Version)
 * This version focuses solely on motor control testing with all non-essential
 * hardware components disabled for streamlined motor verification.
 *
 * TRIAL VERSION 1 - MOTOR TESTING FOCUS:
 * - Keeps all motor control and UART communication code active
 * - Comments out sensor initialization and reading
 * - Comments out WiFi, WebSocket, and networking code
 * - Comments out safety monitoring, gas sensors, etc.
 * - Keeps emergency stop functionality
 * - Maintains motor driver pin configurations
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// Include our libraries
#include "config.h"

// System state
bool systemReady = false;
unsigned long lastUARTUpdate = 0;
unsigned long lastHeartbeat = 0;
bool emergencyStop = false;
unsigned long emergencyTimestamp = 0;

// Motor control state
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;
int targetLeftSpeed = 0;
int targetRightSpeed = 0;

// DISABLED FOR MOTOR TESTING - Sensor variables (commented out)
// float frontDistance = 0.0;
// float rearDistance = 0.0;
// int gasLevel = 0;

// DISABLED FOR MOTOR TESTING - WiFi variables (commented out)
// const char* ssid = "ProjectNightfall";
// const char* password = "rescue2025";

// Motor driver 1 (Front)
const int MOTOR1_LEFT_PWM = 13;
const int MOTOR1_LEFT_IN1 = 23;
const int MOTOR1_LEFT_IN2 = 22;
const int MOTOR1_RIGHT_PWM = 25;
const int MOTOR1_RIGHT_IN1 = 26;
const int MOTOR1_RIGHT_IN2 = 27;

// Motor driver 2 (Front Aux)
const int MOTOR2_LEFT_PWM = 14;
const int MOTOR2_LEFT_IN1 = 32;
const int MOTOR2_LEFT_IN2 = 33;
const int MOTOR2_RIGHT_PWM = 15;
const int MOTOR2_RIGHT_IN1 = 19;
const int MOTOR2_RIGHT_IN2 = 21;

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

// DISABLED FOR MOTOR TESTING - Sensor functions (commented out)
// void updateSensors();
// void checkSafetyConditions();

// DISABLED FOR MOTOR TESTING - WiFi/WebSocket functions (commented out)
// void setupWiFi();
// void handleWebSocketEvent();
// void sendTelemetry();

// DISABLED FOR MOTOR TESTING - Serial command handler (simplified)
// void handleSerialCommands();

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL FRONT ESP32         ║");
    DEBUG_PRINTLN("║      TRIAL VERSION 1 - MOTOR TESTING      ║");
    DEBUG_PRINTLN("║            Motor Slave Controller         ║");
    DEBUG_PRINTLN("║              Version 2.0.0-Trial1         ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Initialize UART communication
    Serial2.begin(UART_BAUDRATE);

    systemReady = true;
    lastUARTUpdate = millis();
    lastHeartbeat = millis();

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ FRONT ESP32 Motor Slave Ready (Trial Version 1)!");
    DEBUG_PRINTLN("🔧 MOTOR TESTING MODE - Non-essential features disabled");
    DEBUG_PRINTLN("Listening for UART commands on Serial2");
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

    // Initialize Motor Driver 1 pins
    pinMode(MOTOR1_LEFT_PWM, OUTPUT);
    pinMode(MOTOR1_LEFT_IN1, OUTPUT);
    pinMode(MOTOR1_LEFT_IN2, OUTPUT);
    pinMode(MOTOR1_RIGHT_PWM, OUTPUT);
    pinMode(MOTOR1_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR1_RIGHT_IN2, OUTPUT);

    // Initialize Motor Driver 2 pins
    pinMode(MOTOR2_LEFT_PWM, OUTPUT);
    pinMode(MOTOR2_LEFT_IN1, OUTPUT);
    pinMode(MOTOR2_LEFT_IN2, OUTPUT);
    pinMode(MOTOR2_RIGHT_PWM, OUTPUT);
    pinMode(MOTOR2_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR2_RIGHT_IN2, OUTPUT);

    // Stop all motors initially
    stopAllMotors();

    DEBUG_PRINTLN("Motor control hardware initialized");
    DEBUG_PRINTLN("Motor Driver 1: PWM=13, IN1=23, IN2=22 (Left) | PWM=25, IN1=26, IN2=27 (Right)");
    DEBUG_PRINTLN("Motor Driver 2: PWM=14, IN1=32, IN2=33 (Left) | PWM=15, IN1=19, IN2=21 (Right)");

    // DISABLED FOR MOTOR TESTING - Sensor initialization (commented out)
    /*
    DEBUG_PRINTLN("Initializing sensors...");
    // Initialize sensor pins
    pinMode(14, OUTPUT); // Front US Trig
    pinMode(18, INPUT);  // Front US Echo
    pinMode(19, OUTPUT); // Rear US Trig
    pinMode(21, INPUT);  // Rear US Echo
    pinMode(32, INPUT);  // Gas Sensor Analog
    pinMode(4, OUTPUT);  // Buzzer
    */

    // DISABLED FOR MOTOR TESTING - WiFi initialization (commented out)
    /*
    DEBUG_PRINTLN("Initializing WiFi...");
    setupWiFi();
    */

    DEBUG_PRINTLN("Hardware initialization complete (Motor Testing Mode)");
}

void handleMainLoop()
{
    unsigned long now = millis();

    // Listen for UART commands from Back ESP32
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

    // DISABLED FOR MOTOR TESTING - Sensor updates (commented out)
    /*
    // Update sensors every 100ms
    if (now - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL)
    {
        updateSensors();
        checkSafetyConditions();
        lastSensorUpdate = now;
    }
    */

    // DISABLED FOR MOTOR TESTING - WebSocket handling (commented out)
    /*
    // Handle WebSocket
    webSocketServer.loop();
    */

    // DISABLED FOR MOTOR TESTING - Serial commands (simplified)
    // handleSerialCommands();
}

void listenForUARTCommands()
{
    // Check for data from Back ESP32
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
    // Parse JSON command: {"L": val, "R": val}
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
        targetLeftSpeed = doc["L"];
        targetRightSpeed = doc["R"];

        // Constrain motor speeds to valid range
        targetLeftSpeed = constrain(targetLeftSpeed, -255, 255);
        targetRightSpeed = constrain(targetRightSpeed, -255, 255);

        DEBUG_PRINT("Motor speeds updated - Left: ");
        DEBUG_PRINT(targetLeftSpeed);
        DEBUG_PRINT(", Right: ");
        DEBUG_PRINTLN(targetRightSpeed);
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
            targetLeftSpeed = 0;
            targetRightSpeed = 0;
        }
        else if (cmd == "test")
        {
            DEBUG_PRINTLN("Test motor movement command received");
            testMotorMovement();
        }
        // DISABLED FOR MOTOR TESTING - Additional commands (commented out)
        /*
        else if (cmd == "forward")
        {
            targetLeftSpeed = 150;
            targetRightSpeed = 150;
        }
        else if (cmd == "backward")
        {
            targetLeftSpeed = -150;
            targetRightSpeed = -150;
        }
        else if (cmd == "left")
        {
            targetLeftSpeed = -100;
            targetRightSpeed = 100;
        }
        else if (cmd == "right")
        {
            targetLeftSpeed = 100;
            targetRightSpeed = -100;
        }
        */
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
    leftMotorSpeed = targetLeftSpeed;
    rightMotorSpeed = targetRightSpeed;

    // Update both motor drivers
    updateMotorDriver1(leftMotorSpeed, rightMotorSpeed);
    updateMotorDriver2(leftMotorSpeed, rightMotorSpeed);
}

void updateMotorDriver1(int leftSpeed, int rightSpeed)
{
    // Motor Driver 1 - Left motor
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

    // Motor Driver 1 - Right motor
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
    // Motor Driver 2 - Left motor
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

    // Motor Driver 2 - Right motor
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
        targetLeftSpeed = 0;
        targetRightSpeed = 0;

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
        targetLeftSpeed = 0;
        targetRightSpeed = 0;

        DEBUG_PRINTLN("Front ESP32 emergency stop reset - system resumed");
    }
}

void stopAllMotors()
{
    // Stop Motor Driver 1
    analogWrite(MOTOR1_LEFT_PWM, 0);
    analogWrite(MOTOR1_RIGHT_PWM, 0);
    digitalWrite(MOTOR1_LEFT_IN1, LOW);
    digitalWrite(MOTOR1_LEFT_IN2, LOW);
    digitalWrite(MOTOR1_RIGHT_IN1, LOW);
    digitalWrite(MOTOR1_RIGHT_IN2, LOW);

    // Stop Motor Driver 2
    analogWrite(MOTOR2_LEFT_PWM, 0);
    analogWrite(MOTOR2_RIGHT_PWM, 0);
    digitalWrite(MOTOR2_LEFT_IN1, LOW);
    digitalWrite(MOTOR2_LEFT_IN2, LOW);
    digitalWrite(MOTOR2_RIGHT_IN1, LOW);
    digitalWrite(MOTOR2_RIGHT_IN2, LOW);
}

void sendHeartbeat()
{
    // Send status back to Back ESP32 via Serial2
    StaticJsonDocument<256> heartbeatDoc;
    heartbeatDoc["type"] = "heartbeat";
    heartbeatDoc["source"] = "front";
    heartbeatDoc["timestamp"] = millis();
    heartbeatDoc["emergency"] = emergencyStop;
    heartbeatDoc["leftSpeed"] = leftMotorSpeed;
    heartbeatDoc["rightSpeed"] = rightMotorSpeed;
    heartbeatDoc["uptime"] = millis();
    heartbeatDoc["trial_version"] = "motor_testing_v1";

    String heartbeat;
    serializeJson(heartbeatDoc, heartbeat);
    Serial2.print(heartbeat);
    Serial2.print("\n");

    DEBUG_PRINT("Heartbeat sent - Emergency: ");
    DEBUG_PRINT(emergencyStop ? "YES" : "NO");
    DEBUG_PRINT(", Left Speed: ");
    DEBUG_PRINT(leftMotorSpeed);
    DEBUG_PRINT(", Right Speed: ");
    DEBUG_PRINTLN(rightMotorSpeed);
}

void testMotorMovement()
{
    DEBUG_PRINTLN("Testing motor movement...");

    // Test forward movement
    targetLeftSpeed = 100;
    targetRightSpeed = 100;
    delay(2000);

    // Test turning
    targetLeftSpeed = -100;
    targetRightSpeed = 100;
    delay(1000);

    // Test reverse
    targetLeftSpeed = -100;
    targetRightSpeed = -100;
    delay(2000);

    // Stop
    targetLeftSpeed = 0;
    targetRightSpeed = 0;

    DEBUG_PRINTLN("Motor test complete");
}

// DISABLED FOR MOTOR TESTING - Simplified sensor functions (commented out)
/*
void updateSensors()
{
    // Update Front Ultrasonic Sensor
    digitalWrite(14, LOW);
    delayMicroseconds(2);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);

    long duration = pulseIn(18, HIGH, 30000); // 30ms timeout
    if (duration > 0)
    {
        frontDistance = duration * 0.034 / 2;
        if (frontDistance > 400)
            frontDistance = 400; // Max range
    }

    // Update Rear Ultrasonic Sensor
    digitalWrite(19, LOW);
    delayMicroseconds(2);
    digitalWrite(19, HIGH);
    delayMicroseconds(10);
    digitalWrite(19, LOW);

    duration = pulseIn(21, HIGH, 30000); // 30ms timeout
    if (duration > 0)
    {
        rearDistance = duration * 0.034 / 2;
        if (rearDistance > 400)
            rearDistance = 400; // Max range
    }

    // Update Gas Sensor
    gasLevel = analogRead(32);
}

void checkSafetyConditions()
{
    // SAFETY OVERRIDE: If (FrontDistance < 20cm) OR (Gas > 400), MUST override commands
    if (!emergencyStop && (frontDistance < EMERGENCY_STOP_DISTANCE || gasLevel > GAS_THRESHOLD_ANALOG))
    {
        String reason = "";
        if (frontDistance < EMERGENCY_STOP_DISTANCE)
        {
            reason = "Obstacle detected: " + String(frontDistance, 1) + "cm";
        }
        if (gasLevel > GAS_THRESHOLD_ANALOG)
        {
            if (reason.length() > 0)
                reason += " & ";
            reason += "Gas level critical: " + String(gasLevel);
        }

        handleEmergencyStop();
    }
}
*/

// DISABLED FOR MOTOR TESTING - WiFi setup (commented out)
/*
void setupWiFi()
{
    DEBUG_PRINTLN("Setting up WiFi Access Point...");

    // Create WiFi access point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    DEBUG_PRINT("Access Point IP: ");
    DEBUG_PRINTLN(IP);
    DEBUG_PRINT("SSID: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("Password: ");
    DEBUG_PRINTLN(password);
}
*/

// DISABLED FOR MOTOR TESTING - Serial command handler (simplified for testing)
/*
void handleSerialCommands()
{
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "status")
        {
            Serial.println("=== FRONT ESP32 STATUS (TRIAL VERSION 1) ===");
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
            Serial.print("Last UART Update: ");
            Serial.print((millis() - lastUARTUpdate));
            Serial.println(" ms ago");
            Serial.println("Mode: MOTOR TESTING");
            Serial.println("===========================================");
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
            targetLeftSpeed = 0;
            targetRightSpeed = 0;
            Serial.println("Motors stopped");
        }
        else if (command == "forward")
        {
            targetLeftSpeed = 150;
            targetRightSpeed = 150;
            Serial.println("Moving forward");
        }
        else if (command == "help")
        {
            Serial.println("Available commands (Motor Testing Mode):");
            Serial.println("  status  - Show system status");
            Serial.println("  emergency - Emergency stop");
            Serial.println("  reset - Reset emergency stop");
            Serial.println("  stop - Stop motors");
            Serial.println("  forward - Move forward");
            Serial.println("  test - Test motor movement");
            Serial.println("  help - Show this help");
        }
    }
}
*/