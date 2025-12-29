/**
 * @file    main_front.cpp
 * @brief   Project Nightfall - Phase 2 MVP FRONT ESP32 (Motor Slave)
 * @author  Project Nightfall Team
 * @version 2.0.0
 * @date    December 29, 2025
 *
 * Motor slave controller for the autonomous rescue robot. Handles:
 * - Listens on UART (Serial2) for motor commands {"L": val, "R": val}
 * - Dual motor driver control (4 motors total)
 * - Emergency stop if no UART data received for 1000ms
 * - Non-blocking operation with millis() timers
 * - Robust JSON parsing with ArduinoJson v7
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebSockets.h>
#include <WebSocketsClient.h>

// Include our libraries
#include "config.h"

// System state
bool systemReady = false;
unsigned long lastUARTUpdate = 0;
unsigned long lastHeartbeat = 0;
bool emergencyStop = false;
unsigned long emergencyTimestamp = 0;

// WiFi communication (Test Version 2)
#ifdef TEST_VERSION_2_WIFI_NETWORKING
bool wifiConnected = false;
WebSocketsClient webSocket;
bool webSocketConnected = false;
unsigned long lastWiFiHeartbeat = 0;
const char *rearESP32IP = "192.168.4.1";
const uint16_t rearWebSocketPort = 8888;
#endif

// Motor control state
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;
int targetLeftSpeed = 0;
int targetRightSpeed = 0;

// Motor driver definitions (using preprocessor macros from platformio.ini)
// Motor driver 1 (Front): MOTOR_LEFT_PWM, MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_RIGHT_PWM, MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2
// Motor driver 2 (Front Aux): MOTOR2_LEFT_PWM, MOTOR2_LEFT_IN1, MOTOR2_LEFT_IN2, MOTOR2_RIGHT_PWM, MOTOR2_RIGHT_IN1, MOTOR2_RIGHT_IN2

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

#ifdef TEST_VERSION_2_WIFI_NETWORKING
void initializeWiFi();
void handleWiFiCommunication();
void sendWiFiHeartbeat();
void connectToWiFi();
void connectToWebSocket();
void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
#endif

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL FRONT ESP32         ║");
    DEBUG_PRINTLN("║            Motor Slave Controller         ║");
    DEBUG_PRINTLN("║              Version 2.0.0                ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Initialize UART communication
    Serial2.begin(UART_BAUDRATE);

#ifdef TEST_VERSION_2_WIFI_NETWORKING
    // Initialize WiFi for Test Version 2
    initializeWiFi();
#endif

    systemReady = true;
    lastUARTUpdate = millis();
    lastHeartbeat = millis();

#ifdef TEST_VERSION_2_WIFI_NETWORKING
    lastWiFiHeartbeat = millis();
#endif

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ FRONT ESP32 Motor Slave Ready!");
#ifdef TEST_VERSION_2_WIFI_NETWORKING
    DEBUG_PRINTLN("WiFi + UART Hybrid Communication Mode");
    DEBUG_PRINT("WiFi Host: ");
    DEBUG_PRINTLN(rearESP32IP);
#else
    DEBUG_PRINTLN("UART Communication Mode");
#endif
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

#ifdef TEST_VERSION_2_WIFI_NETWORKING
    // Test Version 2: Single motor driver configuration
    // Motor Driver: PWM=13, IN1=23, IN2=22 (Left) | PWM=25, IN1=26, IN2=27 (Right)
    pinMode(13, OUTPUT); // Left Motor PWM
    pinMode(23, OUTPUT); // Left Motor IN1
    pinMode(22, OUTPUT); // Left Motor IN2
    pinMode(25, OUTPUT); // Right Motor PWM
    pinMode(26, OUTPUT); // Right Motor IN1
    pinMode(27, OUTPUT); // Right Motor IN2
#else
    // Legacy: Dual motor driver configuration
    // Initialize Motor Driver 1 pins
    pinMode(MOTOR_LEFT_PWM, OUTPUT);
    pinMode(MOTOR_LEFT_IN1, OUTPUT);
    pinMode(MOTOR_LEFT_IN2, OUTPUT);
    pinMode(MOTOR_RIGHT_PWM, OUTPUT);
    pinMode(MOTOR_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR_RIGHT_IN2, OUTPUT);

    // Initialize Motor Driver 2 pins
    pinMode(MOTOR2_LEFT_PWM, OUTPUT);
    pinMode(MOTOR2_LEFT_IN1, OUTPUT);
    pinMode(MOTOR2_LEFT_IN2, OUTPUT);
    pinMode(MOTOR2_RIGHT_PWM, OUTPUT);
    pinMode(MOTOR2_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR2_RIGHT_IN2, OUTPUT);
#endif

    // Stop all motors initially
    stopAllMotors();

    DEBUG_PRINTLN("Motor control hardware initialized");
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

#ifdef TEST_VERSION_2_WIFI_NETWORKING
    // Handle WiFi communication
    handleWiFiCommunication();
#endif

    // Send periodic heartbeat
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL)
    {
        sendHeartbeat();
        lastHeartbeat = now;
    }
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
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, command);

    if (error)
    {
        DEBUG_PRINTLN("JSON parse error in motor command");
        return;
    }

    // Extract motor speeds
    if (doc["L"].is<int>() && doc["R"].is<int>())
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
    else if (doc["cmd"].is<String>())
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
        analogWrite(MOTOR_LEFT_PWM, leftSpeed);
        digitalWrite(MOTOR_LEFT_IN1, HIGH);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
    }
    else
    {
        analogWrite(MOTOR_LEFT_PWM, abs(leftSpeed));
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, HIGH);
    }

    // Motor Driver 1 - Right motor
    if (rightSpeed >= 0)
    {
        analogWrite(MOTOR_RIGHT_PWM, rightSpeed);
        digitalWrite(MOTOR_RIGHT_IN1, HIGH);
        digitalWrite(MOTOR_RIGHT_IN2, LOW);
    }
    else
    {
        analogWrite(MOTOR_RIGHT_PWM, abs(rightSpeed));
        digitalWrite(MOTOR_RIGHT_IN1, LOW);
        digitalWrite(MOTOR_RIGHT_IN2, HIGH);
    }
}

void updateMotorDriver2(int leftSpeed, int rightSpeed)
{
#ifndef TEST_VERSION_2_WIFI_NETWORKING
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
#endif
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
    analogWrite(MOTOR_LEFT_PWM, 0);
    analogWrite(MOTOR_RIGHT_PWM, 0);
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);

#ifndef TEST_VERSION_2_WIFI_NETWORKING
    // Stop Motor Driver 2
    analogWrite(MOTOR2_LEFT_PWM, 0);
    analogWrite(MOTOR2_RIGHT_PWM, 0);
    digitalWrite(MOTOR2_LEFT_IN1, LOW);
    digitalWrite(MOTOR2_LEFT_IN2, LOW);
    digitalWrite(MOTOR2_RIGHT_IN1, LOW);
    digitalWrite(MOTOR2_RIGHT_IN2, LOW);
#endif
}

void sendHeartbeat()
{
#ifdef TEST_VERSION_2_WIFI_NETWORKING
    // Send heartbeat via WiFi WebSocket if connected, otherwise UART
    if (wifiConnected && webSocketConnected)
    {
        JsonDocument heartbeatDoc;
        heartbeatDoc["type"] = "heartbeat";
        heartbeatDoc["source"] = "front";
        heartbeatDoc["timestamp"] = millis();
        heartbeatDoc["emergency"] = emergencyStop;
        heartbeatDoc["leftSpeed"] = leftMotorSpeed;
        heartbeatDoc["rightSpeed"] = rightMotorSpeed;
        heartbeatDoc["uptime"] = millis();
        heartbeatDoc["wifi_rssi"] = WiFi.RSSI();
        heartbeatDoc["wifi_ip"] = WiFi.localIP().toString();

        String heartbeat;
        serializeJson(heartbeatDoc, heartbeat);
        webSocket.sendTXT(heartbeat);

        DEBUG_PRINT("WiFi Heartbeat sent - Emergency: ");
        DEBUG_PRINT(emergencyStop ? "YES" : "NO");
        DEBUG_PRINT(", Left Speed: ");
        DEBUG_PRINT(leftMotorSpeed);
        DEBUG_PRINT(", Right Speed: ");
        DEBUG_PRINTLN(rightMotorSpeed);
    }
    else
    {
        // Fallback to UART
        JsonDocument heartbeatDoc;
        heartbeatDoc["type"] = "heartbeat";
        heartbeatDoc["source"] = "front";
        heartbeatDoc["timestamp"] = millis();
        heartbeatDoc["emergency"] = emergencyStop;
        heartbeatDoc["leftSpeed"] = leftMotorSpeed;
        heartbeatDoc["rightSpeed"] = rightMotorSpeed;
        heartbeatDoc["uptime"] = millis();

        String heartbeat;
        serializeJson(heartbeatDoc, heartbeat);
        Serial2.print(heartbeat);
        Serial2.print("\n");

        DEBUG_PRINT("UART Heartbeat sent - Emergency: ");
        DEBUG_PRINT(emergencyStop ? "YES" : "NO");
        DEBUG_PRINT(", Left Speed: ");
        DEBUG_PRINT(leftMotorSpeed);
        DEBUG_PRINT(", Right Speed: ");
        DEBUG_PRINTLN(rightMotorSpeed);
    }
#else
    // Legacy UART-only heartbeat
    JsonDocument heartbeatDoc;
    heartbeatDoc["type"] = "heartbeat";
    heartbeatDoc["source"] = "front";
    heartbeatDoc["timestamp"] = millis();
    heartbeatDoc["emergency"] = emergencyStop;
    heartbeatDoc["leftSpeed"] = leftMotorSpeed;
    heartbeatDoc["rightSpeed"] = rightMotorSpeed;
    heartbeatDoc["uptime"] = millis();

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
#endif
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

// Serial command handler for debugging
void handleSerialCommands()
{
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "status")
        {
            Serial.println("=== FRONT ESP32 STATUS ===");
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
            Serial.println("========================");
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
            Serial.println("Available commands:");
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

#ifdef TEST_VERSION_2_WIFI_NETWORKING
void initializeWiFi()
{
    DEBUG_PRINTLN("Initializing WiFi client for Test Version 2...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    DEBUG_PRINT("Connecting to WiFi");

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 10000))
    {
        delay(500);
        DEBUG_PRINT(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        IPAddress IP = WiFi.localIP();

        DEBUG_PRINTLN();
        DEBUG_PRINTLN("WiFi connected successfully!");
        DEBUG_PRINT("IP Address: ");
        DEBUG_PRINTLN(IP);
        DEBUG_PRINT("RSSI: ");
        DEBUG_PRINT(WiFi.RSSI());
        DEBUG_PRINTLN(" dBm");

        // Connect to WebSocket
        connectToWebSocket();
    }
    else
    {
        wifiConnected = false;
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("WiFi connection failed! Using UART fallback.");
    }
}

void connectToWebSocket()
{
    if (wifiConnected)
    {
        DEBUG_PRINTLN("Connecting to WebSocket...");
        webSocket.begin(rearESP32IP, rearWebSocketPort);
        webSocket.onEvent(handleWebSocketEvent);
        webSocket.setReconnectInterval(5000);
    }
}

void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        DEBUG_PRINTLN("[WSc] Disconnected!");
        webSocketConnected = false;
        break;
    case WStype_CONNECTED:
        DEBUG_PRINTLN("[WSc] Connected to url: ");
        DEBUG_PRINTLN((char *)payload);
        webSocketConnected = true;
        break;
    case WStype_TEXT:
        DEBUG_PRINTLN("[WSc] get text: ");
        DEBUG_PRINTLN((char *)payload);
        processMotorCommand(String((char *)payload));
        break;
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
    }
}

void handleWiFiCommunication()
{
    // Handle WebSocket communication
    if (wifiConnected)
    {
        webSocket.loop();

        // Send periodic heartbeat
        unsigned long now = millis();
        if (now - lastWiFiHeartbeat >= HEARTBEAT_INTERVAL)
        {
            sendWiFiHeartbeat();
            lastWiFiHeartbeat = now;
        }

        // Check WiFi connection status
        if (WiFi.status() != WL_CONNECTED)
        {
            wifiConnected = false;
            webSocketConnected = false;
            DEBUG_PRINTLN("WiFi disconnected, attempting reconnection...");
            connectToWiFi();
        }
    }
}

void sendWiFiHeartbeat()
{
    if (webSocketConnected)
    {
        JsonDocument heartbeatDoc;
        heartbeatDoc["type"] = "heartbeat";
        heartbeatDoc["source"] = "front";
        heartbeatDoc["timestamp"] = millis();
        heartbeatDoc["emergency"] = emergencyStop;
        heartbeatDoc["leftSpeed"] = leftMotorSpeed;
        heartbeatDoc["rightSpeed"] = rightMotorSpeed;
        heartbeatDoc["uptime"] = millis();
        heartbeatDoc["wifi_rssi"] = WiFi.RSSI();
        heartbeatDoc["wifi_ip"] = WiFi.localIP().toString();

        String heartbeat;
        serializeJson(heartbeatDoc, heartbeat);
        webSocket.sendTXT(heartbeat);

        DEBUG_PRINT("WiFi Heartbeat sent - RSSI: ");
        DEBUG_PRINT(WiFi.RSSI());
        DEBUG_PRINT(" dBm");
    }
}

void connectToWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN("Attempting WiFi reconnection...");
        WiFi.reconnect();
        delay(1000);

        if (WiFi.status() == WL_CONNECTED)
        {
            wifiConnected = true;
            connectToWebSocket();
        }
    }
}
#endif
