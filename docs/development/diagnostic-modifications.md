# Code Modifications for Enhanced Diagnostics

## Overview

This document outlines specific code modifications to enhance network connectivity diagnostics and troubleshooting capabilities in the Project Nightfall ESP32 system.

## 1. Enhanced Network Diagnostics Library

### 1.1 Create NetworkDiagnostics.h

```cpp
/**
 * @file    NetworkDiagnostics.h
 * @brief   Enhanced network diagnostics for Project Nightfall
 * @author  Project Nightfall Team
 * @version 2.0.0
 */

#ifndef NETWORK_DIAGNOSTICS_H
#define NETWORK_DIAGNOSTICS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

class NetworkDiagnostics
{
public:
    NetworkDiagnostics();

    // WiFi Diagnostics
    struct WiFiStatus {
        bool connected;
        String ssid;
        String ip;
        String mac;
        int32_t rssi;
        uint8_t channel;
        bool ap_mode;
        uint8_t client_count;
    };

    WiFiStatus getWiFiStatus();
    void scanNetworks();
    String getNetworkScanResults();

    // WebSocket Diagnostics
    struct WebSocketStatus {
        bool server_running;
        uint8_t client_count;
        bool client_connected[4];
        String client_ips[4];
        bool client_websockets[4];
    };

    WebSocketStatus getWebSocketStatus();
    String getConnectionHistory();

    // UART Diagnostics
    struct UARTStatus {
        bool available;
        unsigned long last_receive;
        uint32_t baud_rate;
        uint32_t bytes_sent;
        uint32_t bytes_received;
        uint32_t errors;
    };

    UARTStatus getUARTStatus();
    void startPacketCapture(bool enable);
    String getPacketLog();

    // System Diagnostics
    struct SystemStatus {
        uint32_t uptime;
        uint32_t free_heap;
        uint32_t min_heap;
        float cpu_usage;
        uint32_t wifi_errors;
        uint32_t websocket_errors;
        uint32_t uart_errors;
    };

    SystemStatus getSystemStatus();
    void resetCounters();

    // Diagnostic Commands
    void runFullDiagnostic();
    String generateDiagnosticReport();
    void enableVerboseLogging(bool enable);

private:
    bool _verbose_logging;
    uint32_t _packet_count;
    String _packet_log[100];
    uint32_t _wifi_error_count;
    uint32_t _websocket_error_count;
    uint32_t _uart_error_count;
    unsigned long _diagnostic_start_time;
};

#endif // NETWORK_DIAGNOSTICS_H
```

### 1.2 Create NetworkDiagnostics.cpp

