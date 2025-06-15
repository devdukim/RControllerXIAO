#include "DisplayManager.h"
#include "MotorController.h"
#include "EncoderManager.h"
#include <Wire.h>

DisplayManager::DisplayManager() 
    : display(nullptr), motorController(nullptr), encoderManager(nullptr), showEncoderInfo(false), isInitialized(false) {
}

DisplayManager::~DisplayManager() {
    if (display) {
        delete display;
    }
}

bool DisplayManager::initialize() {
    if (isInitialized) {
        Serial.println("Display Manager already initialized");
        return true;
    }

    Serial.println("Initializing Display Manager...");
    
    // Wire 초기화 확인
    Wire.begin();
    Wire.setClock(400000);  // 400kHz로 설정
    delay(100);  // I2C 버스 안정화를 위한 지연
    
    // OLED 디스플레이 초기화
    if (display) {
        delete display;
    }
    
    display = new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
    if (!display) {
        Serial.println("Failed to create display object!");
        return false;
    }
    
    // 디스플레이 초기화 시도 (최대 3번)
    bool initSuccess = false;
    for (int i = 0; i < 3 && !initSuccess; i++) {
        Serial.print("Display initialization attempt ");
        Serial.println(i + 1);
        
        if (display->begin()) {
            initSuccess = true;
            break;
        }
        
        Serial.println("Display init failed, resetting I2C bus...");
        resetI2CBus();
        delay(100);
    }
    
    if (!initSuccess) {
        Serial.println("Failed to initialize display after 3 attempts!");
        delete display;
        display = nullptr;
        return false;
    }
    
    display->setFlipMode(1);  // 화면 회전
    display->setFont(u8x8_font_chroma48medium8_r);
    
    // 초기 상태 설정
    isInitialized = true;
    isSending = false;
    currentStatus = "";
    lastResponse = "";
    lastUpdateTime = millis();
    
    // 시작 화면 표시
    clearScreen();
    display->setCursor(0, 0);
    display->print("ESP32C3 4Motor");
    display->setCursor(0, 1);
    display->print("Mecanum Robot");
    display->setCursor(0, 2);
    display->print("BLE: Starting");
    display->setCursor(0, 3);
    display->print("Status: Init");
    
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
    if (!isInitialized || !display) {
        Serial.println("Display not ready for connection update");
        return;
    }
    
    Serial.print("Updating BLE connection status: ");
    Serial.println(connected ? "Connected" : "Disconnected");
    
    // BLE 연결 상태 표시 (1번째 줄)
    display->clearLine(0);
    display->setCursor(0, 0);
    if (connected) {
        display->print("BLE: Connected");
        // 연결 시 통신 상태 초기화
        display->clearLine(2);
        display->setCursor(0, 2);
        display->print("Waiting for cmd");
        
        display->clearLine(3);
        display->setCursor(0, 3);
        display->print("Status: Active");
        
        // LED 효과로 연결 알림
        showMessageReceivedEffect();
    } else {
        display->print("BLE: Disconnected");
        // 연결 해제 시 통신 상태 초기화
        display->clearLine(2);
        display->setCursor(0, 2);
        display->print("No Connection");
        
        display->clearLine(3);
        display->setCursor(0, 3);
        display->print("Status: Offline");
    }
    
    // 상태 업데이트 시간 기록
    lastUpdateTime = millis();
}

void DisplayManager::updateReceivedMessage(const String& message) {
    if (!isInitialized || !display) {
        Serial.println("Display not ready for message update");
        return;
    }
    
    Serial.print("Received message: ");
    Serial.println(message);
    
    // 수신된 메시지 표시 (2번째 줄)
    display->clearLine(2);
    display->setCursor(0, 2);
    display->print("RX: ");
    
    // 메시지가 너무 길 경우 처리
    if (message.length() <= 16) {
        display->print(message);
    } else {
        display->print(message.substring(0, 13));
        display->print("...");
    }
    
    // 수신 상태 표시 (3번째 줄)
    display->clearLine(3);
    display->setCursor(0, 3);
    display->print("Status: Received");
    
    // LED 효과 표시
    showMessageReceivedEffect();
    
    // 마지막 수신 시간 업데이트
    lastUpdateTime = millis();
    lastResponse = message;  // 마지막 응답 저장
}

