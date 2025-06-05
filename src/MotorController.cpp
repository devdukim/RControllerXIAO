#include "MotorController.h"

MotorController::MotorController() 
    : pwm(nullptr), currentSpeed(PWM_HALF), currentDirection(DIR_STOP), isRunning(false) {
}

MotorController::~MotorController() {
    if (pwm) {
        delete pwm;
    }
}

bool MotorController::initialize() {
    pwm = new Adafruit_PWMServoDriver();
    if (!pwm) {
        return false;
    }
    
    pwm->begin();
    pwm->setPWMFreq(PWM_FREQUENCY);
    
    // 모든 모터 정지 상태로 초기화
    setMecanumMotors(0, 0, 0, 0);
    
    Serial.println("Motor controller initialized successfully");
    return true;
}

void MotorController::setMotor(MotorIndex motorIndex, int speed) {
    int speedChannel, dirAChannel, dirBChannel;
    String motorName;
    
    switch(motorIndex) {
        case MOTOR_FRONT_LEFT:
            speedChannel = MOTOR_FL_SPEED;
            dirAChannel = MOTOR_FL_DIR_A;
            dirBChannel = MOTOR_FL_DIR_B;
            motorName = "FL";
            break;
        case MOTOR_FRONT_RIGHT:
            speedChannel = MOTOR_FR_SPEED;
            dirAChannel = MOTOR_FR_DIR_A;
            dirBChannel = MOTOR_FR_DIR_B;
            motorName = "FR";
            break;
        case MOTOR_REAR_LEFT:
            speedChannel = MOTOR_RL_SPEED;
            dirAChannel = MOTOR_RL_DIR_A;
            dirBChannel = MOTOR_RL_DIR_B;
            motorName = "RL";
            break;
        case MOTOR_REAR_RIGHT:
            speedChannel = MOTOR_RR_SPEED;
            dirAChannel = MOTOR_RR_DIR_A;
            dirBChannel = MOTOR_RR_DIR_B;
            motorName = "RR";
            break;
        default:
            Serial.println("Invalid motor index");
            return;
    }
    
    int pwmSpeed = abs(speed);
    if (pwmSpeed > PWM_MAX) pwmSpeed = PWM_MAX;
    
    if (speed > 0) {
        // 정방향
        pwm->setPWM(dirAChannel, 0, PWM_MAX);
        pwm->setPWM(dirBChannel, 0, 0);
    } else if (speed < 0) {
        // 역방향
        pwm->setPWM(dirAChannel, 0, 0);
        pwm->setPWM(dirBChannel, 0, PWM_MAX);
    } else {
        // 정지
        pwm->setPWM(dirAChannel, 0, 0);
        pwm->setPWM(dirBChannel, 0, 0);
    }
    
    // 속도 설정
    pwm->setPWM(speedChannel, 0, pwmSpeed);
    
    Serial.print(motorName);
    Serial.print(" Motor: ");
    Serial.println(speed);
}

void MotorController::setMecanumMotors(int frontLeft, int frontRight, int rearLeft, int rearRight) {
    // 전방 좌측 모터
    if (frontLeft > 0) {
        pwm->setPWM(MOTOR_FL_DIR_A, 0, PWM_MAX);
        pwm->setPWM(MOTOR_FL_DIR_B, 0, 0);
    } else if (frontLeft < 0) {
        pwm->setPWM(MOTOR_FL_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_FL_DIR_B, 0, PWM_MAX);
    } else {
        pwm->setPWM(MOTOR_FL_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_FL_DIR_B, 0, 0);
    }
    
    // 전방 우측 모터
    if (frontRight > 0) {
        pwm->setPWM(MOTOR_FR_DIR_A, 0, PWM_MAX);
        pwm->setPWM(MOTOR_FR_DIR_B, 0, 0);
    } else if (frontRight < 0) {
        pwm->setPWM(MOTOR_FR_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_FR_DIR_B, 0, PWM_MAX);
    } else {
        pwm->setPWM(MOTOR_FR_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_FR_DIR_B, 0, 0);
    }
    
    // 후방 좌측 모터
    if (rearLeft > 0) {
        pwm->setPWM(MOTOR_RL_DIR_A, 0, PWM_MAX);
        pwm->setPWM(MOTOR_RL_DIR_B, 0, 0);
    } else if (rearLeft < 0) {
        pwm->setPWM(MOTOR_RL_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_RL_DIR_B, 0, PWM_MAX);
    } else {
        pwm->setPWM(MOTOR_RL_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_RL_DIR_B, 0, 0);
    }
    
    // 후방 우측 모터
    if (rearRight > 0) {
        pwm->setPWM(MOTOR_RR_DIR_A, 0, PWM_MAX);
        pwm->setPWM(MOTOR_RR_DIR_B, 0, 0);
    } else if (rearRight < 0) {
        pwm->setPWM(MOTOR_RR_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_RR_DIR_B, 0, PWM_MAX);
    } else {
        pwm->setPWM(MOTOR_RR_DIR_A, 0, 0);
        pwm->setPWM(MOTOR_RR_DIR_B, 0, 0);
    }
    
    // 속도 설정 (동시에)
    int pwmFL = abs(frontLeft);
    int pwmFR = abs(frontRight);
    int pwmRL = abs(rearLeft);
    int pwmRR = abs(rearRight);
    
    if (pwmFL > PWM_MAX) pwmFL = PWM_MAX;
    if (pwmFR > PWM_MAX) pwmFR = PWM_MAX;
    if (pwmRL > PWM_MAX) pwmRL = PWM_MAX;
    if (pwmRR > PWM_MAX) pwmRR = PWM_MAX;
    
    pwm->setPWM(MOTOR_FL_SPEED, 0, pwmFL);
    pwm->setPWM(MOTOR_FR_SPEED, 0, pwmFR);
    pwm->setPWM(MOTOR_RL_SPEED, 0, pwmRL);
    pwm->setPWM(MOTOR_RR_SPEED, 0, pwmRR);
    
    isRunning = (frontLeft != 0 || frontRight != 0 || rearLeft != 0 || rearRight != 0);
    
    Serial.print("Mecanum Motors - FL:");
    Serial.print(frontLeft);
    Serial.print(" FR:");
    Serial.print(frontRight);
    Serial.print(" RL:");
    Serial.print(rearLeft);
    Serial.print(" RR:");
    Serial.println(rearRight);
}

