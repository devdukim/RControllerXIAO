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
    delay(1000); // 시리얼 통신 안정화를 위한 지연
    Serial.println("\n\n=== ESP32C3 Mecanum Wheel Robot (4 Motors) - Modular Version ===");
    Serial.println("Starting initialization sequence...");
    
    // LED 핀 설정
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED pin configured");
    
    // I2C 통신 초기화 (가장 먼저)
    Serial.println("Initializing I2C...");
    Wire.begin();
    Wire.setClock(400000); // 400kHz
    Serial.println("I2C initialized at 400kHz");
    
    // 객체 생성
    Serial.println("\nCreating objects...");
    Serial.println("Creating MotorController...");
    motorController = new MotorController();
    Serial.println("Creating EncoderManager...");
    encoderManager = EncoderManager::getInstance();
    Serial.println("Creating DisplayManager...");
    displayManager = new DisplayManager();
    Serial.println("Creating BluetoothManager...");
    bluetoothManager = new BluetoothManager();
    Serial.println("Creating CommandProcessor...");
    commandProcessor = new CommandProcessor();
    Serial.println("All objects created successfully");
    
    // 각 모듈 초기화
    Serial.println("\nInitializing modules...");
    
    Serial.println("\n1. Initializing Display Manager...");
    if (!displayManager->initialize()) {
        Serial.println("ERROR: Failed to initialize display manager!");
        return;
    }
    Serial.println("Display manager initialized successfully");
    
    Serial.println("\n2. Initializing Motor Controller...");
    if (!motorController->initialize()) {
        Serial.println("ERROR: Failed to initialize motor controller!");
        return;
    }
    Serial.println("Motor controller initialized successfully");
    
    Serial.println("\n3. Initializing Encoder Manager...");
    if (!encoderManager->initialize()) {
        Serial.println("ERROR: Failed to initialize encoder manager!");
        return;
    }
    Serial.println("Encoder manager initialized successfully");
    
    // 의존성 주입 (객체 간 연결)
    Serial.println("\nSetting up dependencies...");
    Serial.println("Connecting Display Manager to Motor and Encoder...");
    displayManager->setMotorController(motorController);
    displayManager->setEncoderManager(encoderManager);
    
    Serial.println("Connecting Command Processor to all managers...");
    commandProcessor->setMotorController(motorController);
    commandProcessor->setEncoderManager(encoderManager);
    commandProcessor->setDisplayManager(displayManager);
    
    Serial.println("Connecting Bluetooth Manager to Command Processor...");
    bluetoothManager->setCommandProcessor(commandProcessor);
    
    Serial.println("\n4. Initializing Bluetooth Manager...");
    if (!bluetoothManager->initialize()) {
        Serial.println("ERROR: Failed to initialize bluetooth manager!");
        return;
    }
    Serial.println("Bluetooth manager initialized successfully");
    
    // 시작 화면 표시
    Serial.println("\nUpdating display...");
    displayManager->updateStartupScreen();
    displayManager->showStartupEffect();
    
    // 초기 상태 업데이트
    displayManager->updateMotorStatus();
    
    Serial.println("\n=== 4-Motor Mecanum Robot setup complete ===");
    Serial.println("Ready for commands...");
    Serial.println("----------------------------------------");
}

void loop() {
    // 블루투스 연결 상태 변경 처리
    bluetoothManager->handleConnectionChange();
    
    // 인코더 정보 주기적 출력
    encoderManager->periodicPrint();
    
    // 메인 루프 지연
    delay(10);
}