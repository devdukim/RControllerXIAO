#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "config.h"

// 전방 선언
class MotorController;
class EncoderManager;
class DisplayManager;

class CommandProcessor {
private:
    MotorController* motorController;
    EncoderManager* encoderManager;
    DisplayManager* displayManager;
    
    bool isAutoMode;
    
public:
    CommandProcessor();
    ~CommandProcessor();
    
    // 초기화 및 의존성 주입
    void setMotorController(MotorController* controller);
    void setEncoderManager(EncoderManager* manager);
    void setDisplayManager(DisplayManager* display);
    
    // 명령 처리
    void processCommand(const String& command);
    
    // 모드 관리
    void setAutoMode(bool autoMode);
    bool isInAutoMode() const;
    
private:
    // 명령 처리 헬퍼 함수
    void processMovementCommand(const String& command);
    void processSpeedCommand(const String& command);
    bool processSystemCommand(const String& command);
    
    // 유틸리티 함수
    String toLowerCase(const String& str) const;
    int extractSpeedValue(const String& command) const;
};

#endif // COMMAND_PROCESSOR_H