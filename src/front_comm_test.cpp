/**
 * @file    main_front_enhanced.cpp
 * @brief   Project Nightfall - Optimized Front Slave (Fixed for V4.1 Master)
 */
#include <Arduino.h>
#include <ArduinoJson.h>

// --- পিন ম্যাপিং (আপনার কোড এবং Wire.txt অনুযায়ী) ---
// Front Left (M1_L)
#define M1_L_PWM 13
#define M1_L_IN1 23
#define M1_L_IN2 22

// Front Right (M1_R)
#define M1_R_PWM 25
#define M1_R_IN1 26
#define M1_R_IN2 27

// Middle Left (M2_L)
#define M2_L_PWM 14
#define M2_L_IN1 32
#define M2_L_IN2 33

// Middle Right (M2_R)
#define M2_R_PWM 18
#define M2_R_IN1 19
#define M2_R_IN2 21

// Communication
#define RXD2 16
#define TXD2 17

const unsigned long TIMEOUT_MS = 1000;

unsigned long lastSignalTime = 0;
int targetL = 0, targetR = 0;

void setMotor(int pwmPin, int in1, int in2, int speed);
void handleUART();
void emergencyStop();

void setup()
{
    Serial.begin(115200);
    // Rear ESP32 এর সাথে কথা বলার জন্য (RX=16, TX=17)
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    // পিন মোড সেটআপ
    pinMode(M1_L_PWM, OUTPUT);
    pinMode(M1_L_IN1, OUTPUT);
    pinMode(M1_L_IN2, OUTPUT);
    pinMode(M1_R_PWM, OUTPUT);
    pinMode(M1_R_IN1, OUTPUT);
    pinMode(M1_R_IN2, OUTPUT);

    pinMode(M2_L_PWM, OUTPUT);
    pinMode(M2_L_IN1, OUTPUT);
    pinMode(M2_L_IN2, OUTPUT);
    pinMode(M2_R_PWM, OUTPUT);
    pinMode(M2_R_IN1, OUTPUT);
    pinMode(M2_R_IN2, OUTPUT);

    emergencyStop();
    Serial.println("Front ESP32 Ready. Waiting for commands...");
}

void loop()
{
    unsigned long now = millis();
    handleUART();

    // ১ সেকেন্ড সিগন্যাল না পেলে অটোমেটিক স্টপ (সেফটি)
    if (now - lastSignalTime > TIMEOUT_MS)
    {
        emergencyStop();
    }
    else
    {
        // সব চাকা (সামনের + মাঝখানের) একসাথে ঘুরবে
        setMotor(M1_L_PWM, M1_L_IN1, M1_L_IN2, targetL); // Front Left
        setMotor(M1_R_PWM, M1_R_IN1, M1_R_IN2, targetR); // Front Right
        setMotor(M2_L_PWM, M2_L_IN1, M2_L_IN2, targetL); // Middle Left
        setMotor(M2_R_PWM, M2_R_IN1, M2_R_IN2, targetR); // Middle Right
    }
}

void handleUART()
{
    if (Serial2.available())
    {
        String data = Serial2.readStringUntil('\n');

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);

        if (!err)
        {
            lastSignalTime = millis();
            // শুধু L এবং R রিড করছি, যা সব মোটরে যাবে
            targetL = doc["L"] | 0;
            targetR = doc["R"] | 0;
        }
    }
}

void setMotor(int pwmPin, int in1, int in2, int speed)
{
    // স্পিড লিমিট চেক
    speed = constrain(speed, -255, 255);

    if (speed > 0)
    { // সামনে
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(pwmPin, speed);
    }
    else if (speed < 0)
    { // পেছনে
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(pwmPin, abs(speed));
    }
    else
    { // থামা
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(pwmPin, 0);
    }
}

void emergencyStop()
{
    targetL = 0;
    targetR = 0;
    setMotor(M1_L_PWM, M1_L_IN1, M1_L_IN2, 0);
    setMotor(M1_R_PWM, M1_R_IN1, M1_R_IN2, 0);
    setMotor(M2_L_PWM, M2_L_IN1, M2_L_IN2, 0);
    setMotor(M2_R_PWM, M2_R_IN1, M2_R_IN2, 0);
}