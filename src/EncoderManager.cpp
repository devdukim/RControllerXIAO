#include "EncoderManager.h"

// 싱글톤 인스턴스 초기화
EncoderManager* EncoderManager::instance = nullptr;

EncoderManager::EncoderManager() 
    : encoderFL_Count(0), encoderFR_Count(0), encoderRL_Count(0), encoderRR_Count(0), lastPrintTime(0) {
}

EncoderManager::~EncoderManager() {
}

EncoderManager* EncoderManager::getInstance() {
    if (instance == nullptr) {
        instance = new EncoderManager();
    }
    return instance;
}

bool EncoderManager::initialize() {
    // 인코더 핀 설정
    pinMode(ENCODER_FL_A, INPUT_PULLUP);
    pinMode(ENCODER_FL_B, INPUT_PULLUP);
    pinMode(ENCODER_FR_A, INPUT_PULLUP);
    pinMode(ENCODER_FR_B, INPUT_PULLUP);
    pinMode(ENCODER_RL_A, INPUT_PULLUP);
    pinMode(ENCODER_RL_B, INPUT_PULLUP);
    pinMode(ENCODER_RR_A, INPUT_PULLUP);
    pinMode(ENCODER_RR_B, INPUT_PULLUP);
    
    // 인코더 인터럽트 설정
    attachInterrupt(digitalPinToInterrupt(ENCODER_FL_A), encoderFL_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_FR_A), encoderFR_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_RL_A), encoderRL_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_RR_A), encoderRR_ISR, RISING);
    
    Serial.println("Encoder manager initialized successfully");
    return true;
}

long EncoderManager::getEncoderCount(MotorIndex motorIndex) const {
    switch(motorIndex) {
        case MOTOR_FRONT_LEFT:
            return encoderFL_Count;
        case MOTOR_FRONT_RIGHT:
            return encoderFR_Count;
        case MOTOR_REAR_LEFT:
            return encoderRL_Count;
        case MOTOR_REAR_RIGHT:
            return encoderRR_Count;
        default:
            return 0;
    }
}

void EncoderManager::resetEncoder(MotorIndex motorIndex) {
    switch(motorIndex) {
        case MOTOR_FRONT_LEFT:
            encoderFL_Count = 0;
            break;
        case MOTOR_FRONT_RIGHT:
            encoderFR_Count = 0;
            break;
        case MOTOR_REAR_LEFT:
            encoderRL_Count = 0;
            break;
        case MOTOR_REAR_RIGHT:
            encoderRR_Count = 0;
            break;
    }
}

void EncoderManager::resetAllEncoders() {
    encoderFL_Count = 0;
    encoderFR_Count = 0;
    encoderRL_Count = 0;
    encoderRR_Count = 0;
    Serial.println("All encoders reset");
}

void EncoderManager::printEncoderInfo() {
    Serial.print("Encoders - FL:");
    Serial.print(encoderFL_Count);
    Serial.print(" FR:");
    Serial.print(encoderFR_Count);
    Serial.print(" RL:");
    Serial.print(encoderRL_Count);
    Serial.print(" RR:");
    Serial.println(encoderRR_Count);
}

void EncoderManager::periodicPrint() {
    if (millis() - lastPrintTime >= ENCODER_PRINT_INTERVAL) {
        printEncoderInfo();
        lastPrintTime = millis();
    }
}

// 정적 ISR 함수들
void IRAM_ATTR EncoderManager::encoderFL_ISR() {
    if (instance) {
        bool direction = digitalRead(ENCODER_FL_B) == HIGH;
        instance->updateEncoderFL(direction);
    }
}

void IRAM_ATTR EncoderManager::encoderFR_ISR() {
    if (instance) {
        bool direction = digitalRead(ENCODER_FR_B) == HIGH;
        instance->updateEncoderFR(direction);
    }
}

void IRAM_ATTR EncoderManager::encoderRL_ISR() {
    if (instance) {
        bool direction = digitalRead(ENCODER_RL_B) == HIGH;
        instance->updateEncoderRL(direction);
    }
}

void IRAM_ATTR EncoderManager::encoderRR_ISR() {
    if (instance) {
        bool direction = digitalRead(ENCODER_RR_B) == HIGH;
        instance->updateEncoderRR(direction);
    }
}

// 인코더 값 업데이트 함수들 (ISR에서 호출)
void EncoderManager::updateEncoderFL(bool direction) {
    if (direction) {
        encoderFL_Count++;
    } else {
        encoderFL_Count--;
    }
}

void EncoderManager::updateEncoderFR(bool direction) {
    if (direction) {
        encoderFR_Count++;
    } else {
        encoderFR_Count--;
    }
}

void EncoderManager::updateEncoderRL(bool direction) {
    if (direction) {
        encoderRL_Count++;
    } else {
        encoderRL_Count--;
    }
}

void EncoderManager::updateEncoderRR(bool direction) {
    if (direction) {
        encoderRR_Count++;
    } else {
        encoderRR_Count--;
    }
}