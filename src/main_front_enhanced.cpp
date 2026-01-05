/**
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