void DisplayManager::updateCommunicationStatus(bool isSending, const String& status) {
    if (!isInitialized || !display) return;
    
    this->isSending = isSending;
    this->currentStatus = status;
    this->lastUpdateTime = millis();
    
    // 통신 상태 표시 (2번째 줄)
    display->clearLine(2);
    display->setCursor(0, 2);
    if (isSending) {
        display->print("TX: ");
        if (status.length() > 0) {
            if (status.length() <= 16) {
                display->print(status);
            } else {
                display->print(status.substring(0, 13));
                display->print("...");
            }
        }
    } else {
        display->print("RX: ");
        if (lastResponse.length() > 0) {
            if (lastResponse.length() <= 16) {
                display->print(lastResponse);
            } else {
                display->print(lastResponse.substring(0, 13));
                display->print("...");
            }
        } else {
            display->print("Waiting");
        }
    }
    
    // 상태 업데이트 (3번째 줄)
    display->clearLine(3);
    display->setCursor(0, 3);
    if (isSending) {
        display->print("Status: Sending");
    } else {
        display->print("Status: Active");
    }
}

void DisplayManager::updateResponseStatus(const String& response) {
    if (!isInitialized || !display) return;
    
    this->lastResponse = response;
    this->isSending = false;
    this->lastUpdateTime = millis();
    
    // 응답 표시 (2번째 줄)
    display->clearLine(2);
    display->setCursor(0, 2);
    display->print("RX: ");
    if (response.length() > 0) {
        if (response.length() <= 16) {
            display->print(response);
        } else {
            display->print(response.substring(0, 13));
            display->print("...");
        }
    } else {
        display->print("Waiting");
    }
    
    // 상태 업데이트 (3번째 줄)
    display->clearLine(3);
    display->setCursor(0, 3);
    display->print("Status: Received");
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
    if (!isInitialized || !display) {
        Serial.println("Display not ready for startup screen");
        return;
    }

    clearScreen();
    Serial.println("After clearScreen, checking display object...");
    if (!display) {
        Serial.println("ERROR: Display object became null after clearScreen!");
        return;
    }
    
    display->setCursor(0, 0);
    display->print("ESP32C3 4Motor");
    display->setCursor(0, 1);
    display->print("Mecanum Robot");
    display->setCursor(0, 2);
    display->print("BLE: Starting");
    
    // 초기 통신 상태 표시
    display->clearLine(3);
    display->setCursor(0, 3);
    display->print("Status: Init");
    
    // 모터 상태 초기화
    display->clearLine(5);
    display->clearLine(6);
    display->clearLine(7);
    
    // 상태 초기화
    isSending = false;
    currentStatus = "";
    lastResponse = "";
    lastUpdateTime = millis();
}

void DisplayManager::toggleEncoderInfo() {
    showEncoderInfo = !showEncoderInfo;
    updateMotorStatus();
}

bool DisplayManager::isShowingEncoderInfo() const {
    return showEncoderInfo;
}

void DisplayManager::resetI2CBus() {
    Serial.println("Resetting I2C bus...");
    Wire.end();
    delay(100);  // I2C 버스 안정화를 위한 지연
    Wire.begin();
    Wire.setClock(400000);  // 400kHz로 설정
    delay(100);  // I2C 버스 안정화를 위한 지연
    Serial.println("I2C bus reset completed");
}

void DisplayManager::clearScreen() {
    if (!display) {
        Serial.println("ERROR: Display object is null in clearScreen");
        return;
    }
    
    Serial.println("Attempting to clear display...");
    
    // I2C 버스 상태 확인
    Wire.beginTransmission(0x3C);  // OLED 디스플레이의 I2C 주소
    byte error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.print("I2C error before clear: ");
        Serial.println(error);
        Serial.println("Attempting to reset I2C bus...");
        resetI2CBus();
        
        // 디스플레이 재초기화 시도
        if (!display->begin()) {
            Serial.println("Failed to reinitialize display after I2C reset!");
            return;
        }
        display->setFlipMode(1);
        display->setFont(u8x8_font_chroma48medium8_r);
    }
    
    // 디스플레이 클리어 시도
    display->clear();
    Serial.println("Display clear operation completed");
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
    display->print(encoderManager->getEncoderCount(MOTOR_FRONT_LEFT) / 100);
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