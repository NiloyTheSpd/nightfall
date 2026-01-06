/**
 * @file    main_front_enhanced.cpp
 * @brief   SLAVE (Front) - Receives commands & drives motors
 */
#include <Arduino.h>
#include <ArduinoJson.h>

// Front Motors (Wire.txt অনুযায়ী)
#define M1_ENA 13
#define M1_IN1 23
#define M1_IN2 22
#define M1_ENB 25
#define M1_IN3 26
#define M1_IN4 27

// Middle Motors
#define M2_ENA 14
#define M2_IN1 32
#define M2_IN2 33
#define M2_ENB 18
#define M2_IN3 19
#define M2_IN4 21

void setMotor(int en, int in1, int in2, int speed);

void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17); // Listen to Rear

    // Pin Setup
    pinMode(M1_ENA, OUTPUT);
    pinMode(M1_IN1, OUTPUT);
    pinMode(M1_IN2, OUTPUT);
    pinMode(M1_ENB, OUTPUT);
    pinMode(M1_IN3, OUTPUT);
    pinMode(M1_IN4, OUTPUT);
    pinMode(M2_ENA, OUTPUT);
    pinMode(M2_IN1, OUTPUT);
    pinMode(M2_IN2, OUTPUT);
    pinMode(M2_ENB, OUTPUT);
    pinMode(M2_IN3, OUTPUT);
    pinMode(M2_IN4, OUTPUT);
}

void loop()
{
    if (Serial2.available())
    {
        String data = Serial2.readStringUntil('\n');
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);

        if (!err)
        {
            int L = doc["L"];
            int R = doc["R"];

            // Front Wheels
            setMotor(M1_ENA, M1_IN1, M1_IN2, L);
            setMotor(M1_ENB, M1_IN3, M1_IN4, R);

            // Middle Wheels (Copy logic)
            setMotor(M2_ENA, M2_IN1, M2_IN2, L);
            setMotor(M2_ENB, M2_IN3, M2_IN4, R);
        }
    }
}

void setMotor(int en, int in1, int in2, int speed)
{
    if (speed > 0)
    {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(en, speed);
    }
    else if (speed < 0)
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(en, abs(speed));
    }
    else
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(en, 0);
    }
}