```cpp
/**
 * @file    NetworkDiagnostics.cpp
 * @brief   Enhanced network diagnostics implementation
 */

#include "NetworkDiagnostics.h"

NetworkDiagnostics::NetworkDiagnostics()
    : _verbose_logging(false)
    , _packet_count(0)
    , _wifi_error_count(0)
    , _websocket_error_count(0)
    , _uart_error_count(0)
    , _diagnostic_start_time(millis())
{
    for (int i = 0; i < 100; i++) {
        _packet_log[i] = "";
    }
}

NetworkDiagnostics::WiFiStatus NetworkDiagnostics::getWiFiStatus()
{
    WiFiStatus status;

    status.connected = (WiFi.status() == WL_CONNECTED);
    status.ssid = WiFi.SSID();
    status.ip = WiFi.localIP().toString();
    status.mac = WiFi.macAddress();
    status.rssi = WiFi.RSSI();
    status.channel = WiFi.channel();
    status.ap_mode = WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;

    // Get client count if in AP mode
    status.client_count = 0;
    if (status.ap_mode) {
        wifi_sta_list_t sta_list;
        if (esp_wifi_get_sta_list(&sta_list) == ESP_OK) {
            status.client_count = sta_list.num;
        }
    }

    return status;
}

NetworkDiagnostics::WebSocketStatus NetworkDiagnostics::getWebSocketStatus()
{
    WebSocketStatus status;
    status.server_running = true; // Would be set by WebSocket server
    status.client_count = 0;

    // Reset arrays
    for (int i = 0; i < 4; i++) {
        status.client_connected[i] = false;
        status.client_websockets[i] = false;
        status.client_ips[i] = "";
    }

    // This would be populated by actual WebSocket server
    // For now, return structure template

    return status;
}

NetworkDiagnostics::UARTStatus NetworkDiagnostics::getUARTStatus()
{
    UARTStatus status;
    status.available = Serial2.available() > 0;
    status.last_receive = millis(); // Would be updated by UART library
    status.baud_rate = 115200;
    status.bytes_sent = 0; // Would be tracked by UART library
    status.bytes_received = 0; // Would be tracked by UART library
    status.errors = _uart_error_count;

    return status;
}

NetworkDiagnostics::SystemStatus NetworkDiagnostics::getSystemStatus()
{
    SystemStatus status;
    status.uptime = millis() - _diagnostic_start_time;
    status.free_heap = ESP.getFreeHeap();
    status.min_heap = ESP.getMinFreeHeap();
    status.cpu_usage = 0.0; // Would require CPU usage monitoring
    status.wifi_errors = _wifi_error_count;
    status.websocket_errors = _websocket_error_count;
    status.uart_errors = _uart_error_count;

    return status;
}

void NetworkDiagnostics::startPacketCapture(bool enable)
{
    _verbose_logging = enable;
}

String NetworkDiagnostics::generateDiagnosticReport()
{
    String report = "\n=== NETWORK DIAGNOSTIC REPORT ===\n";

    // WiFi Status
    WiFiStatus wifi = getWiFiStatus();
    report += "WiFi Status:\n";
    report += "  Connected: " + String(wifi.connected ? "YES" : "NO") + "\n";
    if (wifi.connected) {
        report += "  SSID: " + wifi.ssid + "\n";
        report += "  IP: " + wifi.ip + "\n";
        report += "  RSSI: " + String(wifi.rssi) + " dBm\n";
        report += "  Channel: " + String(wifi.channel) + "\n";
        if (wifi.ap_mode) {
            report += "  AP Mode - Clients: " + String(wifi.client_count) + "\n";
        }
    }
    report += "\n";

    // System Status
    SystemStatus sys = getSystemStatus();
    report += "System Status:\n";
    report += "  Uptime: " + String(sys.uptime / 1000) + "s\n";
    report += "  Free Heap: " + String(sys.free_heap) + " bytes\n";
    report += "  Min Heap: " + String(sys.min_heap) + " bytes\n";
    report += "  WiFi Errors: " + String(sys.wifi_errors) + "\n";
    report += "  WebSocket Errors: " + String(sys.websocket_errors) + "\n";
    report += "  UART Errors: " + String(sys.uart_errors) + "\n";

    return report;
}

void NetworkDiagnostics::enableVerboseLogging(bool enable)
{
    _verbose_logging = enable;
}
```

## 2. BACK ESP32 Diagnostic Enhancements

### 2.1 Add to main_rear.cpp

