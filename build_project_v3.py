import os

# --- 1. FIRMWARE CONFIGURATION (platformio.ini) ---
platformio_ini = r"""; PlatformIO Project Configuration File
; Project Nightfall - Phase 2 MVP Rescue Robot
; Optimized for V3.1.0 Firmware Suite (Rear/Front/Cam)

[platformio]
default_envs = rear_esp32
src_dir = src
lib_dir = lib
include_dir = include

[env]
framework = arduino
monitor_speed = 115200
board_build.f_cpu = 240000000L
board_build.flash_mode = dio
board_build.flash_size = 4MB
build_flags = 
    -D VERSION="3.1.0"
    -D SERIAL_DEBUG=1

[env:rear_esp32]
platform = espressif32
board = esp32dev
build_src_filter = +<main_rear_enhanced.cpp>
upload_port = COM8  ; CHECK YOUR PORT!
monitor_port = COM8 ; CHECK YOUR PORT!
lib_deps =
    bblanchon/ArduinoJson @ ^7.0.0
    https://github.com/mathieucarbou/ESPAsyncWebServer.git
    links2004/WebSockets @ ^2.4.1
build_flags =
    ${env.build_flags}
    -D REAR_CONTROLLER
    -D WIFI_SSID="ProjectNightfall"
    -D WIFI_PASSWORD="rescue2025"
    -D PIN_MOTOR_1=13
    -D PIN_MOTOR_2=14
    -D PIN_MOTOR_3=18
    -D PIN_MOTOR_4=19
    -D PIN_MOTOR_5=23
    -D PIN_MOTOR_6=27
    -D PIN_US_TRIG=4
    -D PIN_US_ECHO=36
    -D PIN_GAS_ANALOG=32
    -D PIN_GAS_DIGITAL=33

[env:front_esp32]
platform = espressif32
board = esp32dev
build_src_filter = +<main_front_enhanced.cpp>
upload_port = COM9  ; CHANGE THIS!
monitor_port = COM9 ; CHANGE THIS!
lib_deps =
    bblanchon/ArduinoJson @ ^7.0.0
build_flags =
    ${env.build_flags}
    -D FRONT_CONTROLLER
    -D UART_BAUDRATE=115200
    -D PIN_UART_RX=16
    -D PIN_UART_TX=17

[env:camera_esp32]
platform = espressif32
board = esp32cam
build_src_filter = +<main_camera.cpp>
upload_port = COM10 ; CHANGE THIS!
monitor_port = COM10 ; CHANGE THIS!
board_build.f_flash = 80000000L
build_flags =
    ${env.build_flags}
    -D BOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
    -D CAMERA_MODULE
    -D FLASH_LED_PIN=4
    -D STATUS_LED_PIN=33
lib_deps =
    links2004/WebSockets @ ^2.4.1
"""

# --- 2. REAR MASTER FIRMWARE (V3.2 AUTO) ---
rear_code = r"""/**
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
"""

