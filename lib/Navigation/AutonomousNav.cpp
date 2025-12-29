#include "AutonomousNav.h"

AutonomousNav::AutonomousNav()
    : _currentState(NAV_INIT),
      _stateStartTime(0),
      _navigationStartTime(0),
      _stateChangeCount(0),
      _frontDistance(0.0),
      _rearDistance(0.0),
      _gasLevel(0),
      _mlDetection(false),
      _mlConfidence(0.0),
      _mlObject(ML_UNKNOWN),
      _obstacleThreshold(SAFE_DISTANCE),
      _safeDistance(SAFE_DISTANCE),
      _climbThreshold(10.0),
      _backupDistance(30.0),
      _leftSpeed(0),
      _rightSpeed(0),
      _lastSpeedUpdate(0),
      _stuckCounter(0),
      _lastSignificantMovement(0),
      _movementTimeout(10000),
      _scanAngle(0),
      _scanDirection(1),
      _scanComplete(false) {}

void AutonomousNav::begin()
{
    resetNavigation();

    DEBUG_PRINTLN("Autonomous Navigation initialized");
    DEBUG_PRINT("Obstacle threshold: ");
    DEBUG_PRINTLN(_obstacleThreshold);
    DEBUG_PRINT("Safe distance: ");
    DEBUG_PRINTLN(_safeDistance);
}

void AutonomousNav::update()
{
    unsigned long now = millis();

    // Update movement tracking
    updateMovementTracking();

    // Check for stuck condition
    if (isStuck())
    {
        setState(NAV_STUCK);
        return;
    }

    // Update state based on sensor data and current state
    switch (_currentState)
    {
    case NAV_INIT:
        setState(NAV_FORWARD);
        break;

    case NAV_FORWARD:
        if (_frontDistance > 0 && _frontDistance < _obstacleThreshold)
        {
            // Check if obstacle is climbable
            if (shouldClimb())
            {
                setState(NAV_CLIMB);
            }
            else
            {
                setState(NAV_AVOID);
            }
        }
        break;

    case NAV_AVOID:
        if (_frontDistance > _obstacleThreshold)
        {
            setState(NAV_FORWARD);
        }
        break;

    case NAV_TURN:
        if (millis() - _stateStartTime > TURN_DURATION)
        {
            setState(NAV_FORWARD);
        }
        break;

    case NAV_CLIMB:
        // Climbing logic would be more complex in real implementation
        if (millis() - _stateStartTime > 3000)
        { // 3 second climb attempt
            setState(NAV_FORWARD);
        }
        break;

    case NAV_BACKUP:
        if (millis() - _stateStartTime > BACKUP_DURATION)
        {
            setState(NAV_TURN);
        }
        break;

    case NAV_SCAN:
        if (_scanComplete)
        {
            setState(NAV_TURN);
        }
        break;

    case NAV_STUCK:
        if (millis() - _stateStartTime > 5000)
        { // 5 second timeout
            setState(NAV_BACKUP);
        }
        break;
    }
}

NavigationCommand AutonomousNav::getNextCommand()
{
    NavigationCommand cmd = {MOVE_STOP, 0, 0, false};

    switch (_currentState)
    {
    case NAV_INIT:
        cmd = handleInitState();
        break;

    case NAV_FORWARD:
        cmd = handleForwardState();
        break;

    case NAV_AVOID:
        cmd = handleAvoidState();
        break;

    case NAV_TURN:
        cmd = handleTurnState();
        break;

    case NAV_CLIMB:
        cmd = handleClimbState();
        break;

    case NAV_BACKUP:
        cmd = handleBackupState();
        break;

    case NAV_SCAN:
        cmd = handleScanState();
        break;

    case NAV_STUCK:
        cmd = handleStuckState();
        break;
    }

    return cmd;
}

NavigationState AutonomousNav::getCurrentState()
{
    return _currentState;
}

void AutonomousNav::updateSensorData(float frontDistance, float rearDistance,
                                     int gasLevel, bool mlDetection, float mlConfidence,
                                     MLObjectClass mlObject)
{
    _frontDistance = frontDistance;
    _rearDistance = rearDistance;
    _gasLevel = gasLevel;
    _mlDetection = mlDetection;
    _mlConfidence = mlConfidence;
    _mlObject = mlObject;
}

void AutonomousNav::setState(NavigationState newState)
{
    if (newState != _currentState)
    {
        _currentState = newState;
        _stateStartTime = millis();
        _stateChangeCount++;

        DEBUG_PRINT("State changed to: ");
        switch (newState)
        {
        case NAV_INIT:
            DEBUG_PRINTLN("INIT");
            break;
        case NAV_FORWARD:
            DEBUG_PRINTLN("FORWARD");
            break;
        case NAV_AVOID:
            DEBUG_PRINTLN("AVOID");
            break;
        case NAV_TURN:
            DEBUG_PRINTLN("TURN");
            break;
        case NAV_CLIMB:
            DEBUG_PRINTLN("CLIMB");
            break;
        case NAV_BACKUP:
            DEBUG_PRINTLN("BACKUP");
            break;
        case NAV_SCAN:
            DEBUG_PRINTLN("SCAN");
            break;
        case NAV_STUCK:
            DEBUG_PRINTLN("STUCK");
            break;
        }
    }
}

void AutonomousNav::resetNavigation()
{
    _currentState = NAV_INIT;
    _stateStartTime = millis();
    _navigationStartTime = millis();
    _stateChangeCount = 0;
    _stuckCounter = 0;
    _lastSignificantMovement = millis();
    _scanAngle = 0;
    _scanDirection = 1;
    _scanComplete = false;

    DEBUG_PRINTLN("Navigation reset");
}

