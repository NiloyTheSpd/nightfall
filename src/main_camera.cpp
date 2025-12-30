/**
 * @file    main_camera.cpp
 * @brief   Project Nightfall - Phase 2 MVP CAMERA ESP32 (Telemetry Node)
 * @author  Project Nightfall Team
 * @version 2.0.0
 * @date    December 29, 2025
 *
 * Telemetry node for the autonomous rescue robot. Handles:
 * - WiFi client connection to "ProjectNightfall" AP
 * - WebSocket connection to master ESP32 (192.168.4.1:8888)
 * - Heartbeat telemetry every 5 seconds
 * - Flash LED (pin 4) and Status LED (pin 33) control
 * - Non-blocking operation with millis() timers
 * - Robust JSON communication with ArduinoJson v7
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// Define controller type for conditional compilation BEFORE including pins.h
#define CAMERA_MODULE

// Include our libraries
#include "config.h"
#include "pins.h"

// Global objects
WebSocketsClient webSocket;

// System state
bool systemReady = false;
bool wifiConnected = false;
bool webSocketConnected = false;
unsigned long lastHeartbeat = 0;
unsigned long lastConnectionCheck = 0;
unsigned long lastLEDUpdate = 0;
bool flashLEDState = false;
bool statusLEDState = false;

// WiFi configuration for Test Version 2
const char *ssid = "ProjectNightfall";
const char *password = "rescue2025";
const unsigned long wifi_connect_timeout_ms = 10000;

// WebSocket configuration
const char *websocket_host = "192.168.4.1";
const uint16_t websocket_port = 8888;

// Camera streaming configuration
const uint16_t camera_stream_port = 81;
const uint16_t camera_capture_port = 82;

// Function declarations
void setup();
void loop();
void initializeHardware();
void setupWiFi();
void startFallbackAP();
void setupWebSocket();
void handleMainLoop();
void checkConnections();
void sendHeartbeat();
void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
void updateLEDs();
void connectToWiFi();
void connectToWebSocket();
void handleSerialCommands();

// Setup function
void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("╔═══════════════════════════════════════════╗");
    DEBUG_PRINTLN("║     PROJECT NIGHTFALL CAMERA ESP32        ║");
    DEBUG_PRINTLN("║            Telemetry Node                 ║");
    DEBUG_PRINTLN("║              Version 2.0.0                ║");
    DEBUG_PRINTLN("╚═══════════════════════════════════════════╝");
    DEBUG_PRINTLN();

    // Initialize hardware
    initializeHardware();

    // Setup WiFi and WebSocket
    setupWiFi();
    setupWebSocket();

    systemReady = true;
    lastHeartbeat = millis();
    lastConnectionCheck = millis();
    lastLEDUpdate = millis();

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("✅ CAMERA ESP32 Telemetry Node Ready!");
    DEBUG_PRINT("WiFi SSID: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("WebSocket Host: ");
    DEBUG_PRINT(websocket_host);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(websocket_port);
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

    // Initialize LED pins per pin.md specifications
    pinMode(4, OUTPUT);              // Flash LED
    pinMode(PIN_STATUS_LED, OUTPUT); // Status LED (GPIO33 - LOW=ON per pin.md)

    // Turn off LEDs initially
    digitalWrite(4, LOW);
    digitalWrite(PIN_STATUS_LED, LOW); // LOW=ON for status LED

    DEBUG_PRINTLN("Hardware initialized");
    DEBUG_PRINTLN("Flash LED: Pin 4");
    DEBUG_PRINTLN("Status LED: GPIO33 (LOW=ON)");
}

void setupWiFi()
{
    DEBUG_PRINTLN("Setting up WiFi client connection...");

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    DEBUG_PRINT("Connecting to WiFi");

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < wifi_connect_timeout_ms))
    {
        delay(500);
        DEBUG_PRINT(".");

        // Flash status LED while connecting (GPIO33 - LOW=ON)
        static unsigned long lastFlash = 0;
        if (millis() - lastFlash > 200)
        {
            statusLEDState = !statusLEDState;
            digitalWrite(PIN_STATUS_LED, statusLEDState ? HIGH : LOW);
            lastFlash = millis();
        }
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

        // Turn on status LED to indicate successful connection (GPIO33 - LOW=ON)
        digitalWrite(PIN_STATUS_LED, LOW); // LOW turns LED on
    }
    else
    {
        wifiConnected = false;
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("WiFi connection failed! (STA only)");

        // Blink status LED to indicate error (GPIO33 - LOW=ON)
        for (int i = 0; i < 6; i++)
        {
            digitalWrite(PIN_STATUS_LED, LOW); // LED on
            delay(100);
            digitalWrite(PIN_STATUS_LED, HIGH); // LED off
            delay(100);
        }
    }
}

void setupWebSocket()
{
    DEBUG_PRINTLN("Setting up WebSocket client...");

    // Configure WebSocket
    webSocket.begin(websocket_host, websocket_port);
    webSocket.onEvent(handleWebSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void handleMainLoop()
{
    unsigned long now = millis();

    // Check connections periodically
    if (now - lastConnectionCheck >= 2000)
    {
        checkConnections();
        lastConnectionCheck = now;
    }

    // Send heartbeat every 5 seconds
    if (now - lastHeartbeat >= CAMERA_HEARTBEAT_INTERVAL)
    {
        sendHeartbeat();
        lastHeartbeat = now;
    }

    // Update LEDs
    updateLEDs();

    // Handle WebSocket
    webSocket.loop();

    // Handle serial commands
    handleSerialCommands();
}

void checkConnections()
{
    // Check WiFi connection
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!wifiConnected)
        {
            wifiConnected = true;
            DEBUG_PRINTLN("WiFi reconnected");
        }
    }
    else
    {
        if (wifiConnected)
        {
            wifiConnected = false;
            webSocketConnected = false;
            DEBUG_PRINTLN("WiFi disconnected");
        }

        // Try to reconnect WiFi
        connectToWiFi();
    }

    // Check WebSocket connection
    if (wifiConnected && !webSocketConnected)
    {
        connectToWebSocket();
    }
}

void connectToWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN("Attempting WiFi reconnection...");
        WiFi.reconnect();

        // Wait a bit for connection
        delay(1000);
    }
}

void connectToWebSocket()
{
    if (wifiConnected && !webSocketConnected)
    {
        DEBUG_PRINTLN("Attempting WebSocket connection...");
        webSocket.begin(websocket_host, websocket_port);
    }
}

void sendHeartbeat()
{
    if (webSocketConnected)
    {
        // Create heartbeat JSON
        JsonDocument heartbeatDoc;
        heartbeatDoc["type"] = "heartbeat";
        heartbeatDoc["source"] = "camera";
        heartbeatDoc["timestamp"] = millis();
        heartbeatDoc["uptime"] = millis();
        heartbeatDoc["wifi_rssi"] = WiFi.RSSI();
        heartbeatDoc["wifi_ip"] = WiFi.localIP().toString();
        heartbeatDoc["memory_free"] = ESP.getFreeHeap();
        heartbeatDoc["boot_count"] = 1;
        heartbeatDoc["version"] = VERSION_STRING;

        String heartbeat;
        serializeJson(heartbeatDoc, heartbeat);

        // Send heartbeat via WebSocket
        webSocket.sendTXT(heartbeat);

        DEBUG_PRINT("Heartbeat sent - WiFi RSSI: ");
        DEBUG_PRINT(WiFi.RSSI());
        DEBUG_PRINT(" dBm, Free Memory: ");
        DEBUG_PRINT(ESP.getFreeHeap());
        DEBUG_PRINTLN(" bytes");
    }
    else
    {
        DEBUG_PRINTLN("Cannot send heartbeat - WebSocket not connected");
    }
}

void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    String message;
    JsonDocument doc;
    DeserializationError error;
    String command;

    switch (type)
    {
    case WStype_CONNECTED:
        webSocketConnected = true;
        DEBUG_PRINTLN("WebSocket connected to master ESP32");

        // Turn on status LED solid to indicate WebSocket connection (GPIO33 - LOW=ON)
        digitalWrite(PIN_STATUS_LED, LOW); // LOW turns LED on

        // Send immediate heartbeat upon connection
        sendHeartbeat();
        break;

    case WStype_DISCONNECTED:
        webSocketConnected = false;
        DEBUG_PRINTLN("WebSocket disconnected from master ESP32");

        // Blink status LED to indicate disconnection
        statusLEDState = false;
        break;

    case WStype_TEXT:
        message = String((char *)payload);
        DEBUG_PRINT("WebSocket message received: ");
        DEBUG_PRINTLN(message);

        // Parse incoming messages (if any)
        error = deserializeJson(doc, message);

        if (!error)
        {
            if (doc["command"].is<String>())
            {
                command = doc["command"].as<String>();

                if (command == "flash")
                {
                    // Flash LED command
                    flashLEDState = true;
                    digitalWrite(4, HIGH);
                    DEBUG_PRINTLN("Flash LED activated");
                }
                else if (command == "status")
                {
                    // Send status update
                    sendHeartbeat();
                }
            }
        }
        break;

    case WStype_ERROR:
        DEBUG_PRINTLN("WebSocket error occurred");
        webSocketConnected = false;
        break;

    default:
        break;
    }
}

void updateLEDs()
{
    unsigned long now = millis();

    if (flashLEDState && (now - lastLEDUpdate >= 500))
    {
        flashLEDState = false;
        digitalWrite(4, LOW);
    }

    // Update Status LED pattern (GPIO33 - LOW=ON)
    if (!wifiConnected)
    {
        // Blink red if WiFi not connected
        if (now - lastLEDUpdate >= 500)
        {
            statusLEDState = !statusLEDState;
            digitalWrite(PIN_STATUS_LED, statusLEDState ? HIGH : LOW);
            lastLEDUpdate = now;
        }
    }
    else if (wifiConnected && !webSocketConnected)
    {
        // Fast blink if WiFi connected but WebSocket not connected
        if (now - lastLEDUpdate >= 200)
        {
            statusLEDState = !statusLEDState;
            digitalWrite(PIN_STATUS_LED, statusLEDState ? HIGH : LOW);
            lastLEDUpdate = now;
        }
    }
    else if (webSocketConnected)
    {
        // Solid green if fully connected (LOW=ON)
        digitalWrite(PIN_STATUS_LED, LOW);
    }
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
            Serial.println("=== CAMERA ESP32 STATUS ===");
            Serial.print("Uptime: ");
            Serial.print(millis() / 1000);
            Serial.println(" seconds");
            Serial.print("WiFi Connected: ");
            Serial.println(wifiConnected ? "YES" : "NO");
            Serial.print("WiFi IP: ");
            Serial.println(WiFi.localIP());
            Serial.print("WiFi RSSI: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
            Serial.print("WebSocket Connected: ");
            Serial.println(webSocketConnected ? "YES" : "NO");
            Serial.print("Free Memory: ");
            Serial.print(ESP.getFreeHeap());
            Serial.println(" bytes");
            Serial.print("Flash LED: ");
            Serial.println(digitalRead(4) ? "ON" : "OFF");
            Serial.print("Status LED: ");
            Serial.println(digitalRead(PIN_STATUS_LED) ? "OFF" : "ON"); // GPIO33 is LOW=ON
            Serial.print("Last Heartbeat: ");
            Serial.print((millis() - lastHeartbeat) / 1000);
            Serial.println(" seconds ago");
            Serial.println("==========================");
        }
        else if (command == "wifi")
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("WiFi Status: Connected");
                Serial.print("IP: ");
                Serial.println(WiFi.localIP());
                Serial.print("RSSI: ");
                Serial.print(WiFi.RSSI());
                Serial.println(" dBm");
            }
            else
            {
                Serial.println("WiFi Status: Disconnected");
            }
        }
        else if (command == "websocket")
        {
            Serial.print("WebSocket Status: ");
            Serial.println(webSocketConnected ? "Connected" : "Disconnected");
            Serial.print("Host: ");
            Serial.print(websocket_host);
            Serial.print(":");
            Serial.println(websocket_port);
        }
        else if (command == "flash")
        {
            digitalWrite(4, HIGH);
            delay(500);
            digitalWrite(4, LOW);
            Serial.println("Flash LED test");
        }
        else if (command == "heartbeat")
        {
            sendHeartbeat();
            Serial.println("Heartbeat sent manually");
        }
        else if (command == "reconnect")
        {
            DEBUG_PRINTLN("Attempting reconnection...");
            WiFi.reconnect();
            webSocket.begin(websocket_host, websocket_port);
        }
        else if (command == "help")
        {
            Serial.println("Available commands:");
            Serial.println("  status  - Show system status");
            Serial.println("  wifi    - Show WiFi status");
            Serial.println("  websocket - Show WebSocket status");
            Serial.println("  flash   - Test flash LED");
            Serial.println("  heartbeat - Send manual heartbeat");
            Serial.println("  reconnect - Attempt reconnection");
            Serial.println("  help - Show this help");
        }
    }
}
