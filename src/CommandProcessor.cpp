#include "CommandProcessor.h"
#include "MotorController.h"
#include "EncoderManager.h"
#include "DisplayManager.h"

CommandProcessor::CommandProcessor() 
    : motorController(nullptr), encoderManager(nullptr), displayManager(nullptr), isAutoMode(false) {
}

CommandProcessor::~CommandProcessor() {
}

void CommandProcessor::setMotorController(MotorController* controller) {
    motorController = controller;
}

void CommandProcessor::setEncoderManager(EncoderManager* manager) {
    encoderManager = manager;
}

void CommandProcessor::setDisplayManager(DisplayManager* display) {
    displayManager = display;
}

void CommandProcessor::processCommand(const String& command) {
    String cmd = command;  // 복사본 생성
    cmd.trim();           // 복사본 수정
    cmd = toLowerCase(cmd);
    
    Serial.print("Processing command: ");
    Serial.println(cmd);
    
    // 디스플레이에 수신된 메시지 표시
    if (displayManager) {
        displayManager->updateReceivedMessage(command);
    }
    
    // 시스템 명령 처리
    if (processSystemCommand(cmd)) {
        return;
    }
    
    // 속도 명령 처리
    if (cmd.startsWith("speed:")) {
        processSpeedCommand(cmd);
        return;
    }
    
    // 이동 명령 처리
    processMovementCommand(cmd);
    
    // 디스플레이 업데이트
    if (displayManager) {
        displayManager->updateMotorStatus();
    }
}

void CommandProcessor::setAutoMode(bool autoMode) {
    isAutoMode = autoMode;
    Serial.print("Auto mode: ");
    Serial.println(isAutoMode ? "ON" : "OFF");
}

bool CommandProcessor::isInAutoMode() const {
    return isAutoMode;
}

bool CommandProcessor::processSystemCommand(const String& command) {
    if (command == "auto") {
        setAutoMode(true);
        return true;
    }
    else if (command == "manual") {
        setAutoMode(false);
        return true;
    }
    else if (command == "encoder") {
        if (displayManager) {
            displayManager->toggleEncoderInfo();
        }
        return true;
    }
    else if (command == "reset") {
        if (encoderManager) {
            encoderManager->resetAllEncoders();
        }
        if (displayManager) {
            displayManager->updateMotorStatus();
        }
        return true;
    }
    
    return false; // 시스템 명령이 아님
}

void CommandProcessor::processMovementCommand(const String& command) {
    if (!motorController) {
        Serial.println("Motor controller not available");
        return;
    }
    
    if (command == "forward") {
        motorController->moveForward();
    }
    else if (command == "backward") {
        motorController->moveBackward();
    }
    else if (command == "left") {
        motorController->moveLeft();
    }
    else if (command == "right") {
        motorController->moveRight();
    }
    else if (command == "rotate_left") {
        motorController->rotateLeft();
    }
    else if (command == "rotate_right") {
        motorController->rotateRight();
    }
    else if (command == "diagonal_fl") {
        motorController->moveDiagonalFL();
    }
    else if (command == "diagonal_fr") {
        motorController->moveDiagonalFR();
    }
    else if (command == "stop") {
        motorController->stop();
    }
    else {
        Serial.print("Unknown movement command: ");
        Serial.println(command);
    }
}

void CommandProcessor::processSpeedCommand(const String& command) {
    if (!motorController) {
        Serial.println("Motor controller not available");
        return;
    }
    
    int speed = extractSpeedValue(command);
    if (speed >= 0) {
        motorController->setSpeed(speed);
        
        // 현재 모터가 작동 중이면 새 속도로 적용
        if (motorController->isMotorRunning()) {
            Direction currentDir = motorController->getCurrentDirection();
            String dirStr = motorController->directionToString(currentDir);
            dirStr.toLowerCase();
            processMovementCommand(dirStr);
        }
    } else {
        Serial.println("Invalid speed value");
    }
}

String CommandProcessor::toLowerCase(const String& str) const {
    String result = str;
    result.toLowerCase();
    return result;
}

int CommandProcessor::extractSpeedValue(const String& command) const {
    // "speed:50" 형태에서 숫자 추출
    int colonIndex = command.indexOf(':');
    if (colonIndex != -1 && colonIndex < command.length() - 1) {
        String speedStr = command.substring(colonIndex + 1);
        speedStr.trim();
        return speedStr.toInt();
    }
    return -1; // 잘못된 형식
}