```cpp
// Add global diagnostic object
#include "NetworkDiagnostics.h"
NetworkDiagnostics netDiag;

// Add to setup() function after web server setup:
void setup()
{
    // ... existing setup code ...

    // Initialize network diagnostics
    netDiag.enableVerboseLogging(true);

    // Add diagnostic commands
    addDiagnosticCommands();

    // ... rest of setup ...
}

void addDiagnosticCommands()
{
    // Add to handleSerialCommands() function
    else if (command == "diagnostic")
    {
        String report = netDiag.generateDiagnosticReport();
        Serial.println(report);
    }
    else if (command == "wifi_status")
    {
        auto wifi = netDiag.getWiFiStatus();
        Serial.println("WiFi Status:");
        Serial.println("  Connected: " + String(wifi.connected ? "YES" : "NO"));
        if (wifi.connected) {
            Serial.println("  SSID: " + wifi.ssid);
            Serial.println("  IP: " + wifi.ip);
            Serial.println("  RSSI: " + String(wifi.rssi) + " dBm");
        }
    }
    else if (command == "scan_wifi")
    {
        netDiag.scanNetworks();
        Serial.println("WiFi scan completed - check results");
    }
    else if (command == "reset_errors")
    {
        netDiag.resetCounters();
        Serial.println("Error counters reset");
    }
}

// Add to handleMainLoop():
void handleMainLoop()
{
    unsigned long now = millis();

    // Existing code...

    // Add periodic diagnostic update every 30 seconds
    static unsigned long lastDiagnosticUpdate = 0;
    if (now - lastDiagnosticUpdate >= 30000) {
        if (netDiag.getWiFiStatus().connected) {
            DEBUG_PRINTLN("Network OK - WiFi RSSI: " + String(netDiag.getWiFiStatus().rssi));
        }
        lastDiagnosticUpdate = now;
    }
}
```

### 2.2 Enhanced WebSocket Status Tracking

```cpp
// Add to main_rear.cpp - global variables
uint8_t connectedClients[4] = {0};
String clientIPs[4] = {"", "", "", ""};
unsigned long clientConnectTime[4] = {0, 0, 0, 0};

// Modify handleWebSocketEvent to track client info:
void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_CONNECTED:
        connectedClients[num] = 1;
        clientIPs[num] = "Client_" + String(num);
        clientConnectTime[num] = millis();
        DEBUG_PRINT("Client ");
        DEBUG_PRINT(num);
        DEBUG_PRINTLN(" connected");
        break;

    case WStype_DISCONNECTED:
        connectedClients[num] = 0;
        clientIPs[num] = "";
        DEBUG_PRINT("Client ");
        DEBUG_PRINT(num);
        DEBUG_PRINTLN(" disconnected");
        break;

    // ... existing cases ...
    }

    // Add to diagnostic tracking
    if (type == WStype_ERROR) {
        netDiag.incrementWebSocketErrors();
    }
}
```

## 3. FRONT ESP32 Diagnostic Enhancements

### 3.1 Add to main_front.cpp

```cpp
// Add to existing global variables
unsigned long lastUARTReceive = 0;
uint32_t uartBytesReceived = 0;
uint32_t uartBytesSent = 0;
uint32_t uartErrorCount = 0;
String commandLog[10];
uint8_t commandLogIndex = 0;

// Add enhanced UART command processing:
void listenForUARTCommands()
{
    // Check for data from Back ESP32
    if (Serial2.available())
    {
        unsigned long now = millis();
        String command = Serial2.readStringUntil('\n');
        command.trim();

        if (command.length() > 0)
        {
            lastUARTReceive = now;
            uartBytesReceived += command.length();

            // Log command for diagnostics
            commandLog[commandLogIndex] = "RX: " + command + " @ " + String(now);
            commandLogIndex = (commandLogIndex + 1) % 10;

            DEBUG_PRINT("UART Command received: ");
            DEBUG_PRINTLN(command);

            processMotorCommand(command);
        }
    }
}

// Add enhanced heartbeat with diagnostics:
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
    heartbeatDoc["freeHeap"] = ESP.getFreeHeap();
    heartbeatDoc["uartErrors"] = uartErrorCount;
    heartbeatDoc["lastRX"] = lastUARTReceive;

    String heartbeat;
    serializeJson(heartbeatDoc, heartbeat);
    Serial2.print(heartbeat);
    Serial2.print("\n");

    uartBytesSent += heartbeat.length();
}

// Add to handleSerialCommands():
else if (command == "uart_stats")
{
    Serial.println("=== UART Statistics ===");
    Serial.print("Bytes Received: ");
    Serial.println(uartBytesReceived);
    Serial.print("Bytes Sent: ");
    Serial.println(uartBytesSent);
    Serial.print("Errors: ");
    Serial.println(uartErrorCount);
    Serial.print("Last RX: ");
    Serial.print((millis() - lastUARTReceive) / 1000);
    Serial.println(" seconds ago");
    Serial.println("Recent Commands:");
    for (int i = 0; i < 10; i++) {
        if (commandLog[i].length() > 0) {
            Serial.println("  " + commandLog[i]);
        }
    }
}
else if (command == "mem")
{
    Serial.println("=== Memory Status ===");
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("Min Free Heap: ");
    Serial.print(ESP.getMinFreeHeap());
    Serial.println(" bytes");
}
```