# --- 3. FRONT SLAVE FIRMWARE ---
front_code = r"""/**
 * @file    main_front_enhanced.cpp
 * @brief   Project Nightfall - Optimized Front Slave
 * @version 3.1.0
 */
#include <Arduino.h>
#include <ArduinoJson.h>

#define M1_L_PWM 13  
#define M1_L_IN1 23
#define M1_L_IN2 22
#define M1_R_PWM 25  
#define M1_R_IN1 26
#define M1_R_IN2 27

#define M2_L_PWM 14  
#define M2_L_IN1 32
#define M2_L_IN2 33
#define M2_R_PWM 18  
#define M2_R_IN1 19
#define M2_R_IN2 21

#define RXD2 16
#define TXD2 17

const unsigned long TIMEOUT_MS = 1000;    
const unsigned long HEARTBEAT_MS = 500;   

unsigned long lastSignalTime = 0;
unsigned long lastHeartbeatTime = 0;
bool emergencyState = false;

int t_FL = 0, t_FR = 0, t_CL = 0, t_CR = 0;

void setMotor(int pwmPin, int in1, int in2, int speed);
void handleUART();
void sendHeartbeat();
void emergencyStop();

void setup() {
    Serial.begin(115200);  
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); 

    pinMode(M1_L_PWM, OUTPUT); pinMode(M1_L_IN1, OUTPUT); pinMode(M1_L_IN2, OUTPUT);
    pinMode(M1_R_PWM, OUTPUT); pinMode(M1_R_IN1, OUTPUT); pinMode(M1_R_IN2, OUTPUT);
    
    pinMode(M2_L_PWM, OUTPUT); pinMode(M2_L_IN1, OUTPUT); pinMode(M2_L_IN2, OUTPUT);
    pinMode(M2_R_PWM, OUTPUT); pinMode(M2_R_IN1, OUTPUT); pinMode(M2_R_IN2, OUTPUT);

    emergencyStop(); 
}

void loop() {
    unsigned long now = millis();
    handleUART();

    if (now - lastSignalTime > TIMEOUT_MS) {
        if (!emergencyState) {
            emergencyStop();
            emergencyState = true;
        }
    } else {
        emergencyState = false;
        setMotor(M1_L_PWM, M1_L_IN1, M1_L_IN2, t_FL);
        setMotor(M1_R_PWM, M1_R_IN1, M1_R_IN2, t_FR);
        setMotor(M2_L_PWM, M2_L_IN1, M2_L_IN2, t_CL);
        setMotor(M2_R_PWM, M2_R_IN1, M2_R_IN2, t_CR);
    }

    if (now - lastHeartbeatTime > HEARTBEAT_MS) {
        sendHeartbeat();
        lastHeartbeatTime = now;
    }
}

void handleUART() {
    if (Serial2.available()) {
        String data = Serial2.readStringUntil('\n');
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);

        if (!err) {
            lastSignalTime = millis();
            t_FL = doc["L"] | 0;
            t_FR = doc["R"] | 0;
            t_CL = doc["CL"] | 0;
            t_CR = doc["CR"] | 0;
        }
    }
}

void setMotor(int pwmPin, int in1, int in2, int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        digitalWrite(in1, HIGH); digitalWrite(in2, LOW); analogWrite(pwmPin, speed);
    } else if (speed < 0) {
        digitalWrite(in1, LOW); digitalWrite(in2, HIGH); analogWrite(pwmPin, abs(speed));
    } else {
        digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwmPin, 0);
    }
}

void emergencyStop() {
    t_FL = 0; t_FR = 0; t_CL = 0; t_CR = 0;
    setMotor(M1_L_PWM, M1_L_IN1, M1_L_IN2, 0);
    setMotor(M1_R_PWM, M1_R_IN1, M1_R_IN2, 0);
    setMotor(M2_L_PWM, M2_L_IN1, M2_L_IN2, 0);
    setMotor(M2_R_PWM, M2_R_IN1, M2_R_IN2, 0);
}

void sendHeartbeat() {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), 
        "{\"type\":\"heartbeat\",\"leftSpeed\":%d,\"rightSpeed\":%d}", t_FL, t_FR);
    Serial2.println(buffer);
}
"""

# --- 4. CAMERA FIRMWARE ---
camera_code = r"""/**
 * @file    main_camera.cpp
 * @brief   Project Nightfall - Optimized Camera Node
 * @version 3.1.0
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h" 
#include "soc/rtc_cntl_reg.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LED_PIN      4
#define STATUS_LED_PIN    33 

const char *ssid = "ProjectNightfall";
const char *password = "rescue2025";
const char *master_host = "192.168.4.1";
const uint16_t master_port = 8888;

WebSocketsClient webSocket;
WiFiServer streamServer(80);

unsigned long lastHeartbeat = 0;
bool flashState = false;

void setupCamera();
void handleStream();
void sendHeartbeat();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  
  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, HIGH);

  setupCamera();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); 
  }
  
  digitalWrite(STATUS_LED_PIN, LOW); 
  streamServer.begin();

  webSocket.begin(master_host, master_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  handleStream();

  if (millis() - lastHeartbeat > 2000) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 15;           
  config.fb_count = 2;
  esp_camera_init(&config);
}

void handleStream() {
  WiFiClient client = streamServer.available();
  if (!client) return;

  String req = client.readStringUntil('\r');
  if (req.indexOf("GET /stream") != -1) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println();

    while (client.connected()) {
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) break;
      client.printf("Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
      client.write(fb->buf, fb->len);
      client.println("\r\n--frame");
      esp_camera_fb_return(fb);
      webSocket.loop(); 
    }
  }
  client.stop();
}

void sendHeartbeat() {
  char buffer[128];
  snprintf(buffer, sizeof(buffer), 
    "{\"type\":\"cam_telemetry\",\"ip\":\"%s\",\"rssi\":%d}", 
    WiFi.localIP().toString().c_str(), WiFi.RSSI());
  webSocket.sendTXT(buffer);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = (char*)payload;
    if (msg.indexOf("flash_on") >= 0) digitalWrite(FLASH_LED_PIN, HIGH);
    else if (msg.indexOf("flash_off") >= 0) digitalWrite(FLASH_LED_PIN, LOW);
  }
}
"""