void AutonomousNav::setObstacleThreshold(float threshold)
{
    _obstacleThreshold = threshold;
    DEBUG_PRINT("Obstacle threshold updated: ");
    DEBUG_PRINTLN(threshold);
}

void AutonomousNav::setSafeDistance(float distance)
{
    _safeDistance = distance;
    DEBUG_PRINT("Safe distance updated: ");
    DEBUG_PRINTLN(distance);
}

void AutonomousNav::setClimbThreshold(float threshold)
{
    _climbThreshold = threshold;
    DEBUG_PRINT("Climb threshold updated: ");
    DEBUG_PRINTLN(threshold);
}

void AutonomousNav::setBackupDistance(float distance)
{
    _backupDistance = distance;
    DEBUG_PRINT("Backup distance updated: ");
    DEBUG_PRINTLN(distance);
}

unsigned long AutonomousNav::getNavigationTime()
{
    return millis() - _navigationStartTime;
}

int AutonomousNav::getStateChangeCount()
{
    return _stateChangeCount;
}

float AutonomousNav::getAverageSpeed()
{
    return (abs(_leftSpeed) + abs(_rightSpeed)) / 2.0;
}

NavigationCommand AutonomousNav::handleInitState()
{
    NavigationCommand cmd = {MOVE_STOP, 0, 0, true};
    return cmd;
}

NavigationCommand AutonomousNav::handleForwardState()
{
    NavigationCommand cmd = {MOVE_FORWARD, MAX_MOTOR_SPEED, 0, true};
    _leftSpeed = MAX_MOTOR_SPEED;
    _rightSpeed = MAX_MOTOR_SPEED;
    return cmd;
}

NavigationCommand AutonomousNav::handleAvoidState()
{
    NavigationCommand cmd = {MOVE_STOP, 0, 0, true};
    _leftSpeed = 0;
    _rightSpeed = 0;
    return cmd;
}

NavigationCommand AutonomousNav::handleTurnState()
{
    // Simple turn - could be improved with better path planning
    NavigationCommand cmd = {MOVE_TURN_RIGHT, MAX_MOTOR_SPEED, TURN_DURATION, true};
    _leftSpeed = MAX_MOTOR_SPEED;
    _rightSpeed = -MAX_MOTOR_SPEED;
    return cmd;
}

NavigationCommand AutonomousNav::handleClimbState()
{
    NavigationCommand cmd = {MOVE_CLIMB, CLIMB_MOTOR_SPEED, 0, true};
    _leftSpeed = CLIMB_MOTOR_SPEED;
    _rightSpeed = CLIMB_MOTOR_SPEED;
    return cmd;
}

NavigationCommand AutonomousNav::handleBackupState()
{
    NavigationCommand cmd = {MOVE_BACKWARD, MAX_MOTOR_SPEED, BACKUP_DURATION, true};
    _leftSpeed = -MAX_MOTOR_SPEED;
    _rightSpeed = -MAX_MOTOR_SPEED;
    return cmd;
}

NavigationCommand AutonomousNav::handleScanState()
{
    NavigationCommand cmd = {MOVE_TURN_LEFT, 100, 0, true};
    _leftSpeed = -100;
    _rightSpeed = 100;

    _scanAngle += 10;
    if (_scanAngle >= 360)
    {
        _scanComplete = true;
    }

    return cmd;
}

NavigationCommand AutonomousNav::handleStuckState()
{
    NavigationCommand cmd = {MOVE_TURN_LEFT, MAX_MOTOR_SPEED, 0, true};
    _leftSpeed = -MAX_MOTOR_SPEED;
    _rightSpeed = MAX_MOTOR_SPEED;
    return cmd;
}

bool AutonomousNav::isStuck()
{
    // Simple stuck detection based on lack of significant movement
    if (millis() - _lastSignificantMovement > _movementTimeout)
    {
        _stuckCounter++;
        if (_stuckCounter > 3)
        {
            return true;
        }
    }
    return false;
}

void AutonomousNav::detectClimbing()
{
    // In a real implementation, this would use IMU data or other sensors
    // For now, it's a placeholder based on distance changes
}

bool AutonomousNav::shouldClimb()
{
    // Check if obstacle is low enough to climb
    // This would be enhanced with ML data and height estimation
    if (_mlDetection && _mlConfidence > ML_CONFIDENCE_THRESHOLD)
    {
        switch (_mlObject)
        {
        case ML_STAIRS:
        case ML_DOOR:
            return true;
        default:
            break;
        }
    }

    // Simple heuristic: if front distance suddenly decreases significantly
    static float lastFrontDistance = 0.0;
    if (lastFrontDistance > 0 && _frontDistance > 0)
    {
        float distanceChange = lastFrontDistance - _frontDistance;
        if (distanceChange > _climbThreshold)
        {
            return true;
        }
    }
    lastFrontDistance = _frontDistance;

    return false;
}

bool AutonomousNav::isPathClear()
{
    return (_frontDistance > _obstacleThreshold);
}

void AutonomousNav::updateMovementTracking()
{
    unsigned long now = millis();

    // Check for significant speed changes
    int currentSpeed = (abs(_leftSpeed) + abs(_rightSpeed)) / 2;

    if (currentSpeed > 50)
    { // Moving significantly
        _lastSignificantMovement = now;
        _stuckCounter = 0;
    }
}