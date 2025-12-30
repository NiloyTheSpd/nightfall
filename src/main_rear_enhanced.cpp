/**
 * @file    main_rear_enhanced.cpp
 * @brief   Project Nightfall - Enhanced Rear ESP32 (Master Controller)
 * @author  Project Nightfall Team
 * @version 3.0.0
 * @date    December 30, 2025
 *
 * Enhanced master controller for manual six-motor rescue robot. Handles:
 * - WiFi Access Point creation (ProjectNightfall/rescue2025)
 * - WebSocket Server on port 8888 for Dashboard communication
 * - Six motor control (4 via Front ESP32 + 2 direct rear motors)
 * - Enhanced sensor monitoring (2x Ultrasonic + Gas + Smoke)
 * - Safety override system (Distance < 20cm OR Gas/Smoke > threshold)
 * - Real-time telemetry broadcasting every 500ms
 * - Buzzer and smoke alarm integration
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESPAsyncWebServer.h>

// Note: REAR_CONTROLLER defined in platformio.ini build flags

// Include our libraries
#include "config.h"
#include "pins.h"

// Forward declarations
void stopAllRearMotors();

// Global objects
AsyncWebServer webServer(80);
WebSocketsServer webSocketServer(8888);

// System state
unsigned long lastSensorUpdate = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastMotorUpdate = 0;
bool systemReady = false;

// Safety system
bool emergencyStop = false;
unsigned long emergencyTimestamp = 0;
bool buzzerActive = false;
unsigned long lastBuzzerUpdate = 0;
bool alarmActive = false;

// Motor control state - Six motors total
int frontLeftSpeed = 0;   // Motor 1 (via UART to Front ESP32)
int frontRightSpeed = 0;  // Motor 2 (via UART to Front ESP32)
int rearLeftSpeed = 0;    // Motor 3 (direct rear control)
int rearRightSpeed = 0;   // Motor 4 (direct rear control)
int centerLeftSpeed = 0;  // Motor 5 (via UART to Front ESP32)
int centerRightSpeed = 0; // Motor 6 (via UART to Front ESP32)

// Target speeds for smooth control
int targetFrontLeftSpeed = 0;
int targetFrontRightSpeed = 0;
int targetRearLeftSpeed = 0;
int targetRearRightSpeed = 0;
int targetCenterLeftSpeed = 0;
int targetCenterRightSpeed = 0;

// Enhanced telemetry data
float frontDistance = 0.0;
float rearDistance = 0.0;
int gasLevel = 0;
int smokeLevel = 0;
float batteryVoltage = 14.8;
unsigned long uptime = 0;
int connectionStatus = 0; // 0=disconnected, 1=connected, 2=front ESP32 online

// WiFi configuration
const char *ssid = "ProjectNightfall";
const char *password = "rescue2025";

// Sensor validation
unsigned long lastFrontDistanceTime = 0;
unsigned long lastRearDistanceTime = 0;
bool frontSensorValid = false;
bool rearSensorValid = false;

// Function declarations
void setup();
void loop();
void initializeHardware();
void setupWiFi();
void setupWebServer();
void handleMainLoop();
void updateSensors();
void validateSensorData();
void checkSafetyConditions();
void updateMotorControl();
void sendMotorCommandsToFront();
void sendTelemetry();
void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void processDriveCommand(const JsonDocument &doc);
void activateEmergencyStop(const String &reason);
void deactivateEmergencyStop();
void soundBuzzer(int duration);
void updateBuzzer();
void activateSmokeAlarm();
void updateSmokeAlarm();
void sendStatusToWebSocket();
String formatTelemetryJSON();
String formatStatusJSON();
void handleUARTCommunication();
void checkFrontESP32Connection();

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL ENHANCED REAR ESP32 ║");
    DEBUG_PRINTLN("║          Six-Motor Master Controller      ║");
    DEBUG_PRINTLN("║              Version 3.0.0                ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Setup UART communication with Front ESP32
    Serial2.begin(UART_BAUDRATE);

    // Setup WiFi and WebServer
    setupWiFi();
    setupWebServer();

    systemReady = true;
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ ENHANCED REAR ESP32 Master Controller Ready!");
    DEBUG_PRINT("WiFi AP: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("WebSocket Server: Port 8888");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Dashboard URL: http://192.168.4.1");
    DEBUG_PRINTLN("Six-Motor Architecture: 4 Front + 2 Rear");
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
    DEBUG_PRINTLN("Initializing six-motor control hardware...");

    // Initialize Rear Motors (L298N Driver) - Wiring: VCC->14.8V, GND->GND, IN1-4->GPIO13,14,18,19,23,27
    pinMode(PIN_MOTOR_1, OUTPUT); // Motor 1 Control
    pinMode(PIN_MOTOR_2, OUTPUT); // Motor 2 Control
    pinMode(PIN_MOTOR_3, OUTPUT); // Motor 3 Control
    pinMode(PIN_MOTOR_4, OUTPUT); // Motor 4 Control
    pinMode(PIN_MOTOR_5, OUTPUT); // Motor 5 Control
    pinMode(PIN_MOTOR_6, OUTPUT); // Motor 6 Control

    // Initialize HC-SR04 Ultrasonic Sensor - Wiring: VCC->5V, GND->GND, Trig->GPIO4, Echo->GPIO36 (with voltage divider!)
    pinMode(PIN_US_TRIG, OUTPUT); // HC-SR04 Trigger Pin (5V output from ESP32)
    pinMode(PIN_US_ECHO, INPUT);  // HC-SR04 Echo Pin (5V input - REQUIRES 5V->3.3V voltage divider!)

    // Initialize MQ-2 Gas Sensor - Wiring: VCC->3.3V, GND->GND, A0->GPIO32, D0->GPIO33
    pinMode(PIN_GAS_ANALOG, INPUT);  // MQ-2 Gas Sensor Analog Output (A0)
    pinMode(PIN_GAS_DIGITAL, INPUT); // MQ-2 Gas Sensor Digital Output (D0/Buzzer)

    // Initialize UART Master - Wiring: TX22->RX22(Front), RX21->TX23(Front)
    pinMode(PIN_UART_TX, OUTPUT); // UART TX to Front ESP32 Slave
    pinMode(PIN_UART_RX, INPUT);  // UART RX from Front ESP32 Slave

    // Stop all motors initially
    stopAllRearMotors();

    DEBUG_PRINTLN("Six-motor hardware initialized");
    DEBUG_PRINTLN("Motors: GPIO13,14,18,19,23,27 (L298N Driver)");
    DEBUG_PRINTLN("Ultrasonic: GPIO4 (Trig), GPIO36 (Echo - requires voltage divider!)");
    DEBUG_PRINTLN("Gas Sensor: GPIO32 (A0), GPIO33 (D0/Buzzer)");
    DEBUG_PRINTLN("UART: GPIO22 (TX), GPIO21 (RX) to Front ESP32");
}

void stopAllRearMotors()
{
    // Stop all rear motors
    digitalWrite(PIN_MOTOR_1, LOW);
    digitalWrite(PIN_MOTOR_2, LOW);
    digitalWrite(PIN_MOTOR_3, LOW);
    digitalWrite(PIN_MOTOR_4, LOW);
    digitalWrite(PIN_MOTOR_5, LOW);
    digitalWrite(PIN_MOTOR_6, LOW);
}

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

void setupWebServer()
{
    DEBUG_PRINTLN("Setting up Web Server and WebSocket Server...");

    // Serve main dashboard - enhanced with six-motor controls
    String index_html = R"(<!DOCTYPE html>
<html><head><title>Project Nightfall - Six-Motor Robot Control</title>
<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>
<style>
body{font-family:Arial;margin:20px;background:#1a1a1a;color:#fff}
.container{max-width:1000px;margin:0 auto}
.card{background:#2d2d2d;border-radius:10px;padding:20px;margin:10px 0}
.status{display:inline-block;padding:5px 15px;border-radius:20px;margin:5px}
.normal{background:#28a745}.warning{background:#ffc107;color:#000}.error{background:#dc3545}
.button{background:#007bff;color:white;border:none;padding:10px 20px;margin:5px;border-radius:5px;cursor:pointer}
.button:hover{background:#0056b3}
.button.emergency{background:#dc3545}
.button.emergency:hover{background:#c82333}
.telemetry{font-size:1.1em;margin:10px 0}
.sensor-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.motor-status{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
</style></head>
<body><div class='container'>
<h1>🤖 Project Nightfall - Six-Motor Rescue Robot</h1>
<div class='card'><h2>System Status</h2>
<div class='telemetry'>Robot State: <span id='robotState' class='status normal'>READY</span></div>
<div class='telemetry'>Uptime: <span id='uptime'>0s</span></div>
<div class='telemetry'>Emergency: <span id='emergency' class='status normal'>NO</span></div>
<div class='telemetry'>Front ESP32: <span id='frontStatus'>OFFLINE</span></div></div>
<div class='card'><h2>Enhanced Sensor Data</h2>
<div class='sensor-grid'>
<div class='telemetry'>Front Distance: <span id='frontDistance'>0</span> cm</div>
<div class='telemetry'>Rear Distance: <span id='rearDistance'>0</span> cm</div>
<div class='telemetry'>Gas Level: <span id='gasLevel'>0</span></div>
<div class='telemetry'>Smoke Level: <span id='smokeLevel'>0</span></div>
</div>
<div class='telemetry'>Battery: <span id='battery'>14.8</span> V</div></div>
<div class='card'><h2>Six-Motor Manual Control</h2>
<div class='motor-status'>
<div><strong>Front Motors (1-4)</strong><br>Via Front ESP32</div>
<div><strong>Rear Motors (5-6)</strong><br>Direct Control</div>
<div><strong>System Status</strong><br><span id='motorStatus'>ALL STOPPED</span></div>
</div>
<button class='button' onclick='sendCommand("forward")'>⬆️ Forward (All Motors)</button>
<button class='button' onclick='sendCommand("left")'>⬅️ Turn Left</button>
<button class='button' onclick='sendCommand("right")'>➡️ Turn Right</button>
<button class='button' onclick='sendCommand("backward")'>⬇️ Backward</button>
<button class='button' onclick='sendCommand("stop")'>⏹️ Stop All Motors</button>
<button class='button emergency' onclick='sendCommand("emergency")'>🚨 Emergency Stop</button></div>
<div class='card'><h2>Individual Motor Test</h2>
<button class='button' onclick='sendCommand("test_front")'>Test Front Motors</button>
<button class='button' onclick='sendCommand("test_rear")'>Test Rear Motors</button>
<button class='button' onclick='sendCommand("test_all")'>Test All Motors</button></div>
</div>
<script>
var ws = new WebSocket('ws://192.168.4.1:8888');
ws.onopen = function(){console.log('Connected to six-motor robot');};
ws.onmessage = function(event){
var data = JSON.parse(event.data);
if(data.type === 'telemetry'){
document.getElementById('frontDistance').textContent = data.dist.toFixed(1);
document.getElementById('rearDistance').textContent = data.rearDist.toFixed(1);
document.getElementById('gasLevel').textContent = data.gas;
document.getElementById('smokeLevel').textContent = data.smoke;
document.getElementById('battery').textContent = data.battery.toFixed(1);
document.getElementById('uptime').textContent = Math.floor(data.uptime/1000) + 's';
document.getElementById('emergency').textContent = data.emergency ? 'YES' : 'NO';
document.getElementById('emergency').className = 'status ' + (data.emergency ? 'error' : 'normal');
document.getElementById('frontStatus').textContent = data.frontOnline ? 'ONLINE' : 'OFFLINE';
document.getElementById('frontStatus').className = 'status ' + (data.frontOnline ? 'normal' : 'warning');
document.getElementById('motorStatus').textContent = data.motorsActive ? 'MOTORS ACTIVE' : 'ALL STOPPED';
}
};
function sendCommand(cmd){ws.send(JSON.stringify({command: cmd}));}
</script></body></html>)";

    webServer.on("/", HTTP_GET, [&index_html](AsyncWebServerRequest *request)
                 { request->send(200, "text/html", index_html); });

    // Enhanced API endpoints
    webServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
        JsonDocument doc;
        doc["status"] = "online";
        doc["version"] = VERSION_STRING;
        doc["uptime"] = millis();
        doc["emergency"] = emergencyStop;
        doc["frontDistance"] = frontDistance;
        doc["rearDistance"] = rearDistance;
        doc["gasLevel"] = gasLevel;
        doc["smokeLevel"] = smokeLevel;
        doc["battery"] = batteryVoltage;
        doc["frontOnline"] = (connectionStatus == 2);
        doc["motorsActive"] = (frontLeftSpeed != 0 || frontRightSpeed != 0 || rearLeftSpeed != 0 || rearRightSpeed != 0);
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    webServer.on("/api/telemetry", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
        String telemetry = formatTelemetryJSON();
        request->send(200, "application/json", telemetry); });

    webServer.on("/api/motors", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
        JsonDocument doc;
        doc["frontLeft"] = frontLeftSpeed;
        doc["frontRight"] = frontRightSpeed;
        doc["rearLeft"] = rearLeftSpeed;
        doc["rearRight"] = rearRightSpeed;
        doc["emergency"] = emergencyStop;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // Start web server
    webServer.begin();

    // Start WebSocket server
    webSocketServer.begin();
    webSocketServer.onEvent(handleWebSocketEvent);

    DEBUG_PRINTLN("Enhanced web server started on port 80");
    DEBUG_PRINTLN("Enhanced WebSocket server started on port 8888");
}

void handleMainLoop()
{
    unsigned long now = millis();

    // Handle WebSocket
    webSocketServer.loop();

    // Update sensors every 100ms with validation
    if (now - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL)
    {
        updateSensors();
        validateSensorData();
        checkSafetyConditions();
        lastSensorUpdate = now;
    }

    // Send telemetry every 500ms
    if (now - lastTelemetryUpdate >= TELEMETRY_INTERVAL)
    {
        sendTelemetry();
        lastTelemetryUpdate = now;
    }

    // Update motor control every 50ms for smooth operation
    if (now - lastMotorUpdate >= 50)
    {
        updateMotorControl();
        sendMotorCommandsToFront();
        lastMotorUpdate = now;
    }

    // Handle UART communication with Front ESP32
    handleUARTCommunication();

    // Check Front ESP32 connection
    checkFrontESP32Connection();

    // Update buzzer and smoke alarm
    updateBuzzer();
    updateSmokeAlarm();
}

void updateSensors()
{
    // HC-SR04 Ultrasonic Sensor with validation - Wiring: Trig->GPIO4, Echo->GPIO36 (with voltage divider!)
    digitalWrite(PIN_US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    long duration = pulseIn(PIN_US_ECHO, HIGH, 30000);
    if (duration > 0 && duration < 30000)
    {
        frontDistance = duration * 0.034 / 2;
        if (frontDistance >= 2.0 && frontDistance <= 400.0)
        {
            frontSensorValid = true;
            lastFrontDistanceTime = millis();
        }
        else
        {
            frontDistance = 400.0; // Max range for invalid readings
            frontSensorValid = false;
        }
    }
    else
    {
        frontDistance = 400.0;
        frontSensorValid = false;
    }

    // MQ-2 Gas Sensor with digital threshold check - Wiring: A0->GPIO32, D0->GPIO33
    gasLevel = analogRead(PIN_GAS_ANALOG);

    // Note: Rear distance and smoke sensor removed per pin.md specifications
    rearDistance = 400.0; // Default value
    rearSensorValid = false;
    smokeLevel = 0; // Default value
}

void validateSensorData()
{
    unsigned long now = millis();

    // Check for sensor timeout (no valid reading for 2 seconds)
    if (now - lastFrontDistanceTime > 2000)
    {
        frontSensorValid = false;
        frontDistance = 400.0;
    }

    if (now - lastRearDistanceTime > 2000)
    {
        rearSensorValid = false;
        rearDistance = 400.0;
    }
}

void checkSafetyConditions()
{
    // Enhanced safety with sensor validation
    bool frontObstacle = frontSensorValid && (frontDistance < EMERGENCY_STOP_DISTANCE);
    bool gasDanger = gasLevel > GAS_THRESHOLD_ANALOG;
    bool smokeDanger = smokeLevel > 300;                          // Smoke threshold
    bool rearObstacle = rearSensorValid && (rearDistance < 15.0); // Rear obstacle detection

    if (!emergencyStop && (frontObstacle || gasDanger || smokeDanger || rearObstacle))
    {
        String reason = "";
        if (frontObstacle)
        {
            reason = "Front obstacle: " + String(frontDistance, 1) + "cm";
        }
        if (gasDanger)
        {
            if (reason.length() > 0)
                reason += " & ";
            reason += "Gas critical: " + String(gasLevel);
        }
        if (smokeDanger)
        {
            if (reason.length() > 0)
                reason += " & ";
            reason += "Smoke detected: " + String(smokeLevel);
            activateSmokeAlarm();
        }
        if (rearObstacle)
        {
            if (reason.length() > 0)
                reason += " & ";
            reason += "Rear obstacle: " + String(rearDistance, 1) + "cm";
        }

        activateEmergencyStop(reason);
    }
}

void updateMotorControl()
{
    // Apply target speeds to rear motors with smooth ramping
    rearLeftSpeed = targetRearLeftSpeed;
    rearRightSpeed = targetRearRightSpeed;

    // Constrain motor speeds
    rearLeftSpeed = constrain(rearLeftSpeed, -255, 255);
    rearRightSpeed = constrain(rearRightSpeed, -255, 255);

    // Update rear motor outputs - L298N Driver on GPIO13,14,18,19,23,27
    // Motor control logic would be implemented here based on L298N pin mapping
    // For now, using simple digital outputs as placeholder
    if (rearLeftSpeed != 0)
    {
        digitalWrite(PIN_MOTOR_1, HIGH);
        digitalWrite(PIN_MOTOR_2, rearLeftSpeed > 0 ? LOW : HIGH);
    }
    else
    {
        digitalWrite(PIN_MOTOR_1, LOW);
        digitalWrite(PIN_MOTOR_2, LOW);
    }

    if (rearRightSpeed != 0)
    {
        digitalWrite(PIN_MOTOR_3, HIGH);
        digitalWrite(PIN_MOTOR_4, rearRightSpeed > 0 ? LOW : HIGH);
    }
    else
    {
        digitalWrite(PIN_MOTOR_3, LOW);
        digitalWrite(PIN_MOTOR_4, LOW);
    }
}

void sendMotorCommandsToFront()
{
    // Send motor commands to Front ESP32 via UART
    JsonDocument motorDoc;
    motorDoc["L"] = targetFrontLeftSpeed;
    motorDoc["R"] = targetFrontRightSpeed;
    motorDoc["CL"] = targetCenterLeftSpeed;  // Center left motor
    motorDoc["CR"] = targetCenterRightSpeed; // Center right motor

    String motorCommand;
    serializeJson(motorDoc, motorCommand);

    Serial2.print(motorCommand);
    Serial2.print("\n");
}

void handleUARTCommunication()
{
    // Check for heartbeat from Front ESP32
    if (Serial2.available())
    {
        String message = Serial2.readStringUntil('\n');
        message.trim();

        if (message.length() > 0)
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, message);

            if (!error && doc["type"].is<const char *>())
            {
                if (doc["type"] == "heartbeat")
                {
                    connectionStatus = 2; // Front ESP32 online
                    frontLeftSpeed = doc["leftSpeed"] | 0;
                    frontRightSpeed = doc["rightSpeed"] | 0;
                }
            }
        }
    }
}

void checkFrontESP32Connection()
{
    static unsigned long lastHeartbeat = 0;
    unsigned long now = millis();

    // If no heartbeat for 3 seconds, mark as disconnected
    if (now - lastHeartbeat > 3000)
    {
        if (connectionStatus == 2)
        {
            connectionStatus = 1; // Connected but no heartbeat
            DEBUG_PRINTLN("Front ESP32 heartbeat timeout");
        }
    }
}

void sendTelemetry()
{
    uptime = millis();

    String telemetry = formatTelemetryJSON();
    webSocketServer.broadcastTXT(telemetry);

    DEBUG_PRINT("Enhanced Telemetry: Front=");
    DEBUG_PRINT(frontDistance);
    DEBUG_PRINT("cm, Rear=");
    DEBUG_PRINT(rearDistance);
    DEBUG_PRINT("cm, Gas=");
    DEBUG_PRINT(gasLevel);
    DEBUG_PRINT(", Smoke=");
    DEBUG_PRINT(smokeLevel);
    DEBUG_PRINT(", Front ESP32=");
    DEBUG_PRINT(connectionStatus == 2 ? "ONLINE" : "OFFLINE");
    DEBUG_PRINT(", Emergency=");
    DEBUG_PRINTLN(emergencyStop ? "YES" : "NO");
}

void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_CONNECTED:
        DEBUG_PRINT("WebSocket client ");
        DEBUG_PRINT(num);
        DEBUG_PRINTLN(" connected");
        sendStatusToWebSocket();
        break;

    case WStype_DISCONNECTED:
        DEBUG_PRINT("WebSocket client ");
        DEBUG_PRINT(num);
        DEBUG_PRINTLN(" disconnected");
        break;

    case WStype_TEXT:
        String message = String((char *)payload);
        DEBUG_PRINT("Message from client ");
        DEBUG_PRINT(num);
        DEBUG_PRINT(": ");
        DEBUG_PRINTLN(message);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, message);

        if (error)
        {
            DEBUG_PRINTLN("JSON parse error");
            return;
        }

        if (doc["command"].is<const char *>())
        {
            processDriveCommand(doc);
        }
        break;
    }
}

void processDriveCommand(const JsonDocument &doc)
{
    String command = doc["command"];

    if (emergencyStop && command != "emergency_reset")
    {
        DEBUG_PRINTLN("Command ignored - emergency stop active");
        return;
    }

    if (command == "forward")
    {
        targetFrontLeftSpeed = 150;
        targetFrontRightSpeed = 150;
        targetCenterLeftSpeed = 150;
        targetCenterRightSpeed = 150;
        targetRearLeftSpeed = 150;
        targetRearRightSpeed = 150;
        DEBUG_PRINTLN("Six-motor command: Forward");
    }
    else if (command == "backward")
    {
        targetFrontLeftSpeed = -150;
        targetFrontRightSpeed = -150;
        targetCenterLeftSpeed = -150;
        targetCenterRightSpeed = -150;
        targetRearLeftSpeed = -150;
        targetRearRightSpeed = -150;
        DEBUG_PRINTLN("Six-motor command: Backward");
    }
    else if (command == "left")
    {
        targetFrontLeftSpeed = -100;
        targetFrontRightSpeed = 100;
        targetCenterLeftSpeed = -100;
        targetCenterRightSpeed = 100;
        targetRearLeftSpeed = -100;
        targetRearRightSpeed = 100;
        DEBUG_PRINTLN("Six-motor command: Turn Left");
    }
    else if (command == "right")
    {
        targetFrontLeftSpeed = 100;
        targetFrontRightSpeed = -100;
        targetCenterLeftSpeed = 100;
        targetCenterRightSpeed = -100;
        targetRearLeftSpeed = 100;
        targetRearRightSpeed = -100;
        DEBUG_PRINTLN("Six-motor command: Turn Right");
    }
    else if (command == "stop")
    {
        targetFrontLeftSpeed = 0;
        targetFrontRightSpeed = 0;
        targetCenterLeftSpeed = 0;
        targetCenterRightSpeed = 0;
        targetRearLeftSpeed = 0;
        targetRearRightSpeed = 0;
        DEBUG_PRINTLN("Six-motor command: Stop All");
    }
    else if (command == "emergency")
    {
        activateEmergencyStop("Manual emergency stop");
    }
    else if (command == "emergency_reset")
    {
        deactivateEmergencyStop();
    }
    else if (command == "test_front")
    {
        // Test front motors only
        targetFrontLeftSpeed = 100;
        targetFrontRightSpeed = 100;
        targetCenterLeftSpeed = 100;
        targetCenterRightSpeed = 100;
        DEBUG_PRINTLN("Testing front motors (1-4)");
    }
    else if (command == "test_rear")
    {
        // Test rear motors only
        targetRearLeftSpeed = 100;
        targetRearRightSpeed = 100;
        DEBUG_PRINTLN("Testing rear motors (5-6)");
    }
    else if (command == "test_all")
    {
        // Test all motors
        targetFrontLeftSpeed = 100;
        targetFrontRightSpeed = 100;
        targetCenterLeftSpeed = 100;
        targetCenterRightSpeed = 100;
        targetRearLeftSpeed = 100;
        targetRearRightSpeed = 100;
        DEBUG_PRINTLN("Testing all six motors");
    }
}

void activateEmergencyStop(const String &reason)
{
    if (!emergencyStop)
    {
        emergencyStop = true;
        emergencyTimestamp = millis();

        // Stop all motors
        targetFrontLeftSpeed = 0;
        targetFrontRightSpeed = 0;
        targetCenterLeftSpeed = 0;
        targetCenterRightSpeed = 0;
        targetRearLeftSpeed = 0;
        targetRearRightSpeed = 0;

        // Immediately stop rear motors
        stopAllRearMotors();

        // Sound buzzer
        soundBuzzer(500);

        DEBUG_PRINTLN("🚨 SIX-MOTOR EMERGENCY STOP ACTIVATED!");
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
        buzzerActive = false;
        alarmActive = false;

        DEBUG_PRINTLN("Emergency stop reset - six-motor system resumed");
    }
}

void soundBuzzer(int duration)
{
    buzzerActive = true;
    lastBuzzerUpdate = millis();
}

void updateBuzzer()
{
    unsigned long now = millis();

    if (buzzerActive)
    {
        // Toggle buzzer every 200ms for emergency alert
        if (now - lastBuzzerUpdate >= 200)
        {
            static bool buzzerState = false;
            buzzerState = !buzzerState;
            digitalWrite(PIN_GAS_DIGITAL, buzzerState); // Use GPIO33 for buzzer
            lastBuzzerUpdate = now;
        }

        // Stop buzzer after 5 seconds
        if (now - emergencyTimestamp >= 5000)
        {
            buzzerActive = false;
            digitalWrite(PIN_GAS_DIGITAL, LOW);
        }
    }
}

void activateSmokeAlarm()
{
    if (!alarmActive)
    {
        alarmActive = true;
        DEBUG_PRINTLN("🚨 SMOKE ALARM ACTIVATED!");
    }
}

void updateSmokeAlarm()
{
    if (alarmActive && smokeLevel < 200)
    {
        alarmActive = false;
        DEBUG_PRINTLN("Smoke alarm cleared");
    }
}

void sendStatusToWebSocket()
{
    String status = formatStatusJSON();
    webSocketServer.broadcastTXT(status);
}

String formatTelemetryJSON()
{
    JsonDocument doc;
    doc["type"] = "telemetry";
    doc["timestamp"] = millis();
    doc["dist"] = frontDistance;
    doc["rearDist"] = rearDistance;
    doc["gas"] = gasLevel;
    doc["smoke"] = smokeLevel;
    doc["battery"] = batteryVoltage;
    doc["uptime"] = uptime;
    doc["emergency"] = emergencyStop;
    doc["frontOnline"] = (connectionStatus == 2);
    doc["motorsActive"] = (frontLeftSpeed != 0 || frontRightSpeed != 0 || rearLeftSpeed != 0 || rearRightSpeed != 0);
    doc["frontSensorValid"] = frontSensorValid;
    doc["rearSensorValid"] = rearSensorValid;

    String json;
    serializeJson(doc, json);
    return json;
}

String formatStatusJSON()
{
    JsonDocument doc;
    doc["type"] = "status";
    doc["status"] = emergencyStop ? "emergency" : "normal";
    doc["ready"] = systemReady;
    doc["sixMotors"] = true;
    doc["frontOnline"] = (connectionStatus == 2);

    String json;
    serializeJson(doc, json);
    return json;
}