# --- 5. DASHBOARD MAIN (V3.3 ULTIMATE) ---
dashboard_jsx = r"""import React, { useEffect, useRef, useState, useCallback } from 'react';
import {
  Camera, Radio, Gauge, Battery, AlertTriangle, Wind,
  Wifi, WifiOff, ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
  StopCircle, Eye, Zap, Signal, Settings, Maximize2, AlertCircle,
  Bot, Clock, Volume2, VolumeX, Keyboard
} from 'lucide-react';

// --- FIXED IMPORTS (Relative to src/components/) ---
import { DashboardLoader } from './components/LoadingSkeleton';
import TelemetryChart, { useTelemetryHistory } from './components/TelemetryChart';
import SettingsPanel from './components/SettingsPanel';
import SystemHealthPanel from './components/SystemHealthPanel';
import FullscreenVideo from './components/FullscreenVideo';
import DataExportPanel from './components/DataExportPanel';

const WEBSOCKET_URL = 'ws://192.168.4.1:8888';
const ConnectionStates = { DISCONNECTED: 'disconnected', CONNECTING: 'connecting', CONNECTED: 'connected', ERROR: 'error' };
const SystemStatus = { HEALTHY: 'healthy', WARNING: 'warning', CRITICAL: 'critical', OFFLINE: 'offline' };

// --- LOCAL UTILITIES ---
function Tooltip({ children, content, position = 'bottom' }) {
  const [show, setShow] = useState(false);
  return (
    <div className="relative" onMouseEnter={() => setShow(true)} onMouseLeave={() => setShow(false)}>
      {children}
      {show && (
        <div className={`absolute ${position === 'bottom' ? 'top-full mt-2' : 'bottom-full mb-2'} left-1/2 -translate-x-1/2 px-2 py-1 bg-gray-900 text-white text-xs rounded whitespace-nowrap z-50 animate-fade-in shadow-lg border border-gray-700`}>{content}</div>
      )}
    </div>
  );
}

function KeyboardShortcuts({ onClose }) {
  return (
    <div className="fixed inset-0 bg-black/70 flex items-center justify-center z-50 animate-fade-in" onClick={onClose}>
      <div className="bg-gray-800 p-6 rounded-2xl border border-gray-700 max-w-md w-full mx-4 animate-slide-up shadow-2xl" onClick={e => e.stopPropagation()}>
        <div className="flex justify-between items-center mb-4">
          <h3 className="text-lg font-bold flex items-center gap-2"><Keyboard className="w-5 h-5 text-purple-400" /> Keyboard Shortcuts</h3>
          <button onClick={onClose} className="p-1 hover:bg-gray-700 rounded-lg transition-colors text-gray-400 hover:text-white">✕</button>
        </div>
        <div className="space-y-2 text-sm">
          {[{ key: 'W', action: 'Move Forward' }, { key: 'S', action: 'Move Backward' }, { key: 'A', action: 'Turn Left' }, { key: 'D', action: 'Turn Right' }, { key: 'Space', action: 'Stop' }, { key: 'Esc', action: 'Emergency Stop' }, { key: '?', action: 'Show Help' }].map(({ key, action }) => (
            <div key={key} className="flex justify-between items-center py-2 border-b border-gray-700/50 last:border-0"><span className="text-gray-300">{action}</span><kbd className="px-2 py-1 bg-gray-700 rounded text-white font-mono text-xs border border-gray-600 shadow-sm">{key}</kbd></div>
          ))}
        </div>
      </div>
    </div>
  );
}

// --- MAIN DASHBOARD ---
export default function DashboardEnhanced() {
  const [isLoading, setIsLoading] = useState(true);
  const [showSettings, setShowSettings] = useState(false);
  const [showFullscreenVideo, setShowFullscreenVideo] = useState(false);
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [showShortcuts, setShowShortcuts] = useState(false);
  const [soundEnabled, setSoundEnabled] = useState(true);
  
  const [settings, setSettings] = useState(() => {
    const saved = localStorage.getItem('dashboard-settings');
    return saved ? JSON.parse(saved) : { theme: 'dark', maxTelemetryHistory: 50 };
  });

  const { batteryHistory, gasHistory, distanceHistory, signalHistory, addTelemetryPoint } = useTelemetryHistory(settings.maxTelemetryHistory);
  const [connectionState, setConnectionState] = useState(ConnectionStates.DISCONNECTED);
  const [connectionStats, setConnectionStats] = useState({ messagesReceived: 0, messagesSent: 0 });

  const [telemetry, setTelemetry] = useState({
    back_status: 'offline', front_status: false, camera_status: false,
    dist: 0, gas: 0, cam_ip: null, battery: 11.1, signal_strength: 0,
    uptime: 0, emergency: false, auto_mode: false
  });

  const [systemHealth, setSystemHealth] = useState({ brain: SystemStatus.OFFLINE, motors: SystemStatus.OFFLINE, vision: SystemStatus.OFFLINE });
  const [performanceMetrics, setPerformanceMetrics] = useState({ dataRate: 0, commandSuccessRate: 100, fps: 0 });
  const [alerts, setAlerts] = useState([]);
  const [videoFps, setVideoFps] = useState(0);

  const wsRef = useRef(null);
  const fpsCounterRef = useRef({ frames: 0, lastTime: Date.now() });
  const lastCommandTimeRef = useRef(0);
  const commandFeedbackRef = useRef(null);
  
  const playSound = useCallback((type) => {
    if (!soundEnabled) return;
    try {
      const AudioContext = window.AudioContext || window.webkitAudioContext;
      if (!AudioContext) return;
      const ctx = new AudioContext();
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.connect(gain); gain.connect(ctx.destination);
      const now = ctx.currentTime;
      if (type === 'move') { osc.frequency.setValueAtTime(600, now); osc.frequency.exponentialRampToValueAtTime(300, now + 0.1); gain.gain.setValueAtTime(0.05, now); gain.gain.exponentialRampToValueAtTime(0.01, now + 0.1); osc.start(now); osc.stop(now + 0.1); }
      else if (type === 'stop') { osc.frequency.setValueAtTime(200, now); osc.frequency.exponentialRampToValueAtTime(100, now + 0.15); gain.gain.setValueAtTime(0.1, now); gain.gain.exponentialRampToValueAtTime(0.01, now + 0.15); osc.start(now); osc.stop(now + 0.15); }
      else if (type === 'alert') { osc.type = 'square'; osc.frequency.setValueAtTime(800, now); osc.frequency.setValueAtTime(0, now + 0.1); osc.frequency.setValueAtTime(800, now + 0.2); gain.gain.setValueAtTime(0.1, now); osc.start(now); osc.stop(now + 0.3); }
    } catch (e) { console.warn("Audio error", e); }
  }, [soundEnabled]);

  const addAlert = useCallback((message, type = 'info') => {
    const alert = { id: Date.now() + Math.random(), message, type, timestamp: new Date() };
    setAlerts(prev => [alert, ...prev].slice(0, 10));
    if (type === 'error') playSound('alert');
    setTimeout(() => setAlerts(prev => prev.filter(a => a.id !== alert.id)), 5000);
  }, [playSound]);

  const showCommandFeedback = useCallback((command) => {
    if (commandFeedbackRef.current) {
      commandFeedbackRef.current.textContent = command.replace('_', ' ').toUpperCase();
      commandFeedbackRef.current.style.opacity = '1';
      setTimeout(() => { if(commandFeedbackRef.current) commandFeedbackRef.current.style.opacity = '0'; }, 500);
    }
  }, []);

  useEffect(() => {
    setTimeout(() => setIsLoading(false), 1500);
    const connect = () => {
      if (wsRef.current?.readyState === WebSocket.OPEN) return;
      setConnectionState(ConnectionStates.CONNECTING);
      const ws = new WebSocket(WEBSOCKET_URL);
      wsRef.current = ws;
      ws.onopen = () => { setConnectionState(ConnectionStates.CONNECTED); addAlert('Connected to Robot Brain', 'success'); };
      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          setConnectionStats(prev => ({ ...prev, messagesReceived: prev.messagesReceived + 1 }));
          if (data.type === 'cam_telemetry') { setTelemetry(prev => ({ ...prev, cam_ip: data.ip, signal_strength: data.rssi, camera_status: true })); setSystemHealth(prev => ({...prev, vision: SystemStatus.HEALTHY})); } 
          else if (data.d !== undefined) {
            setTelemetry(prev => {
              const newData = { ...prev, dist: data.d, gas: data.g, battery: data.v, emergency: data.e, front_status: data.fo, back_status: 'ok', auto_mode: data.auto, uptime: prev.uptime + 0.5 };
              addTelemetryPoint(newData); return newData;
            });
            setSystemHealth(prev => ({ ...prev, brain: SystemStatus.HEALTHY, motors: data.fo ? SystemStatus.HEALTHY : SystemStatus.CRITICAL }));
            if (data.e) addAlert("EMERGENCY STOP ACTIVE", 'error');
          }
        } catch (e) { console.error("Parse Error", e); }
      };
      ws.onclose = () => { setConnectionState(ConnectionStates.DISCONNECTED); setSystemHealth({ brain: SystemStatus.OFFLINE, motors: SystemStatus.OFFLINE, vision: SystemStatus.OFFLINE }); setTimeout(connect, 3000); };
      ws.onerror = () => { setConnectionState(ConnectionStates.ERROR); ws.close(); };
    };
    connect();
    return () => { if (wsRef.current) wsRef.current.close(); };
  }, [addAlert, addTelemetryPoint]);

  const sendCommand = (cmdString) => {
    if (!wsRef.current || wsRef.current.readyState !== WebSocket.OPEN) return;
    const now = Date.now();
    if (now - lastCommandTimeRef.current < 50) return;
    wsRef.current.send(JSON.stringify({ command: cmdString }));
    lastCommandTimeRef.current = now;
    setConnectionStats(prev => ({ ...prev, messagesSent: prev.messagesSent + 1 }));
    showCommandFeedback(cmdString);
    if(cmdString !== 'auto_toggle') playSound(cmdString === 'stop' ? 'stop' : 'move');
  };

  const toggleAuto = () => sendCommand('auto_toggle');
  const moveForward = () => sendCommand('forward');
  const moveBackward = () => sendCommand('backward');
  const turnLeft = () => sendCommand('left');
  const turnRight = () => sendCommand('right');
  const stopRobot = () => sendCommand('stop');
  const emergencyStop = () => sendCommand('emergency');

  useEffect(() => {
    const handleKeyDown = (e) => {
      if(e.repeat) return;
      switch(e.key.toLowerCase()) {
        case 'w': moveForward(); break; case 's': moveBackward(); break; case 'a': turnLeft(); break; case 'd': turnRight(); break; case ' ': stopRobot(); break; case 'escape': emergencyStop(); break; case '?': setShowShortcuts(true); break; default: break;
      }
    };
    const handleKeyUp = (e) => { if(['w','a','s','d'].includes(e.key.toLowerCase())) stopRobot(); };
    window.addEventListener('keydown', handleKeyDown); window.addEventListener('keyup', handleKeyUp);
    return () => { window.removeEventListener('keydown', handleKeyDown); window.removeEventListener('keyup', handleKeyUp); };
  }, []);

  useEffect(() => {
    const interval = setInterval(() => {
      const now = Date.now();
      if (now - fpsCounterRef.current.lastTime >= 1000) { setVideoFps(fpsCounterRef.current.frames); fpsCounterRef.current.frames = 0; fpsCounterRef.current.lastTime = now; }
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  const formatUptime = (seconds) => { const hrs = Math.floor(seconds / 3600); const mins = Math.floor((seconds % 3600) / 60); return `${hrs}h ${mins}m`; };
  const getConnectionQuality = (signal) => { if (signal >= -50) return 4; if (signal >= -60) return 3; if (signal >= -70) return 2; return 1; };

  if (isLoading) return <DashboardLoader />;

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-900 via-slate-900 to-black text-white p-4 sm:p-6 font-sans animate-fade-in">
      {showShortcuts && <KeyboardShortcuts onClose={() => setShowShortcuts(false)} />}
      <SettingsPanel isOpen={showSettings} onClose={() => setShowSettings(false)} settings={settings} onSettingsChange={setSettings} />
      <FullscreenVideo isOpen={showFullscreenVideo} onClose={() => setShowFullscreenVideo(false)} cameraIp={telemetry.cam_ip || '192.168.4.3'} cameraStatus={telemetry.camera_status} />

      {/* HEADER */}
      <header className="mb-6 flex flex-wrap justify-between items-center bg-gray-800/50 p-4 rounded-2xl backdrop-blur-sm border border-gray-700/50 shadow-xl relative overflow-hidden">
        <div className="absolute inset-0 rounded-2xl opacity-10 bg-gradient-to-r from-blue-500 via-purple-500 to-pink-500 animate-pulse" />
        <div className="flex items-center gap-4 relative z-10">
            <div className="bg-blue-600 p-2 rounded-xl shadow-lg shadow-blue-900/40 relative">
              <Radio className="w-6 h-6 text-white" />
              {connectionState === ConnectionStates.CONNECTED && <span className="absolute -top-1 -right-1 w-3 h-3 bg-green-400 rounded-full animate-ping" />}
            </div>
            <div>
              <h1 className="text-2xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-purple-400 to-pink-400">Project Nightfall</h1>
              <div className="flex items-center gap-2 text-gray-400 text-xs font-medium tracking-wide"><span>Autonomous Rescue System</span><span className="text-gray-600">//</span><Tooltip content="Version 3.3.0"><span className="text-purple-400 cursor-help">V3.3.0</span></Tooltip></div>
            </div>
        </div>
        <div className="flex gap-2 items-center relative z-10 mt-4 sm:mt-0">
            <Tooltip content="Keyboard Shortcuts (?)"><button onClick={() => setShowShortcuts(true)} className="p-2 bg-gray-700/50 hover:bg-gray-600 rounded-lg transition-all"><Keyboard className="w-5 h-5" /></button></Tooltip>
            <Tooltip content={soundEnabled ? 'Sound On' : 'Sound Off'}><button onClick={() => setSoundEnabled(!soundEnabled)} className="p-2 bg-gray-700/50 hover:bg-gray-600 rounded-lg transition-all">{soundEnabled ? <Volume2 className="w-5 h-5" /> : <VolumeX className="w-5 h-5" />}</button></Tooltip>
            {telemetry.auto_mode && (<div className="flex items-center gap-2 px-3 py-1.5 rounded-lg border border-purple-500/50 text-purple-400 bg-purple-500/10 animate-pulse"><Bot className="w-4 h-4" /><span className="font-semibold text-sm hidden sm:inline">AUTO PILOT</span></div>)}
            <div className={`flex items-center gap-2 px-3 py-1.5 rounded-lg border transition-all ${connectionState === ConnectionStates.CONNECTED ? 'text-green-400 bg-green-400/10 border-green-500/30' : 'text-red-400 bg-red-400/10 border-red-500/30'}`}><Wifi className="w-4 h-4" /><div className="flex flex-col"><span className="font-semibold text-sm capitalize hidden sm:inline">{connectionState}</span></div></div>
            <div className="hidden md:flex items-center gap-1 px-3 py-1.5 bg-gray-700/30 rounded-lg border border-gray-700/50"><Clock className="w-4 h-4 text-gray-400" /><span className="font-mono text-sm text-gray-300">{formatUptime(telemetry.uptime)}</span></div>
            <button onClick={() => setShowSettings(true)} className="p-2 bg-gray-700/50 hover:bg-gray-600 rounded-lg transition-all"><Settings className="w-5 h-5" /></button>
        </div>
      </header>

      {/* MAIN LAYOUT */}
      <div className="grid grid-cols-1 xl:grid-cols-12 gap-6">
        {/* ZONE A: VIDEO */}
        <div className="xl:col-span-7 bg-black rounded-2xl overflow-hidden border border-gray-800 shadow-2xl relative group h-[400px] sm:h-[500px]">
             <div className="absolute inset-0 pointer-events-none z-10 bg-[radial-gradient(circle,transparent_60%,rgba(0,0,0,0.8)_100%)]" />
             {telemetry.camera_status && (<div className="absolute top-4 left-4 flex items-center gap-2 z-20"><span className="relative flex h-3 w-3"><span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-red-400 opacity-75"></span><span className="relative inline-flex rounded-full h-3 w-3 bg-red-500"></span></span><span className="text-red-400 font-bold text-xs tracking-wider uppercase">REC</span></div>)}
             {telemetry.cam_ip ? (<img src={`http://${telemetry.cam_ip}:81/stream`} className="w-full h-full object-contain" alt="Live Feed" onLoad={() => fpsCounterRef.current.frames++} onError={(e) => { e.target.style.display='none'; }} />) : (<div className="flex flex-col items-center justify-center h-full text-gray-500 animate-pulse"><Camera className="w-16 h-16 mb-4 opacity-50" /><p className="text-sm">Searching for Camera Signal...</p></div>)}
             <div className="absolute bottom-4 left-4 right-4 z-20 flex justify-between items-end">
               <div className="bg-black/60 backdrop-blur px-3 py-1.5 rounded-lg border border-white/10 text-xs">
                  <div className="flex items-center gap-2 mb-1"><Eye className="w-3 h-3 text-blue-400" /> <span>FPS: <span className="font-mono text-white">{videoFps}</span></span></div>
                  <div className="flex items-center gap-2"><Signal className="w-3 h-3 text-purple-400" /> <span>Signal: <span className="font-mono text-white">{telemetry.signal_strength}dB</span></span></div>
               </div>
               <div ref={commandFeedbackRef} className="bg-black/60 backdrop-blur px-4 py-2 rounded-lg border border-white/10 font-bold text-lg text-white tracking-widest uppercase opacity-0 transition-opacity duration-300"></div>
             </div>
             <button onClick={() => setShowFullscreenVideo(true)} className="absolute top-4 right-4 p-2 bg-black/60 hover:bg-black/80 rounded-lg backdrop-blur text-white z-20 opacity-0 group-hover:opacity-100 transition-all duration-300 hover:scale-110"><Maximize2 className="w-5 h-5" /></button>
        </div>

        {/* ZONE B: CONTROLS */}
        <div className="xl:col-span-5 flex flex-col gap-6">
            <div className="bg-gray-800/40 p-6 rounded-2xl border border-gray-700/50 shadow-xl flex-1 flex flex-col justify-center relative overflow-hidden">
                <div className="absolute inset-0 bg-gradient-to-br from-blue-500/5 to-purple-500/5 rounded-2xl pointer-events-none" />
                <div className="flex justify-between items-center mb-6 relative z-10">
                    <h2 className="text-lg font-semibold flex items-center gap-2"><Zap className="w-5 h-5 text-yellow-400" /> Controls</h2>
                    <button onClick={toggleAuto} className={`px-4 py-2 rounded-lg font-bold text-xs tracking-wider transition-all flex items-center gap-2 ${telemetry.auto_mode ? 'bg-purple-600 hover:bg-purple-500 text-white shadow-lg shadow-purple-500/30' : 'bg-gray-700 hover:bg-gray-600 text-gray-300 border border-gray-600'}`}><Bot className="w-4 h-4" />{telemetry.auto_mode ? 'DISABLE AUTO' : 'ENABLE AUTO'}</button>
                </div>
                <div className="grid grid-cols-3 gap-3 max-w-[220px] mx-auto mb-6 relative z-10">
                    <div></div>
                    <button onMouseDown={moveForward} onMouseUp={stopRobot} onTouchStart={moveForward} onTouchEnd={stopRobot} className="h-14 bg-blue-600 hover:bg-blue-500 rounded-xl shadow-lg shadow-blue-900/40 flex items-center justify-center active:scale-95 transition-all"><ArrowUp className="w-6 h-6"/></button>
                    <div></div>
                    <button onMouseDown={turnLeft} onMouseUp={stopRobot} onTouchStart={turnLeft} onTouchEnd={stopRobot} className="h-14 bg-gray-700 hover:bg-gray-600 rounded-xl flex items-center justify-center active:scale-95 transition-all"><ArrowLeft className="w-6 h-6"/></button>
                    <button onClick={stopRobot} className="h-14 bg-red-900/20 border border-red-500/50 text-red-500 hover:bg-red-900/40 rounded-xl flex items-center justify-center active:scale-95 transition-all"><StopCircle className="w-6 h-6"/></button>
                    <button onMouseDown={turnRight} onMouseUp={stopRobot} onTouchStart={turnRight} onTouchEnd={stopRobot} className="h-14 bg-gray-700 hover:bg-gray-600 rounded-xl flex items-center justify-center active:scale-95 transition-all"><ArrowRight className="w-6 h-6"/></button>
                    <div></div>
                    <button onMouseDown={moveBackward} onMouseUp={stopRobot} onTouchStart={moveBackward} onTouchEnd={stopRobot} className="h-14 bg-gray-700 hover:bg-gray-600 rounded-xl flex items-center justify-center active:scale-95 transition-all"><ArrowDown className="w-6 h-6"/></button>
                    <div></div>
                </div>
                <button onClick={emergencyStop} className={`w-full py-3 rounded-xl font-bold tracking-widest transition-all relative z-10 ${telemetry.emergency ? 'bg-yellow-500 text-black animate-pulse' : 'bg-red-600 hover:bg-red-500 text-white shadow-lg shadow-red-900/30'}`}>{telemetry.emergency ? 'RESET SYSTEM' : 'EMERGENCY STOP'}</button>
            </div>
            <div className="bg-gray-800/40 rounded-2xl border border-gray-700/50 shadow-xl overflow-hidden">
                <SystemHealthPanel systemHealth={systemHealth} telemetry={telemetry} performanceMetrics={performanceMetrics} connectionStats={connectionStats} />
            </div>
        </div>

        {/* ZONE C: ANALYTICS */}
        {showAnalytics && (
             <div className="xl:col-span-12 grid grid-cols-1 md:grid-cols-4 gap-6 animate-slide-up">
                 <TelemetryChart data={batteryHistory} title="Voltage" color="#10b981" icon={Battery} unit="V" />
                 <TelemetryChart data={gasHistory} title="Gas Level" color="#f59e0b" icon={Wind} unit="" />
                 <TelemetryChart data={distanceHistory} title="Distance" color="#3b82f6" icon={Gauge} unit="cm" />
                 <div className="bg-gray-800 p-4 rounded-xl border border-gray-700"><DataExportPanel telemetry={telemetry} waypoints={[]} alerts={alerts} connectionStats={connectionStats} /></div>
             </div>
        )}
        
        {/* ALERTS */}
        {alerts.length > 0 && (
          <div className="fixed bottom-6 right-6 flex flex-col gap-2 pointer-events-none z-50">
            {alerts.map(alert => (
              <div key={alert.id} className={`p-4 rounded-xl shadow-xl flex items-center gap-3 animate-slide-up ${alert.type === 'error' ? 'bg-red-600/90 text-white' : alert.type === 'success' ? 'bg-green-600/90 text-white' : 'bg-gray-800/90 text-white'}`}>
                {alert.type === 'error' ? <AlertCircle className="w-5 h-5"/> : <div className="w-2 h-2 rounded-full bg-white"/>}
                <span className="font-medium text-sm">{alert.message}</span>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
"""


