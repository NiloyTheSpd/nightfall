#include "MotorControl.h"

MotorControl::MotorControl()
    : _enaPin(PIN_MOTOR_ENA),
      _in1Pin(PIN_MOTOR_IN1),
      _in2Pin(PIN_MOTOR_IN2),
      _enbPin(PIN_MOTOR_ENB),
      _in3Pin(PIN_MOTOR_IN3),
      _in4Pin(PIN_MOTOR_IN4),
      _currentLeftSpeed(0),
      _currentRightSpeed(0),
      _targetLeftSpeed(0),
      _targetRightSpeed(0),
      _emergencyStop(false),
      _lastUpdate(0),
      _currentPin1(34), // Analog pin for current sensing (optional)
      _currentPin2(35), // Analog pin for current sensing (optional)
      _leftCurrent(0.0),
      _rightCurrent(0.0),
      _maxSpeed(MAX_MOTOR_SPEED),
      _climbSpeed(CLIMB_MOTOR_SPEED),
      _speedRamp(MOTOR_SPEED_RAMP)
{
}

void MotorControl::begin()
{
    // Initialize motor control pins
    pinMode(_enaPin, OUTPUT);
    pinMode(_in1Pin, OUTPUT);
    pinMode(_in2Pin, OUTPUT);
    pinMode(_enbPin, OUTPUT);
    pinMode(_in3Pin, OUTPUT);
    pinMode(_in4Pin, OUTPUT);

    // Initialize current sensing pins (optional)
    pinMode(_currentPin1, INPUT);
    pinMode(_currentPin2, INPUT);

    // Stop all motors
    stop();

    DEBUG_PRINTLN("Motor Control initialized");
    DEBUG_PRINT("Max speed: ");
    DEBUG_PRINTLN(_maxSpeed);
    DEBUG_PRINT("Climb speed: ");
    DEBUG_PRINTLN(_climbSpeed);
}

void MotorControl::update()
{
    unsigned long now = millis();

    // Update at specified interval
    if (now - _lastUpdate < MOTOR_RESPONSE_TIME)
    {
        return;
    }

    _lastUpdate = now;

    // Apply speed ramping for smooth movement
    applySpeedRamp();

    // Update current sensing
    updateCurrentSensing();
}

void MotorControl::stop()
{
    _targetLeftSpeed = 0;
    _targetRightSpeed = 0;
    _emergencyStop = false;

    DEBUG_PRINTLN("Motors stopped");
}

void MotorControl::forward(uint8_t speed)
{
    if (_emergencyStop)
    {
        return;
    }

    _targetLeftSpeed = constrain(speed, 0, _maxSpeed);
    _targetRightSpeed = constrain(speed, 0, _maxSpeed);

    DEBUG_PRINT("Moving forward at speed: ");
    DEBUG_PRINTLN(speed);
}

void MotorControl::backward(uint8_t speed)
{
    if (_emergencyStop)
    {
        return;
    }

    _targetLeftSpeed = -constrain(speed, 0, _maxSpeed);
    _targetRightSpeed = -constrain(speed, 0, _maxSpeed);

    DEBUG_PRINT("Moving backward at speed: ");
    DEBUG_PRINTLN(speed);
}

void MotorControl::turnLeft(uint8_t speed)
{
    if (_emergencyStop)
    {
        return;
    }

    _targetLeftSpeed = -constrain(speed, 0, _maxSpeed);
    _targetRightSpeed = constrain(speed, 0, _maxSpeed);

    DEBUG_PRINT("Turning left at speed: ");
    DEBUG_PRINTLN(speed);
}

void MotorControl::turnRight(uint8_t speed)
{
    if (_emergencyStop)
    {
        return;
    }

    _targetLeftSpeed = constrain(speed, 0, _maxSpeed);
    _targetRightSpeed = -constrain(speed, 0, _maxSpeed);

    DEBUG_PRINT("Turning right at speed: ");
    DEBUG_PRINTLN(speed);
}

void MotorControl::climb(uint8_t speed)
{
    if (_emergencyStop)
    {
        return;
    }

    // Boost front motors for climbing
    _targetLeftSpeed = constrain(speed, 0, _climbSpeed);
    _targetRightSpeed = constrain(speed, 0, _climbSpeed);

    DEBUG_PRINT("Climbing at speed: ");
    DEBUG_PRINTLN(speed);
}

void MotorControl::setSpeeds(int leftSpeed, int rightSpeed)
{
    if (_emergencyStop)
    {
        return;
    }

    _targetLeftSpeed = constrain(leftSpeed, -_maxSpeed, _maxSpeed);
    _targetRightSpeed = constrain(rightSpeed, -_maxSpeed, _maxSpeed);

    DEBUG_PRINT("Speeds set - Left: ");
    DEBUG_PRINT(leftSpeed);
    DEBUG_PRINT(", Right: ");
    DEBUG_PRINTLN(rightSpeed);
}

