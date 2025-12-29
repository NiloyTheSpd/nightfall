#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>
#include "config.h"

class UltrasonicSensor
{
public:
    UltrasonicSensor(uint8_t trigPin, uint8_t echoPin);

    void begin();
    void update();
    float getDistance();
    bool isObstacleDetected(float threshold = SAFE_DISTANCE);
    bool isValidReading();
    unsigned long getLastReading();

    // Configuration
    void setMaxDistance(float maxDist);
    void setMinDistance(float minDist);
    void setTimeout(unsigned long timeout);

private:
    uint8_t _trigPin;
    uint8_t _echoPin;
    float _lastDistance;
    unsigned long _lastReading;
    bool _validReading;

    // Configuration
    float _maxDistance;
    float _minDistance;
    unsigned long _timeout;

    // Reading buffer for filtering
    static const int BUFFER_SIZE = 5;
    float _distanceBuffer[BUFFER_SIZE];
    int _bufferIndex;
    int _bufferCount;

    // Helper methods
    float measureDistance();
    float getMedianDistance();
    void addDistanceReading(float distance);
};

#endif // ULTRASONIC_SENSOR_H