def write_file(filepath, content):
    """Write content to file with error handling."""
    try:
        # Handle root directory files
        dirname = os.path.dirname(filepath)
        if dirname:
            os.makedirs(dirname, exist_ok=True)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"[OK] Generated: {filepath}")
        return True
    except Exception as e:
        print(f"[ERROR] Error writing {filepath}: {e}")
        return False


def build_all():
    """Generate all firmware files."""
    print("=" * 50)
    print("Project Nightfall - Firmware Builder V3.3")
    print("=" * 50)
    
    files_written = 0
    
    # 1. Generate platformio.ini
    if write_file('platformio.ini', platformio_ini):
        files_written += 1
    
    # 2. Generate Rear ESP32 firmware
    if write_file('src/main_rear_enhanced.cpp', rear_code):
        files_written += 1
    
    # 3. Generate Front ESP32 firmware
    if write_file('src/main_front_enhanced.cpp', front_code):
        files_written += 1
    
    # 4. Generate Camera firmware
    if write_file('src/main_camera.cpp', camera_code):
        files_written += 1
    
    # 5. Generate Dashboard JSX
    if write_file('robot-dashboard/src/DashboardEnhanced.jsx', dashboard_jsx):
        files_written += 1
    
    print("=" * 50)
    print(f"Build Complete! {files_written}/5 files generated.")
    print("=" * 50)
    return files_written


if __name__ == '__main__':
    build_all()