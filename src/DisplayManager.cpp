#include "DisplayManager.h"
#include "MotorController.h"
#include "EncoderManager.h"

DisplayManager::DisplayManager() 
    : display(nullptr), motorController(nullptr), encoderManager(nullptr), showEncoderInfo(false) {
}

DisplayManager::~DisplayManager() {
    if (display) {
        delete display;
    }
}

bool DisplayManager::initialize() {
    display = new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
    if (!display) {
        return false;
    }
    
    display->begin();
    display->setFlipMode(1);
    display->setFont(u8x8_font_chroma48medium8_r);
    
    clearScreen();
    Serial.println("Display manager initialized successfully");
    return true;
}

void DisplayManager::setMotorController(MotorController* controller) {
    motorController = controller;
}

void DisplayManager::setEncoderManager(EncoderManager* manager) {
    encoderManager = manager;
}

void DisplayManager::updateConnectionStatus(bool connected) {
    display->clearLine(0);
    display->setCursor(0, 0);
    if (connected) {
        display->print("Connected!");
    } else {
        display->print("Disconnected");
    }
}

void DisplayManager::updateReceivedMessage(const String& message) {
    display->clearLine(2);
    display->setCursor(0, 2);
    display->print("Received:");
    
    display->clearLine(3);
    display->clearLine(4);
    
    if (message.length() <= 16) {
        display->setCursor(0, 3);
        display->print(message);
    } else if (message.length() <= 32) {
        display->setCursor(0, 3);
        display->print(message.substring(0, 16));
        display->setCursor(0, 4);
        display->print(message.substring(16));
    } else {
        display->setCursor(0, 3);
        display->print(message.substring(0, 16));
        display->setCursor(0, 4);
        display->print(message.substring(16, 32));
        display->print("...");
    }
    
    showMessageReceivedEffect();
}

void DisplayManager::updateMotorStatus() {
    if (!motorController) return;
    
    display->clearLine(5);
    display->setCursor(0, 5);
    display->print("Dir: ");
    display->print(motorController->directionToString(motorController->getCurrentDirection()));
    
    display->clearLine(6);
    display->setCursor(0, 6);
    display->print("Speed: ");
    display->print(motorController->getSpeed());
    display->print("%");
    
    // 인코더 정보 또는 모터 상태 표시
    if (showEncoderInfo) {
        displayEncoderInfo();
    } else {
        displayMotorStatus();
    }
}

void DisplayManager::updateStartupScreen() {
    clearScreen();
    display->setCursor(0, 0);
    display->print("ESP32C3 4Motor");
    display->setCursor(0, 1);
    display->print("Mecanum Robot");
    display->setCursor(0, 2);
    display->print("BLE Ready!");
}

void DisplayManager::toggleEncoderInfo() {
    showEncoderInfo = !showEncoderInfo;
    updateMotorStatus();
}

bool DisplayManager::isShowingEncoderInfo() const {
    return showEncoderInfo;
}

void DisplayManager::clearScreen() {
    display->clear();
}

void DisplayManager::clearLine(int line) {
    display->clearLine(line);
}

void DisplayManager::showMessageReceivedEffect() {
    // LED 깜빡임 효과
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    
    // 연결 상태에 따라 LED 상태 복원
    // 이 부분은 BluetoothManager의 연결 상태를 확인해야 하지만
    // 의존성을 줄이기 위해 단순화
}

void DisplayManager::showStartupEffect() {
    // 시작 신호로 LED 3번 깜빡임
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}

void DisplayManager::displayEncoderInfo() {
    if (!encoderManager) return;
    
    display->clearLine(7);
    display->setCursor(0, 7);
    display->print("FL:");
    display->print(encoderManager->getEncoderCount(MOTOR_FRONT_LEFT) / 100); // 표시용으로 100으로 나눔
    display->print(" FR:");
    display->print(encoderManager->getEncoderCount(MOTOR_FRONT_RIGHT) / 100);
}

void DisplayManager::displayMotorStatus() {
    if (!motorController) return;
    
    display->clearLine(7);
    display->setCursor(0, 7);
    display->print("Status: ");
    display->print(motorController->isMotorRunning() ? "RUNNING" : "STOPPED");
}