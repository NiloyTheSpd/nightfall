/**
 * @file    main_rear.cpp
 * @brief   Project Nightfall - Phase 2 MVP BACK ESP32 (Master/Brain)
 * @author  Project Nightfall Team
 * @version 2.0.0
 * @date    December 29, 2025
 *
 * Master controller for the manual rescue robot. Handles:
 * - WiFi Access Point creation (ProjectNightfall/rescue2025)
 * - WebSocket Server on port 8888 for Dashboard communication
 * - Safety override system (Distance < 20cm OR Gas/Smoke > threshold)
 * - Telemetry broadcasting every 500ms
 * - Motor control and UART communication to Front ESP32
 * - Sensor monitoring (2x Ultrasonic + Gas + Smoke Sensors)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESPAsyncWebServer.h>

// Include our libraries
#include "config.h"

// Global objects
AsyncWebServer webServer(80);
WebSocketsServer webSocketServer(8888);

// System state
unsigned long lastSensorUpdate = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastWebUpdate = 0;
bool systemReady = false;

// Safety system
bool emergencyStop = false;
unsigned long emergencyTimestamp = 0;
bool buzzerActive = false;
unsigned long lastBuzzerUpdate = 0;

// Motor control state
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;
int targetLeftSpeed = 0;
int targetRightSpeed = 0;

// Telemetry data
float frontDistance = 0.0;
float rearDistance = 0.0;
int gasLevel = 0;
int smokeLevel = 0;
float batteryVoltage = 14.8;
unsigned long uptime = 0;

// WiFi configuration
const char *ssid = "ProjectNightfall";
const char *password = "rescue2025";

// Function declarations
void setup();
void loop();
void initializeHardware();
void setupWiFi();
void setupWebServer();
void handleMainLoop();
void updateSensors();
void checkSafetyConditions();
void updateMotorControl();
void sendTelemetry();
void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void processDriveCommand(const JsonDocument &doc);
void activateEmergencyStop(const String &reason);
void deactivateEmergencyStop();
void soundBuzzer(int duration);
void updateBuzzer();
void sendStatusToWebSocket();
String formatTelemetryJSON();
String formatStatusJSON();

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL BACK ESP32          ║");
    DEBUG_PRINTLN("║           Master/Brain Controller         ║");
    DEBUG_PRINTLN("║              Version 2.0.0                ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Setup WiFi and WebServer
    setupWiFi();
    setupWebServer();

    systemReady = true;
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ BACK ESP32 Master Controller Ready!");
    DEBUG_PRINT("WiFi AP: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("WebSocket Server: Port 8888");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Dashboard URL: http://192.168.4.1");
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
    DEBUG_PRINTLN("Initializing hardware...");

    // Initialize motor control pins (L298N driver)
    pinMode(13, OUTPUT); // Motor PWM 1
    pinMode(14, OUTPUT); // Motor IN1
    pinMode(18, OUTPUT); // Motor IN2
    pinMode(19, OUTPUT); // Motor IN3
    pinMode(23, OUTPUT); // Motor IN4
    pinMode(27, OUTPUT); // Motor PWM 2

    // Initialize sensor pins
    pinMode(4, OUTPUT);  // HC-SR04 Trig
    pinMode(36, INPUT);  // HC-SR04 Echo (with voltage divider!)
    pinMode(32, INPUT);  // MQ-2 Gas Sensor Analog (A0)
    pinMode(33, OUTPUT); // Buzzer / MQ-2 Digital (shared)

    // Initialize UART pins (Serial2 - hardware fixed GPIO16/17)
    pinMode(17, OUTPUT); // UART TX2
    pinMode(16, INPUT);  // UART RX2

    // Initialize Serial2 for UART communication
    Serial2.begin(UART_BAUDRATE, SERIAL_8N1, 16, 17); // RX=16, TX=17

    // Stop all motors initially
    analogWrite(13, 0);
    analogWrite(27, 0);
    digitalWrite(14, LOW);
    digitalWrite(18, LOW);
    digitalWrite(19, LOW);
    digitalWrite(23, LOW);
    digitalWrite(33, LOW); // Buzzer off

    DEBUG_PRINTLN("Hardware initialized");
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

    // Serve main dashboard - using String to avoid async issues
    String index_html = R"(<!DOCTYPE html>
<html><head><title>Project Nightfall - Robot Control</title>
<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>
<style>
body{font-family:Arial;margin:20px;background:#1a1a1a;color:#fff}
.container{max-width:800px;margin:0 auto}
.card{background:#2d2d2d;border-radius:10px;padding:20px;margin:10px 0}
.status{display:inline-block;padding:5px 15px;border-radius:20px;margin:5px}
.normal{background:#28a745}.warning{background:#ffc107;color:#000}.error{background:#dc3545}
.button{background:#007bff;color:white;border:none;padding:10px 20px;margin:5px;border-radius:5px;cursor:pointer}
.button:hover{background:#0056b3}
.button.emergency{background:#dc3545}
.button.emergency:hover{background:#c82333}
.telemetry{font-size:1.2em;margin:10px 0}
</style></head>
<body><div class='container'>
<h1>🤖 Project Nightfall - Rescue Robot Control</h1>
<div class='card'><h2>System Status</h2>
<div class='telemetry'>Robot State: <span id='robotState' class='status normal'>READY</span></div>
<div class='telemetry'>Uptime: <span id='uptime'>0s</span></div>
<div class='telemetry'>Emergency: <span id='emergency' class='status normal'>NO</span></div></div>
<div class='card'><h2>Sensor Data</h2>
<div class='telemetry'>Front Distance: <span id='frontDistance'>0</span> cm</div>
<div class='telemetry'>Rear Distance: <span id='rearDistance'>0</span> cm</div>
<div class='telemetry'>Gas Level: <span id='gasLevel'>0</span></div>
<div class='telemetry'>Smoke Level: <span id='smokeLevel'>0</span></div>
<div class='telemetry'>Battery: <span id='battery'>14.8</span> V</div></div>
<div class='card'><h2>Manual Control</h2>
<button class='button' onclick='sendCommand("forward")'>⬆️ Forward</button>
<button class='button' onclick='sendCommand("left")'>⬅️ Turn Left</button>
<button class='button' onclick='sendCommand("right")'>➡️ Turn Right</button>
<button class='button' onclick='sendCommand("backward")'>⬇️ Backward</button>
<button class='button' onclick='sendCommand("stop")'>⏹️ Stop</button>
<button class='button emergency' onclick='sendCommand("emergency")'>🚨 Emergency Stop</button></div>

</div>
<script>
var ws = new WebSocket('ws://192.168.4.1:8888');
ws.onopen = function(){console.log('Connected to robot');};
ws.onmessage = function(event){
var data = JSON.parse(event.data);
if(data.type === 'telemetry'){
document.getElementById('frontDistance').textContent = data.dist;
document.getElementById('rearDistance').textContent = data.rearDist;
document.getElementById('gasLevel').textContent = data.gas;
document.getElementById('smokeLevel').textContent = data.smoke;
document.getElementById('battery').textContent = data.battery;
document.getElementById('uptime').textContent = Math.floor(data.uptime/1000) + 's';
document.getElementById('emergency').textContent = data.emergency ? 'YES' : 'NO';
document.getElementById('emergency').className = 'status ' + (data.emergency ? 'error' : 'normal');
}
};
function sendCommand(cmd){ws.send(JSON.stringify({command: cmd}));}
</script></body></html>)";

    webServer.on("/", HTTP_GET, [&index_html](AsyncWebServerRequest *request)
                 { request->send(200, "text/html", index_html); });

    // API endpoints
    webServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
        DynamicJsonDocument doc(512);
        doc["status"] = "online";
        doc["version"] = VERSION_STRING;
        doc["uptime"] = millis();
        doc["emergency"] = emergencyStop;
        doc["frontDistance"] = frontDistance;
        doc["rearDistance"] = rearDistance;
        doc["gasLevel"] = gasLevel;
        doc["battery"] = batteryVoltage;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    webServer.on("/api/telemetry", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
        String telemetry = formatTelemetryJSON();
        request->send(200, "application/json", telemetry); });

    // Start web server
    webServer.begin();

    // Start WebSocket server
    webSocketServer.begin();
    webSocketServer.onEvent(handleWebSocketEvent);

    DEBUG_PRINTLN("Web server started on port 80");
    DEBUG_PRINTLN("WebSocket server started on port 8888");
}

void handleMainLoop()
{
    unsigned long now = millis();

    // Handle WebSocket
    webSocketServer.loop();
    // Note: AsyncWebServer is event-driven and doesn't need handleClient()

    // Update sensors every 100ms
    if (now - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL)
    {
        updateSensors();
        checkSafetyConditions();
        lastSensorUpdate = now;
    }

    // Send telemetry every 500ms
    if (now - lastTelemetryUpdate >= TELEMETRY_INTERVAL)
    {
        sendTelemetry();
        lastTelemetryUpdate = now;
    }

    // Update motor control every 50ms
    updateMotorControl();

    // Update buzzer
    updateBuzzer();
}

void updateSensors()
{
    // Update Front Ultrasonic Sensor (HC-SR04)
    digitalWrite(4, LOW); // GPIO4 = Trig
    delayMicroseconds(2);
    digitalWrite(4, HIGH);
    delayMicroseconds(10);
    digitalWrite(4, LOW);

    long duration = pulseIn(36, HIGH, 30000); // GPIO36 = Echo, 30ms timeout
    if (duration > 0)
    {
        frontDistance = duration * 0.034 / 2;
        if (frontDistance > 400)
            frontDistance = 400; // Max range
    }
    else
    {
        frontDistance = 400; // No echo = max range
    }

    // Rear ultrasonic not installed - set to safe default
    rearDistance = 400;

    // Update Gas Sensor (MQ-2)
    gasLevel = analogRead(32); // GPIO32 = MQ-2 A0
}

void checkSafetyConditions()
{
    // SAFETY OVERRIDE: If (FrontDistance < 20cm) OR (Gas > threshold), MUST override commands
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
                reason += " | ";
            reason += "Gas detected: " + String(gasLevel);
        }
        reason += " & ";
        reason += "Gas level critical: " + String(gasLevel);
    }
    if (smokeLevel > 300)
    {
        if (reason.length() > 0)
            reason += " & ";
        reason += "Smoke detected: " + String(smokeLevel);
    }

    activateEmergencyStop(reason);
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

    // Update motor outputs
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
    StaticJsonDocument<256> motorDoc;
    motorDoc["L"] = leftMotorSpeed;
    motorDoc["R"] = rightMotorSpeed;

    String motorCommand;
    serializeJson(motorDoc, motorCommand);

    // Send via Serial (for debugging) and UART (to Front ESP32)
    Serial2.print(motorCommand);
    Serial2.print("\n");
}

void sendTelemetry()
{
    uptime = millis();

    String telemetry = formatTelemetryJSON();
    webSocketServer.broadcastTXT(telemetry);

    DEBUG_PRINT("Telemetry: dist=");
    DEBUG_PRINT(frontDistance);
    DEBUG_PRINT("cm, gas=");
    DEBUG_PRINT(gasLevel);
    DEBUG_PRINT(", emergency=");
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
        // Send current status to newly connected client
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

        // Parse JSON command
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, message);

        if (error)
        {
            DEBUG_PRINTLN("JSON parse error");
            return;
        }

        if (doc.containsKey("command"))
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
        targetLeftSpeed = 150;
        targetRightSpeed = 150;
        DEBUG_PRINTLN("Command: Forward");
    }
    else if (command == "backward")
    {
        targetLeftSpeed = -150;
        targetRightSpeed = -150;
        DEBUG_PRINTLN("Command: Backward");
    }
    else if (command == "left")
    {
        targetLeftSpeed = -100;
        targetRightSpeed = 100;
        DEBUG_PRINTLN("Command: Turn Left");
    }
    else if (command == "right")
    {
        targetLeftSpeed = 100;
        targetRightSpeed = -100;
        DEBUG_PRINTLN("Command: Turn Right");
    }
    else if (command == "stop")
    {
        targetLeftSpeed = 0;
        targetRightSpeed = 0;
        DEBUG_PRINTLN("Command: Stop");
    }
    else if (command == "emergency")
    {
        activateEmergencyStop("Manual emergency stop");
    }
    else if (command == "emergency_reset")
    {
        deactivateEmergencyStop();
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

        // Sound buzzer
        soundBuzzer(500);

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
        buzzerActive = false;

        DEBUG_PRINTLN("Emergency stop reset - system resumed");
    }
}

void soundBuzzer(int duration)
{
    buzzerActive = true;
    lastBuzzerUpdate = millis();

    // Buzzer will be handled in updateBuzzer()
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
            digitalWrite(4, buzzerState);
            lastBuzzerUpdate = now;
        }

        // Stop buzzer after 5 seconds
        if (now - emergencyTimestamp >= 5000)
        {
            buzzerActive = false;
            digitalWrite(4, LOW);
        }
    }
}

void sendStatusToWebSocket()
{
    String status = formatStatusJSON();
    // This would be sent to specific client - for now broadcast
    webSocketServer.broadcastTXT(status);
}

String formatTelemetryJSON()
{
    StaticJsonDocument<512> doc;
    doc["type"] = "telemetry";
    doc["timestamp"] = millis();
    doc["dist"] = frontDistance;
    doc["rearDist"] = rearDistance;
    doc["gas"] = gasLevel;
    doc["smoke"] = smokeLevel;
    doc["battery"] = batteryVoltage;
    doc["uptime"] = uptime;
    doc["emergency"] = emergencyStop;

    String json;
    serializeJson(doc, json);
    return json;
}

String formatStatusJSON()
{
    StaticJsonDocument<256> doc;
    doc["type"] = "status";
    doc["status"] = emergencyStop ? "emergency" : "normal";
    doc["ready"] = systemReady;

    String json;
    serializeJson(doc, json);
    return json;
}