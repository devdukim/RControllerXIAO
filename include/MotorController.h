#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Adafruit_PWMServoDriver.h>
#include "config.h"

class MotorController {
private:
    Adafruit_PWMServoDriver* pwm;
    int currentSpeed;
    Direction currentDirection;
    bool isRunning;
    
public:
    MotorController();
    ~MotorController();
    
    // 초기화
    bool initialize();
    
    // 개별 모터 제어
    void setMotor(MotorIndex motorIndex, int speed);
    
    // 메카넘 휠 제어
    void setMecanumMotors(int frontLeft, int frontRight, int rearLeft, int rearRight);
    
    // 방향별 이동
    void moveForward();
    void moveBackward();
    void moveLeft();
    void moveRight();
    void rotateLeft();
    void rotateRight();
    void moveDiagonalFL();
    void moveDiagonalFR();
    void stop();
    
    // 속도 관리
    void setSpeed(int speed);  // 0-100%
    int getSpeed() const;
    
    // 상태 확인
    Direction getCurrentDirection() const;
    bool isMotorRunning() const;
    
    // 방향을 문자열로 변환
    String directionToString(Direction dir) const;
    Direction stringToDirection(const String& dirStr) const;
};

#endif // MOTOR_CONTROLLER_H