## 4. CAMERA ESP32 Diagnostic Enhancements

### 4.1 Add to main_camera.cpp

```cpp
// Add enhanced WiFi monitoring
unsigned long lastWiFiCheck = 0;
int wifiRetryCount = 0;
unsigned long lastRSSIReport = 0;
String connectionHistory[5];
uint8_t historyIndex = 0;

// Add to setupWiFi():
void setupWiFi()
{
    DEBUG_PRINTLN("Setting up WiFi client connection...");

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    DEBUG_PRINT("Connecting to WiFi");

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 10000))
    {
        delay(500);
        DEBUG_PRINT(".");
        wifiRetryCount++;

        // Flash status LED while connecting
        static unsigned long lastFlash = 0;
        if (millis() - lastFlash > 200)
        {
            statusLEDState = !statusLEDState;
            digitalWrite(33, statusLEDState);
            lastFlash = millis();
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        IPAddress IP = WiFi.localIP();

        // Log successful connection
        connectionHistory[historyIndex] = "WiFi Connected: " + IP.toString() + " @ " + String(millis());
        historyIndex = (historyIndex + 1) % 5;

        DEBUG_PRINTLN();
        DEBUG_PRINTLN("WiFi connected successfully!");
        DEBUG_PRINT("IP Address: ");
        DEBUG_PRINTLN(IP);
        DEBUG_PRINT("RSSI: ");
        DEBUG_PRINT(WiFi.RSSI());
        DEBUG_PRINTLN(" dBm");

        // Turn on status LED to indicate successful connection
        digitalWrite(33, HIGH);
    }
    else
    {
        wifiConnected = false;
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("WiFi connection failed!");
        wifiRetryCount++;

        // Blink status LED to indicate error
        for (int i = 0; i < 6; i++)
        {
            digitalWrite(33, HIGH);
            delay(100);
            digitalWrite(33, LOW);
            delay(100);
        }
    }
}

// Add enhanced heartbeat with WiFi diagnostics:
void sendHeartbeat()
{
    if (webSocketConnected)
    {
        // Create enhanced heartbeat JSON
        StaticJsonDocument<256> heartbeatDoc;
        heartbeatDoc["type"] = "heartbeat";
        heartbeatDoc["source"] = "camera";
        heartbeatDoc["timestamp"] = millis();
        heartbeatDoc["uptime"] = millis();
        heartbeatDoc["wifi_rssi"] = WiFi.RSSI();
        heartbeatDoc["wifi_ip"] = WiFi.localIP().toString();
        heartbeatDoc["wifi_retry_count"] = wifiRetryCount;
        heartbeatDoc["free_heap"] = ESP.getFreeHeap();
        heartbeatDoc["min_heap"] = ESP.getMinFreeHeap();
        heartbeatDoc["boot_count"] = 1;
        heartbeatDoc["version"] = VERSION_STRING;
        heartbeatDoc["wifi_quality"] = getWiFiQuality(WiFi.RSSI());

        String heartbeat;
        serializeJson(heartbeatDoc, heartbeat);

        // Send heartbeat via WebSocket
        webSocket.sendTXT(heartbeat);

        DEBUG_PRINT("Enhanced heartbeat sent - WiFi RSSI: ");
        DEBUG_PRINT(WiFi.RSSI());
        DEBUG_PRINT(" dBm (");
        DEBUG_PRINT(getWiFiQuality(WiFi.RSSI()));
        DEBUG_PRINT("), Free Memory: ");
        DEBUG_PRINT(ESP.getFreeHeap());
        DEBUG_PRINTLN(" bytes");
    }
    else
    {
        DEBUG_PRINTLN("Cannot send heartbeat - WebSocket not connected");
    }
}

// Add WiFi quality assessment helper:
String getWiFiQuality(int32_t rssi)
{
    if (rssi >= -50) return "Excellent";
    else if (rssi >= -60) return "Good";
    else if (rssi >= -70) return "Fair";
    else if (rssi >= -80) return "Poor";
    else return "Very Poor";
}

// Add enhanced serial commands:
else if (command == "wifi_stats")
{
    Serial.println("=== WiFi Statistics ===");
    Serial.print("Connected: ");
    Serial.println(wifiConnected ? "YES" : "NO");
    if (wifiConnected) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.print(" dBm (");
        Serial.print(getWiFiQuality(WiFi.RSSI()));
        Serial.println(")");
        Serial.print("Channel: ");
        Serial.println(WiFi.channel());
    }
    Serial.print("Retry Count: ");
    Serial.println(wifiRetryCount);
    Serial.println("Connection History:");
    for (int i = 0; i < 5; i++) {
        if (connectionHistory[i].length() > 0) {
            Serial.println("  " + connectionHistory[i]);
        }
    }
}
else if (command == "mem_detailed")
{
    Serial.println("=== Memory Analysis ===");
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("Min Free Heap: ");
    Serial.print(ESP.getMinFreeHeap());
    Serial.println(" bytes");
    Serial.print("Heap Size: ");
    Serial.print(ESP.getHeapSize());
    Serial.println(" bytes");
    Serial.print("Stack High Watermark: ");
    Serial.print(uxTaskGetStackHighWaterMark(NULL));
    Serial.println(" words");
}
```

