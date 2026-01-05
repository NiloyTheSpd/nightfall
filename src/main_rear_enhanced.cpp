/**
 * @file    main_rear_enhanced.cpp
 * @brief   Project Nightfall - V3.2 Autonomous Master
 * @date    January 6, 2026
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

// --- HARDWARE PINS (STRICT) ---
#ifndef PIN_MOTOR_1
  #define PIN_MOTOR_1 13
  #define PIN_MOTOR_2 14
  #define PIN_MOTOR_3 18
  #define PIN_MOTOR_4 19
  #define PIN_MOTOR_5 23
  #define PIN_MOTOR_6 27
  #define PIN_US_TRIG 4
  #define PIN_US_ECHO 36
  #define PIN_GAS_ANALOG 32
  #define PIN_GAS_DIGITAL 33
#endif

// --- GLOBALS ---
AsyncWebServer webServer(80);
WebSocketsServer webSocketServer(8888);

// Timers
unsigned long currentMillis = 0;
unsigned long lastSensorUpdate = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastMotorUpdate = 0;
unsigned long lastFrontHeartbeat = 0;
unsigned long emergencyTimestamp = 0;
unsigned long lastBuzzerUpdate = 0;

// System State
bool emergencyStop = false;
bool buzzerActive = false;
int connectionStatus = 0; 

// --- AUTONOMOUS VARIABLES ---
bool autoMode = false;           // Is Auto Mode ON?
int autoState = 0;               // 0=Forward, 1=Backing, 2=Turning
unsigned long autoTimer = 0;     // Timer for auto maneuvers

// Motor State
int targetFrontLeft = 0, targetFrontRight = 0;
int targetRearLeft = 0, targetRearRight = 0;
int targetCenterLeft = 0, targetCenterRight = 0;
int currentRearLeft = 0, currentRearRight = 0;

// Sensors
float frontDistance = 400.0;
int gasLevel = 0;
float batteryVoltage = 14.8;
bool frontSensorValid = false;

const char *ssid = "ProjectNightfall";
const char *password = "rescue2025";

// Prototypes
void updateSensors();
void checkSafety();
void updateMotors();
void sendToFront();
void handleUART();
void sendTelemetry();
void processCommand(const JsonDocument &doc);
void setBuzzer(bool state);
void runAutonomousLogic(); // NEW FUNCTION
void stopAll();
void addCors(AsyncWebServerResponse *response);

void setup() {
    Serial.begin(115200);
    
    // Hardware Init
    pinMode(PIN_MOTOR_1, OUTPUT); pinMode(PIN_MOTOR_2, OUTPUT);
    pinMode(PIN_MOTOR_3, OUTPUT); pinMode(PIN_MOTOR_4, OUTPUT);
    pinMode(PIN_MOTOR_5, OUTPUT); pinMode(PIN_MOTOR_6, OUTPUT);
    pinMode(PIN_US_TRIG, OUTPUT); pinMode(PIN_US_ECHO, INPUT);
    pinMode(PIN_GAS_ANALOG, INPUT); pinMode(PIN_GAS_DIGITAL, INPUT); 

    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    WiFi.softAP(ssid, password);
    
    // React API
    webServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"online\"}");
        addCors(response);
        request->send(response);
    });
    webServer.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            AsyncWebServerResponse *response = request->beginResponse(200);
            addCors(response);
            request->send(response);
        } else request->send(404);
    });

    webServer.begin();
    webSocketServer.begin();
    webSocketServer.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length){
        if(type == WStype_TEXT) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if(!err && doc["command"].is<const char*>()) processCommand(doc);
        }
    });
}

void loop() {
    currentMillis = millis();
    webSocketServer.loop();
    handleUART();

    // 1. Motor & Auto Loop (50ms)
    if (currentMillis - lastMotorUpdate >= 50) {
        if (autoMode && !emergencyStop) {
            runAutonomousLogic();
        }
        updateMotors();
        sendToFront();
        lastMotorUpdate = currentMillis;
    }

    // 2. Sensor Loop (100ms)
    if (currentMillis - lastSensorUpdate >= 100) {
        updateSensors();
        checkSafety();
        
        // Buzzer
        if (buzzerActive) {
            if (currentMillis - lastBuzzerUpdate > 200) { 
                static bool bState = false;
                bState = !bState;
                setBuzzer(bState);
                lastBuzzerUpdate = currentMillis;
            }
        } else setBuzzer(false);

        if (emergencyStop && (currentMillis - emergencyTimestamp > 5000)) buzzerActive = false;
        lastSensorUpdate = currentMillis;
    }

    // 3. Telemetry (200ms)
    if (currentMillis - lastTelemetryUpdate >= 200) {
        sendTelemetry();
        lastTelemetryUpdate = currentMillis;
    }
}

// --- NEW AUTONOMOUS LOGIC ---
void runAutonomousLogic() {
    // State 0: Driving Forward
    if (autoState == 0) {
        if (frontSensorValid && frontDistance < 30.0) {
            // Obstacle Detected! Switch to Backing Up
            autoState = 1; 
            autoTimer = currentMillis;
            // Stop briefly
            targetFrontLeft = targetFrontRight = targetRearLeft = targetRearRight = targetCenterLeft = targetCenterRight = 0;
        } else {
            // Path Clear -> Drive Forward
            int speed = 160; 
            targetFrontLeft = targetFrontRight = targetRearLeft = targetRearRight = targetCenterLeft = targetCenterRight = speed;
        }
    }
    // State 1: Backing Up (for 800ms)
    else if (autoState == 1) {
        if (currentMillis - autoTimer > 800) {
            // Done backing up -> Switch to Turning
            autoState = 2;
            autoTimer = currentMillis;
        } else {
            int speed = -150;
            targetFrontLeft = targetFrontRight = targetRearLeft = targetRearRight = targetCenterLeft = targetCenterRight = speed;
        }
    }
    // State 2: Turning Left (for 600ms)
    else if (autoState == 2) {
        if (currentMillis - autoTimer > 600) {
            // Done turning -> Resume Forward
            autoState = 0;
        } else {
            // Tank Turn Left
            targetFrontLeft = targetCenterLeft = targetRearLeft = -160;
            targetFrontRight = targetCenterRight = targetRearRight = 160;
        }
    }
}

void processCommand(const JsonDocument &doc) {
    String cmd = doc["command"];
    
    // If user presses ANY manual button, disable Auto Mode
    if (cmd != "auto_toggle" && cmd != "emergency") {
        autoMode = false;
    }

    if (cmd == "emergency") {
        emergencyStop = !emergencyStop;
        autoMode = false; // Kill auto on emergency
        buzzerActive = emergencyStop;
        if(emergencyStop) emergencyTimestamp = millis();
        return;
    }

    if (emergencyStop) return;

    if (cmd == "auto_toggle") {
        autoMode = !autoMode;
        autoState = 0; // Reset state
        if (!autoMode) stopAll();
        return;
    }

    int spd = 180; 
    if (cmd == "stop") stopAll();
    else if (cmd == "forward") {
        targetFrontLeft = targetFrontRight = targetCenterLeft = targetCenterRight = targetRearLeft = targetRearRight = spd;
    }
    else if (cmd == "backward") {
        targetFrontLeft = targetFrontRight = targetCenterLeft = targetCenterRight = targetRearLeft = targetRearRight = -spd;
    }
    else if (cmd == "left") {
        targetFrontLeft = targetCenterLeft = targetRearLeft = -spd;
        targetFrontRight = targetCenterRight = targetRearRight = spd;
    }
    else if (cmd == "right") {
        targetFrontLeft = targetCenterLeft = targetRearLeft = spd;
        targetFrontRight = targetCenterRight = targetRearRight = -spd;
    }
}

void stopAll() {
    targetFrontLeft = targetFrontRight = targetCenterLeft = targetCenterRight = targetRearLeft = targetRearRight = 0;
}

// --- STANDARD FUNCTIONS ---
void addCors(AsyncWebServerResponse *response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

void updateSensors() {
    digitalWrite(PIN_US_TRIG, LOW); delayMicroseconds(2);
    digitalWrite(PIN_US_TRIG, HIGH); delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);
    
    long duration = pulseIn(PIN_US_ECHO, HIGH, 23500); 
    if (duration > 0) {
        frontDistance = duration * 0.034 / 2;
        frontSensorValid = true;
    } else {
        frontDistance = 400.0;
        frontSensorValid = false;
    }
    gasLevel = analogRead(PIN_GAS_ANALOG);
}

void checkSafety() {
    // In Auto Mode, safety is handled by the state machine (distance check)
    // But we still need emergency stops for extreme close range or gas
    bool criticalObstacle = frontSensorValid && (frontDistance < 10.0); // 10cm HARD STOP
    bool gasDanger = gasLevel > 2000; 

    if (!emergencyStop && (criticalObstacle || gasDanger)) {
        emergencyStop = true;
        autoMode = false; // Kill auto
        emergencyTimestamp = millis();
        buzzerActive = true;
        stopAll();
    }
}

void setBuzzer(bool state) {
    if (state) {
        pinMode(PIN_GAS_DIGITAL, OUTPUT); digitalWrite(PIN_GAS_DIGITAL, HIGH);
    } else {
        digitalWrite(PIN_GAS_DIGITAL, LOW); pinMode(PIN_GAS_DIGITAL, INPUT); 
    }
}

void updateMotors() {
    currentRearLeft = targetRearLeft;
    currentRearRight = targetRearRight;

    if (currentRearLeft >= 0) {
        analogWrite(PIN_MOTOR_1, currentRearLeft);
        digitalWrite(PIN_MOTOR_2, HIGH); digitalWrite(PIN_MOTOR_3, LOW);
    } else {
        analogWrite(PIN_MOTOR_1, abs(currentRearLeft));
        digitalWrite(PIN_MOTOR_2, LOW); digitalWrite(PIN_MOTOR_3, HIGH);
    }

    if (currentRearRight >= 0) {
        analogWrite(PIN_MOTOR_6, currentRearRight);
        digitalWrite(PIN_MOTOR_5, HIGH); digitalWrite(PIN_MOTOR_4, LOW);
    } else {
        analogWrite(PIN_MOTOR_6, abs(currentRearRight));
        digitalWrite(PIN_MOTOR_5, LOW); digitalWrite(PIN_MOTOR_4, HIGH);
    }
}

void sendToFront() {
    JsonDocument doc;
    doc["L"] = targetFrontLeft; doc["R"] = targetFrontRight;
    doc["CL"] = targetCenterLeft; doc["CR"] = targetCenterRight;
    String out; serializeJson(doc, out); Serial2.println(out);
}

void handleUART() {
    while (Serial2.available()) {
        String msg = Serial2.readStringUntil('\n');
        if (msg.indexOf("heartbeat") >= 0) { lastFrontHeartbeat = millis(); connectionStatus = 2; }
    }
    if (millis() - lastFrontHeartbeat > 3000) connectionStatus = 1; 
}

void sendTelemetry() {
    char buffer[150];
    snprintf(buffer, sizeof(buffer), 
        "{\"d\":%.1f,\"g\":%d,\"v\":%.1f,\"e\":%s,\"fo\":%s,\"auto\":%s}",
        frontDistance, gasLevel, batteryVoltage, 
        emergencyStop ? "true" : "false", 
        (connectionStatus==2) ? "true" : "false",
        autoMode ? "true" : "false"
    );
    webSocketServer.broadcastTXT(buffer);
}