void MotorController::moveForward() {
    setMecanumMotors(currentSpeed, currentSpeed, currentSpeed, currentSpeed);
    currentDirection = DIR_FORWARD;
}

void MotorController::moveBackward() {
    setMecanumMotors(-currentSpeed, -currentSpeed, -currentSpeed, -currentSpeed);
    currentDirection = DIR_BACKWARD;
}

void MotorController::moveLeft() {
    setMecanumMotors(-currentSpeed, currentSpeed, currentSpeed, -currentSpeed);
    currentDirection = DIR_LEFT;
}

void MotorController::moveRight() {
    setMecanumMotors(currentSpeed, -currentSpeed, -currentSpeed, currentSpeed);
    currentDirection = DIR_RIGHT;
}

void MotorController::rotateLeft() {
    setMecanumMotors(-currentSpeed, currentSpeed, -currentSpeed, currentSpeed);
    currentDirection = DIR_ROTATE_LEFT;
}

void MotorController::rotateRight() {
    setMecanumMotors(currentSpeed, -currentSpeed, currentSpeed, -currentSpeed);
    currentDirection = DIR_ROTATE_RIGHT;
}

void MotorController::moveDiagonalFL() {
    setMecanumMotors(0, currentSpeed, currentSpeed, 0);
    currentDirection = DIR_DIAGONAL_FL;
}

void MotorController::moveDiagonalFR() {
    setMecanumMotors(currentSpeed, 0, 0, currentSpeed);
    currentDirection = DIR_DIAGONAL_FR;
}

void MotorController::stop() {
    setMecanumMotors(0, 0, 0, 0);
    currentDirection = DIR_STOP;
    isRunning = false;
}

void MotorController::setSpeed(int speed) {
    // 0-100% 범위를 PWM 값으로 변환
    currentSpeed = map(constrain(speed, 0, 100), 0, 100, 0, PWM_MAX);
    Serial.print("Speed set to: ");
    Serial.print(speed);
    Serial.print("% (PWM: ");
    Serial.print(currentSpeed);
    Serial.println(")");
}

int MotorController::getSpeed() const {
    // PWM 값을 0-100% 범위로 변환
    return map(currentSpeed, 0, PWM_MAX, 0, 100);
}

Direction MotorController::getCurrentDirection() const {
    return currentDirection;
}

bool MotorController::isMotorRunning() const {
    return isRunning;
}

String MotorController::directionToString(Direction dir) const {
    switch(dir) {
        case DIR_STOP: return "STOP";
        case DIR_FORWARD: return "FORWARD";
        case DIR_BACKWARD: return "BACKWARD";
        case DIR_LEFT: return "LEFT";
        case DIR_RIGHT: return "RIGHT";
        case DIR_ROTATE_LEFT: return "ROT_LEFT";
        case DIR_ROTATE_RIGHT: return "ROT_RIGHT";
        case DIR_DIAGONAL_FL: return "DIAG_FL";
        case DIR_DIAGONAL_FR: return "DIAG_FR";
        default: return "UNKNOWN";
    }
}

Direction MotorController::stringToDirection(const String& dirStr) const {
    String dir = dirStr;
    dir.toLowerCase();
    
    if (dir == "stop") return DIR_STOP;
    else if (dir == "forward") return DIR_FORWARD;
    else if (dir == "backward") return DIR_BACKWARD;
    else if (dir == "left") return DIR_LEFT;
    else if (dir == "right") return DIR_RIGHT;
    else if (dir == "rotate_left" || dir == "rot_left") return DIR_ROTATE_LEFT;
    else if (dir == "rotate_right" || dir == "rot_right") return DIR_ROTATE_RIGHT;
    else if (dir == "diagonal_fl" || dir == "diag_fl") return DIR_DIAGONAL_FL;
    else if (dir == "diagonal_fr" || dir == "diag_fr") return DIR_DIAGONAL_FR;
    else return DIR_STOP;
}