## 5. UART Communication Library Enhancements

### 5.1 Add to UARTComm.cpp

```cpp
// Add member variables to UARTComm class
class UARTComm
{
private:
    HardwareSerial &_serial;
    uint32_t _baudRate;
    unsigned long _lastReceived;
    uint32_t _bytesSent;
    uint32_t _bytesReceived;
    uint32_t _errorCount;
    String _messageLog[20];
    uint8_t _logIndex;

public:
    UARTComm(HardwareSerial &serial, uint32_t baudRate);

    void begin();
    bool available();
    void sendMessage(const JsonDocument &doc);
    StaticJsonDocument<MAX_JSON_DOCUMENT_SIZE> receiveMessage();
    unsigned long getLastReceived();

    // New diagnostic methods
    uint32_t getBytesSent() { return _bytesSent; }
    uint32_t getBytesReceived() { return _bytesReceived; }
    uint32_t getErrorCount() { return _errorCount; }
    void resetCounters();
    String getStatistics();
    void enableLogging(bool enable);
};

// Update constructor
UARTComm::UARTComm(HardwareSerial &serial, uint32_t baudRate)
    : _serial(serial)
    , _baudRate(baudRate)
    , _lastReceived(0)
    , _bytesSent(0)
    , _bytesReceived(0)
    , _errorCount(0)
    , _logIndex(0)
{
    for (int i = 0; i < 20; i++) {
        _messageLog[i] = "";
    }
}

// Update sendMessage to track statistics
void UARTComm::sendMessage(const JsonDocument &doc)
{
    String output;
    serializeJson(doc, output);

    // Send with newline delimiter
    _serial.println(output);
    _serial.flush();

    _bytesSent += output.length();

    // Log message if logging enabled
    _messageLog[_logIndex] = "TX: " + output + " @" + String(millis());
    _logIndex = (_logIndex + 1) % 20;
}

// Update receiveMessage to track statistics
StaticJsonDocument<MAX_JSON_DOCUMENT_SIZE> UARTComm::receiveMessage()
{
    StaticJsonDocument<MAX_JSON_DOCUMENT_SIZE> doc;

    if (!_serial.available())
    {
        return doc;
    }

    String message = readLine();

    if (message.length() == 0)
    {
        return doc;
    }

    _bytesReceived += message.length();

    // Log message
    _messageLog[_logIndex] = "RX: " + message + " @" + String(millis());
    _logIndex = (_logIndex + 1) % 20;

    // Parse JSON
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        _errorCount++;
        DEBUG_PRINT("JSON parse error: ");
        DEBUG_PRINTLN(error.c_str());
        doc.clear();
        return doc;
    }

    _lastReceived = millis();
    return doc;
}

// Add new methods
void UARTComm::resetCounters()
{
    _bytesSent = 0;
    _bytesReceived = 0;
    _errorCount = 0;
}

String UARTComm::getStatistics()
{
    String stats = "UART Statistics:\n";
    stats += "  Bytes Sent: " + String(_bytesSent) + "\n";
    stats += "  Bytes Received: " + String(_bytesReceived) + "\n";
    stats += "  Errors: " + String(_errorCount) + "\n";
    stats += "  Last RX: " + String((millis() - _lastReceived)) + "ms ago\n";
    stats += "  Baud Rate: " + String(_baudRate) + "\n";
    return stats;
}
```

