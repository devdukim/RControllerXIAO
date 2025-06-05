#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8x8lib.h>
#include "config.h"

// 전방 선언
class MotorController;
class EncoderManager;

class DisplayManager {
private:
    U8X8_SSD1306_128X64_NONAME_HW_I2C* display;
    MotorController* motorController;
    EncoderManager* encoderManager;
    
    bool showEncoderInfo;
    
public:
    DisplayManager();
    ~DisplayManager();
    
    // 초기화
    bool initialize();
    void setMotorController(MotorController* controller);
    void setEncoderManager(EncoderManager* manager);
    
    // 화면 업데이트
    void updateConnectionStatus(bool connected);
    void updateReceivedMessage(const String& message);
    void updateMotorStatus();
    void updateStartupScreen();
    
    // 정보 표시 모드 토글
    void toggleEncoderInfo();
    bool isShowingEncoderInfo() const;
    
    // 화면 지우기 및 기본 설정
    void clearScreen();
    void clearLine(int line);
    
    // LED 효과
    void showMessageReceivedEffect();
    void showStartupEffect();
    
private:
    // 내부 헬퍼 함수
    void displayEncoderInfo();
    void displayMotorStatus();
};

#endif // DISPLAY_MANAGER_H