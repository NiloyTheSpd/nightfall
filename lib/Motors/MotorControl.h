#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "config.h"

class MotorControl
{
public:
    MotorControl(uint8_t enaPin, uint8_t in1Pin, uint8_t in2Pin,
                 uint8_t enbPin, uint8_t in3Pin, uint8_t in4Pin);

    void begin();
    void update();

    // Basic movement commands
    void stop();
    void forward(uint8_t speed = MAX_MOTOR_SPEED);
    void backward(uint8_t speed = MAX_MOTOR_SPEED);
    void turnLeft(uint8_t speed = MAX_MOTOR_SPEED);
    void turnRight(uint8_t speed = MAX_MOTOR_SPEED);
    void climb(uint8_t speed = CLIMB_MOTOR_SPEED);

    // Advanced movement commands
    void setSpeeds(int leftSpeed, int rightSpeed);
    void differentialDrive(int leftSpeed, int rightSpeed);
    void emergencyStop();

    // Status and monitoring
    bool isMoving();
    int getLeftSpeed();
    int getRightSpeed();
    float getLeftCurrent();
    float getRightCurrent();

    // Configuration
    void setMaxSpeed(uint8_t speed);
    void setClimbSpeed(uint8_t speed);

private:
    // L298N pin assignments
    uint8_t _enaPin;
    uint8_t _in1Pin;
    uint8_t _in2Pin;
    uint8_t _enbPin;
    uint8_t _in3Pin;
    uint8_t _in4Pin;

    // Motor state
    int _currentLeftSpeed;
    int _currentRightSpeed;
    int _targetLeftSpeed;
    int _targetRightSpeed;
    bool _emergencyStop;
    unsigned long _lastUpdate;

    // Current sensing (analog pins)
    uint8_t _currentPin1;
    uint8_t _currentPin2;
    float _leftCurrent;
    float _rightCurrent;

    // Speed ramping
    uint8_t _maxSpeed;
    uint8_t _climbSpeed;
    uint8_t _speedRamp;

    // Helper methods
    void setLeftMotor(int speed);
    void setRightMotor(int speed);
    void updateCurrentSensing();
    void applySpeedRamp();
};

#endif // MOTOR_CONTROL_H