#include "BluetoothManager.h"
#include "CommandProcessor.h"

BluetoothManager::BluetoothManager() 
    : pServer(nullptr), pCharacteristic(nullptr), commandProcessor(nullptr),
      deviceConnected(false), oldDeviceConnected(false) {
}

BluetoothManager::~BluetoothManager() {
    // BLE 리소스 정리는 ESP32 라이브러리에서 자동 처리
}

bool BluetoothManager::initialize() {
    // BLE 장치 초기화
    BLEDevice::init(BLE_DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));
    
    // BLE 서비스 생성
    BLEService* pService = pServer->createService(SERVICE_UUID);
    
    // BLE 특성 생성
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    pCharacteristic->setCallbacks(new CharacteristicCallbacks(this));
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Mecanum Ready");
    
    // 서비스 시작
    pService->start();
    
    // 광고 설정 및 시작
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE server started, waiting for connections...");
    return true;
}

void BluetoothManager::setCommandProcessor(CommandProcessor* processor) {
    commandProcessor = processor;
}

bool BluetoothManager::isConnected() const {
    return deviceConnected;
}

void BluetoothManager::handleConnectionChange() {
    // 연결 해제 감지
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // 연결 해제 이벤트 처리 시간
        pServer->startAdvertising(); // 재광고 시작
        Serial.println("Restarting advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    // 새로운 연결 감지
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}

void BluetoothManager::sendMessage(const String& message) {
    if (deviceConnected && pCharacteristic) {
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
    }
}

String BluetoothManager::getLastReceivedMessage() const {
    return receivedMessage;
}

void BluetoothManager::onConnect() {
    deviceConnected = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("BLE device connected");
}

void BluetoothManager::onDisconnect() {
    deviceConnected = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("BLE device disconnected");
}

void BluetoothManager::onMessageReceived(const String& message) {
    receivedMessage = message;
    Serial.print("BLE Received: ");
    Serial.println(message);
    
    // 명령 처리기에 메시지 전달
    if (commandProcessor) {
        commandProcessor->processCommand(message);
    }
    
    // 응답 전송
    String response = "Received: " + message;
    sendMessage(response);
}

// ServerCallbacks 구현
ServerCallbacks::ServerCallbacks(BluetoothManager* manager) : btManager(manager) {
}

void ServerCallbacks::onConnect(BLEServer* pServer) {
    if (btManager) {
        btManager->onConnect();
    }
}

void ServerCallbacks::onDisconnect(BLEServer* pServer) {
    if (btManager) {
        btManager->onDisconnect();
    }
}

// CharacteristicCallbacks 구현
CharacteristicCallbacks::CharacteristicCallbacks(BluetoothManager* manager) : btManager(manager) {
}

void CharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() > 0 && btManager) {
        String message = "";
        for (int i = 0; i < value.length(); i++) {
            message += value[i];
        }
        btManager->onMessageReceived(message);
    }
}