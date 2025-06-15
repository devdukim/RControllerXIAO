#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8x8lib.h>
#include "config.h"
#include "version.h"  // 버전 정보 포함
#include <Arduino.h>  // String 타입을 위해 추가

// 전방 선언
class MotorController;
class EncoderManager;

class DisplayManager {
private:
    U8X8_SSD1306_128X64_NONAME_HW_I2C* display;
    MotorController* motorController;
    EncoderManager* encoderManager;
    
    // 디스플레이 상태 관리
    bool showEncoderInfo;
    bool isInitialized;
    
    // 통신 상태 추적
    bool isSending;
    String currentStatus;
    String lastResponse;
    unsigned long lastUpdateTime;
    
    // 디스플레이 설정
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;
    static const int OLED_RESET = -1;  // 리셋 핀 사용 안함
    static const int SCREEN_ADDRESS = 0x3C;  // I2C 주소
    
    // I2C 버스 리셋
    void resetI2CBus();
    
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
    
    // 통신 상태 표시 메서드
    void updateCommunicationStatus(bool isSending, const String& status = "");
    void updateResponseStatus(const String& response);
    
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
    void displayText(int x, int y, const String& text);
};

#endif // DISPLAY_MANAGER_H