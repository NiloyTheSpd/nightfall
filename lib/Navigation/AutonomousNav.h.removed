#ifndef AUTONOMOUS_NAV_H
#define AUTONOMOUS_NAV_H

#include <Arduino.h>
#include "config.h"

struct NavigationCommand
{
    MovementCommand command;
    uint8_t speed;
    unsigned long duration;
    bool isValid;
};

class AutonomousNav
{
public:
    AutonomousNav();

    void begin();
    void update();

    // Main navigation control
    NavigationCommand getNextCommand();
    NavigationState getCurrentState();

    // Sensor data input
    void updateSensorData(float frontDistance, float rearDistance,
                          int gasLevel, bool mlDetection, float mlConfidence,
                          MLObjectClass mlObject);

    // State management
    void setState(NavigationState newState);
    void resetNavigation();

    // Configuration
    void setObstacleThreshold(float threshold);
    void setSafeDistance(float distance);
    void setClimbThreshold(float threshold);
    void setBackupDistance(float distance);

    // Statistics
    unsigned long getNavigationTime();
    int getStateChangeCount();
    float getAverageSpeed();

private:
    NavigationState _currentState;
    unsigned long _stateStartTime;
    unsigned long _navigationStartTime;
    int _stateChangeCount;

    // Sensor data
    float _frontDistance;
    float _rearDistance;
    int _gasLevel;
    bool _mlDetection;
    float _mlConfidence;
    MLObjectClass _mlObject;

    // Navigation parameters
    float _obstacleThreshold;
    float _safeDistance;
    float _climbThreshold;
    float _backupDistance;

    // Movement tracking
    int _leftSpeed;
    int _rightSpeed;
    unsigned long _lastSpeedUpdate;

    // Stuck detection
    int _stuckCounter;
    unsigned long _lastSignificantMovement;
    unsigned long _movementTimeout;

    // Scanning state
    int _scanAngle;
    int _scanDirection;
    bool _scanComplete;

    // Helper methods
    NavigationCommand handleInitState();
    NavigationCommand handleForwardState();
    NavigationCommand handleAvoidState();
    NavigationCommand handleTurnState();
    NavigationCommand handleClimbState();
    NavigationCommand handleBackupState();
    NavigationCommand handleScanState();
    NavigationCommand handleStuckState();

    bool isStuck();
    void detectClimbing();
    bool shouldClimb();
    bool isPathClear();
    void updateMovementTracking();
};

#endif // AUTONOMOUS_NAV_H