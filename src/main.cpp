#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "MotorController.h"
#include "EncoderManager.h"
#include "BluetoothManager.h"
#include "DisplayManager.h"
#include "CommandProcessor.h"

// 전역 객체 선언
MotorController* motorController;
EncoderManager* encoderManager;
BluetoothManager* bluetoothManager;
DisplayManager* displayManager;
CommandProcessor* commandProcessor;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("ESP32C3 Mecanum Wheel Robot (4 Motors) - Modular Version");
    
    // LED 핀 설정
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // I2C 통신 초기화
    Wire.begin();
    
    // 객체 생성 및 초기화
    motorController = new MotorController();
    encoderManager = EncoderManager::getInstance();
    bluetoothManager = new BluetoothManager();
    displayManager = new DisplayManager();
    commandProcessor = new CommandProcessor();
    
    // 각 모듈 초기화
    Serial.println("Initializing modules...");
    
    if (!motorController->initialize()) {
        Serial.println("Failed to initialize motor controller!");
        return;
    }
    Serial.println("Motor controller initialized");
    
    if (!encoderManager->initialize()) {
        Serial.println("Failed to initialize encoder manager!");
        return;
    }
    Serial.println("Encoder manager initialized");
    
    if (!displayManager->initialize()) {
        Serial.println("Failed to initialize display manager!");
        return;
    }
    Serial.println("Display manager initialized");
    
    // 의존성 주입 (객체 간 연결)
    displayManager->setMotorController(motorController);
    displayManager->setEncoderManager(encoderManager);
    
    commandProcessor->setMotorController(motorController);
    commandProcessor->setEncoderManager(encoderManager);
    commandProcessor->setDisplayManager(displayManager);
    
    bluetoothManager->setCommandProcessor(commandProcessor);
    
    if (!bluetoothManager->initialize()) {
        Serial.println("Failed to initialize bluetooth manager!");
        return;
    }
    Serial.println("Bluetooth manager initialized");
    
    // 시작 화면 표시
    displayManager->updateStartupScreen();
    displayManager->showStartupEffect();
    
    // 초기 상태 업데이트
    displayManager->updateMotorStatus();
    
    Serial.println("4-Motor Mecanum Robot setup complete");
    Serial.println("Ready for commands...");
}

void loop() {
    // 블루투스 연결 상태 변경 처리
    bluetoothManager->handleConnectionChange();
    
    // 인코더 정보 주기적 출력
    encoderManager->periodicPrint();
    
    // 메인 루프 지연
    delay(10);
}