## 6. Configuration File Additions

### 6.1 Add to include/config.h

```cpp
// Add diagnostic configuration options
#define DIAGNOSTICS_ENABLED true
#define VERBOSE_PACKET_LOGGING true
#define DIAGNOSTIC_UPDATE_INTERVAL 30000 // 30 seconds
#define MAX_PACKET_LOG_SIZE 100
#define MEMORY_MONITORING true
#define PERFORMANCE_MONITORING true

// Add diagnostic pin definitions
#ifdef DIAGNOSTICS_ENABLED
#define PIN_DIAGNOSTIC_LED 2     // Status LED for diagnostics
#define PIN_ERROR_LED 12         // Error indicator LED
#define PIN_PACKET_LED 13        // Packet activity LED
#endif

// Add diagnostic timing constants
#define DIAGNOSTIC_REPORT_INTERVAL 60000 // 1 minute
#define WIFI_QUALITY_CHECK_INTERVAL 10000 // 10 seconds
#define PACKET_CAPTURE_BUFFER_SIZE 1000
```

## 7. Build Configuration Updates

### 7.1 Add diagnostic environment to platformio.ini

```ini
; ===== DIAGNOSTIC ENVIRONMENT =====
[env:diagnostic]
platform = espressif32
board = esp32dev
build_src_filter = +<main_*.cpp>
lib_deps =
    bblanchon/ArduinoJson@^7.0.0
    me-no-dev/AsyncTCP@^1.1.3
    me-no-dev/ESPAsyncWebServer@^1.2.3
    links2004/WebSockets@^2.4.1
build_flags =
    ${env.build_flags}
    -DDIAGNOSTICS_ENABLED
    -DVERBOSE_PACKET_LOGGING
    -DMEMORY_MONITORING
    -DPERFORMANCE_MONITORING
    -DSERIAL_DEBUG
    -DWIFI_SSID="ProjectNightfall"
    -DWIFI_PASSWORD="rescue2025"
    -DUART_BAUDRATE=115200
upload_port = /dev/ttyUSB0
monitor_port = /dev/ttyUSB0
```

## 8. Usage Instructions

### 8.1 Build with Diagnostics

```bash
# Build diagnostic version
pio run -e diagnostic -t upload

# Monitor diagnostic output
pio device monitor -e diagnostic
```

### 8.2 Diagnostic Commands

**Available Commands**:

- `diagnostic` - Generate full diagnostic report
- `wifi_status` - Show WiFi connection details
- `uart_stats` - Show UART communication statistics
- `mem` - Show memory usage
- `reset_errors` - Reset error counters

**Enhanced Monitoring**:

- Automatic network status updates every 30 seconds
- Packet capture and logging
- Memory usage tracking
- Connection quality assessment
- Performance metrics

This enhanced diagnostic system provides comprehensive visibility into network connectivity, communication patterns, and system performance for effective troubleshooting and optimization.
