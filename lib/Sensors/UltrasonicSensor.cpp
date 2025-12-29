#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(uint8_t trigPin, uint8_t echoPin)
    : _trigPin(trigPin), _echoPin(echoPin),
      _lastDistance(0.0), _lastReading(0),
      _validReading(false), _maxDistance(400.0),
      _minDistance(2.0), _timeout(ULTRASONIC_TIMEOUT * 1000),
      _bufferIndex(0), _bufferCount(0)
{

    // Initialize distance buffer
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        _distanceBuffer[i] = 0.0;
    }
}

void UltrasonicSensor::begin()
{
    // Initialize pins
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);

    // Ensure trigger pin is low
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    // Initial measurement
    measureDistance();

    DEBUG_PRINTLN("Ultrasonic sensor initialized");
    DEBUG_PRINT("Trig pin: ");
    DEBUG_PRINTLN(_trigPin);
    DEBUG_PRINT("Echo pin: ");
    DEBUG_PRINTLN(_echoPin);
}

void UltrasonicSensor::update()
{
    // Perform new measurement
    float newDistance = measureDistance();

    if (newDistance > 0)
    {
        _validReading = true;
        _lastDistance = newDistance;
        _lastReading = millis();

        // Add to buffer for filtering
        addDistanceReading(newDistance);
    }
    else
    {
        _validReading = false;
    }
}

float UltrasonicSensor::getDistance()
{
    // Return filtered distance
    return getMedianDistance();
}

bool UltrasonicSensor::isObstacleDetected(float threshold)
{
    float distance = getDistance();

    if (distance > 0 && distance < threshold)
    {
        return true;
    }

    return false;
}

bool UltrasonicSensor::isValidReading()
{
    return _validReading;
}

unsigned long UltrasonicSensor::getLastReading()
{
    return _lastReading;
}

void UltrasonicSensor::setMaxDistance(float maxDist)
{
    _maxDistance = maxDist;
    DEBUG_PRINT("Max distance updated: ");
    DEBUG_PRINTLN(_maxDistance);
}

void UltrasonicSensor::setMinDistance(float minDist)
{
    _minDistance = minDist;
    DEBUG_PRINT("Min distance updated: ");
    DEBUG_PRINTLN(_minDistance);
}

void UltrasonicSensor::setTimeout(unsigned long timeout)
{
    _timeout = timeout;
    DEBUG_PRINT("Timeout updated: ");
    DEBUG_PRINTLN(_timeout);
}

float UltrasonicSensor::measureDistance()
{
    // Send 10 microsecond trigger pulse
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // Measure echo pulse duration
    unsigned long startTime = micros();
    while (digitalRead(_echoPin) == LOW && (micros() - startTime) < _timeout)
    {
        // Wait for echo to go high
    }

    if (digitalRead(_echoPin) == LOW)
    {
        // Timeout - no echo received
        return -1.0;
    }

    unsigned long echoStart = micros();
    while (digitalRead(_echoPin) == HIGH && (micros() - echoStart) < _timeout)
    {
        // Wait for echo to go low
    }

    if (digitalRead(_echoPin) == HIGH)
    {
        // Timeout - echo signal too long
        return -1.0;
    }

    unsigned long echoEnd = micros();
    unsigned long pulseDuration = echoEnd - echoStart;

    // Calculate distance in cm (speed of sound = 343 m/s = 0.0343 cm/μs)
    // Distance = (time * speed) / 2
    float distance = (pulseDuration * 0.0343) / 2.0;

    // Validate reading
    if (distance >= _minDistance && distance <= _maxDistance)
    {
        return distance;
    }
    else
    {
        return -1.0;
    }
}

float UltrasonicSensor::getMedianDistance()
{
    if (_bufferCount == 0)
    {
        return _lastDistance;
    }

    // Copy buffer for sorting
    float sortedDistances[BUFFER_SIZE];
    for (int i = 0; i < _bufferCount; i++)
    {
        sortedDistances[i] = _distanceBuffer[i];
    }

    // Simple bubble sort
    for (int i = 0; i < _bufferCount - 1; i++)
    {
        for (int j = 0; j < _bufferCount - i - 1; j++)
        {
            if (sortedDistances[j] > sortedDistances[j + 1])
            {
                float temp = sortedDistances[j];
                sortedDistances[j] = sortedDistances[j + 1];
                sortedDistances[j + 1] = temp;
            }
        }
    }

    // Return median
    int middle = _bufferCount / 2;
    if (_bufferCount % 2 == 0)
    {
        return (sortedDistances[middle - 1] + sortedDistances[middle]) / 2.0;
    }
    else
    {
        return sortedDistances[middle];
    }
}

void UltrasonicSensor::addDistanceReading(float distance)
{
    _distanceBuffer[_bufferIndex] = distance;
    _bufferIndex = (_bufferIndex + 1) % BUFFER_SIZE;

    if (_bufferCount < BUFFER_SIZE)
    {
        _bufferCount++;
    }
}