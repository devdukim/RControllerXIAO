#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "config.h"

class EncoderManager {
private:
    static EncoderManager* instance;  // 싱글톤 패턴
    
    // 인코더 카운터 (volatile for ISR)
    volatile long encoderFL_Count;
    volatile long encoderFR_Count;
    volatile long encoderRL_Count;
    volatile long encoderRR_Count;
    
    unsigned long lastPrintTime;
    
public:
    EncoderManager();
    ~EncoderManager();
    
    // 싱글톤 인스턴스 접근
    static EncoderManager* getInstance();
    
    // 초기화
    bool initialize();
    
    // 인코더 값 접근
    long getEncoderCount(MotorIndex motorIndex) const;
    void resetEncoder(MotorIndex motorIndex);
    void resetAllEncoders();
    
    // 인코더 정보 출력
    void printEncoderInfo();
    void periodicPrint();  // 주기적 출력
    
    // ISR 함수들 (static으로 선언)
    static void IRAM_ATTR encoderFL_ISR();
    static void IRAM_ATTR encoderFR_ISR();
    static void IRAM_ATTR encoderRL_ISR();
    static void IRAM_ATTR encoderRR_ISR();
    
    // 인코더 값 업데이트 (ISR에서 호출)
    void updateEncoderFL(bool direction);
    void updateEncoderFR(bool direction);
    void updateEncoderRL(bool direction);
    void updateEncoderRR(bool direction);
};

#endif // ENCODER_MANAGER_H