void MotorControl::differentialDrive(int leftSpeed, int rightSpeed)
{
    setSpeeds(leftSpeed, rightSpeed);
}

void MotorControl::emergencyStop()
{
    _targetLeftSpeed = 0;
    _targetRightSpeed = 0;
    _currentLeftSpeed = 0;
    _currentRightSpeed = 0;
    _emergencyStop = true;

    // Immediately stop motors
    setLeftMotor(0);
    setRightMotor(0);

    DEBUG_PRINTLN("EMERGENCY STOP - Motors halted!");
}

bool MotorControl::isMoving()
{
    return (abs(_currentLeftSpeed) > 5 || abs(_currentRightSpeed) > 5);
}

int MotorControl::getLeftSpeed()
{
    return _currentLeftSpeed;
}

int MotorControl::getRightSpeed()
{
    return _currentRightSpeed;
}

float MotorControl::getLeftCurrent()
{
    return _leftCurrent;
}

float MotorControl::getRightCurrent()
{
    return _rightCurrent;
}

void MotorControl::setMaxSpeed(uint8_t speed)
{
    _maxSpeed = constrain(speed, 50, 255);
    DEBUG_PRINT("Max speed updated: ");
    DEBUG_PRINTLN(_maxSpeed);
}

void MotorControl::setClimbSpeed(uint8_t speed)
{
    _climbSpeed = constrain(speed, _maxSpeed, 255);
    DEBUG_PRINT("Climb speed updated: ");
    DEBUG_PRINTLN(_climbSpeed);
}

void MotorControl::setLeftMotor(int speed)
{
    // Speed = 0 to 255 for forward, 0 to -255 for reverse
    int pwmSpeed = constrain(abs(speed), 0, 255);

    if (speed >= 0)
    {
        // Forward
        digitalWrite(_in1Pin, HIGH);
        digitalWrite(_in2Pin, LOW);
    }
    else
    {
        // Reverse
        digitalWrite(_in1Pin, LOW);
        digitalWrite(_in2Pin, HIGH);
    }

    analogWrite(_enaPin, pwmSpeed);
}

void MotorControl::setRightMotor(int speed)
{
    // Speed = 0 to 255 for forward, 0 to -255 for reverse
    int pwmSpeed = constrain(abs(speed), 0, 255);

    if (speed >= 0)
    {
        // Forward
        digitalWrite(_in3Pin, HIGH);
        digitalWrite(_in4Pin, LOW);
    }
    else
    {
        // Reverse
        digitalWrite(_in3Pin, LOW);
        digitalWrite(_in4Pin, HIGH);
    }

    analogWrite(_enbPin, pwmSpeed);
}

void MotorControl::updateCurrentSensing()
{
    // Read current from analog pins (if current sensors are connected)
    // This is a simplified implementation - actual current sensing would require
    // appropriate circuitry (shunt resistors, amplifiers, etc.)

    int adc1 = analogRead(_currentPin1);
    int adc2 = analogRead(_currentPin2);

    // Convert ADC readings to current (example calculation)
    // This would need calibration based on actual hardware
    _leftCurrent = (adc1 * 3.3 / 4095.0) * 0.5;  // Example conversion
    _rightCurrent = (adc2 * 3.3 / 4095.0) * 0.5; // Example conversion
}

void MotorControl::applySpeedRamp()
{
    // Ramp speeds for smooth movement
    if (abs(_targetLeftSpeed - _currentLeftSpeed) <= _speedRamp)
    {
        _currentLeftSpeed = _targetLeftSpeed;
    }
    else if (_targetLeftSpeed > _currentLeftSpeed)
    {
        _currentLeftSpeed += _speedRamp;
    }
    else if (_targetLeftSpeed < _currentLeftSpeed)
    {
        _currentLeftSpeed -= _speedRamp;
    }

    if (abs(_targetRightSpeed - _currentRightSpeed) <= _speedRamp)
    {
        _currentRightSpeed = _targetRightSpeed;
    }
    else if (_targetRightSpeed > _currentRightSpeed)
    {
        _currentRightSpeed += _speedRamp;
    }
    else if (_targetRightSpeed < _currentRightSpeed)
    {
        _currentRightSpeed -= _speedRamp;
    }

    // Apply speeds to motors
    if (!_emergencyStop)
    {
        setLeftMotor(_currentLeftSpeed);
        setRightMotor(_currentRightSpeed);
    }
}