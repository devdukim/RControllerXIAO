#include "BluetoothManager.h"
#include "CommandProcessor.h"

BluetoothManager::BluetoothManager() 
    : pServer(nullptr), pCharacteristic(nullptr), commandProcessor(nullptr),
      deviceConnected(false), oldDeviceConnected(false) {
    Serial.println("BluetoothManager constructor called");
}

BluetoothManager::~BluetoothManager() {
    // BLE 리소스 정리는 ESP32 라이브러리에서 자동 처리
}

bool BluetoothManager::initialize() {
    Serial.println("Initializing Bluetooth Manager...");
    
    // BLE 장치 초기화
    BLEDevice::init(BLE_DEVICE_NAME);
    Serial.println("BLE device initialized");
    
    pServer = BLEDevice::createServer();
    if (!pServer) {
        Serial.println("Failed to create BLE server!");
        return false;
    }
    Serial.println("BLE server created");
    
    pServer->setCallbacks(new ServerCallbacks(this));
    Serial.println("Server callbacks set");
    
    // BLE 서비스 생성
    BLEService* pService = pServer->createService(SERVICE_UUID);
    if (!pService) {
        Serial.println("Failed to create BLE service!");
        return false;
    }
    Serial.println("BLE service created");
    
    // BLE 특성 생성
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    if (!pCharacteristic) {
        Serial.println("Failed to create BLE characteristic!");
        return false;
    }
    Serial.println("BLE characteristic created");
    
    pCharacteristic->setCallbacks(new CharacteristicCallbacks(this));
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Mecanum Ready");
    Serial.println("Characteristic callbacks and descriptor set");
    
    // 서비스 시작
    pService->start();
    Serial.println("BLE service started");
    
    // 광고 설정 및 시작
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    if (!pAdvertising) {
        Serial.println("Failed to get advertising object!");
        return false;
    }
    
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE advertising started");
    Serial.println("BLE server ready, waiting for connections...");
    return true;
}

void BluetoothManager::setCommandProcessor(CommandProcessor* processor) {
    commandProcessor = processor;
    Serial.println("Command processor set");
}

bool BluetoothManager::isConnected() const {
    return deviceConnected;
}

void BluetoothManager::handleConnectionChange() {
    // 연결 해제 감지
    if (!deviceConnected && oldDeviceConnected) {
        Serial.println("Connection lost, restarting advertising...");
        delay(500); // 연결 해제 이벤트 처리 시간
        pServer->startAdvertising(); // 재광고 시작
        Serial.println("Advertising restarted");
        oldDeviceConnected = deviceConnected;
    }
    
    // 새로운 연결 감지
    if (deviceConnected && !oldDeviceConnected) {
        Serial.println("New connection established");
        oldDeviceConnected = deviceConnected;
    }
}

void BluetoothManager::sendMessage(const String& message) {
    if (!deviceConnected) {
        Serial.println("Cannot send message: Not connected");
        return;
    }
    
    if (!pCharacteristic) {
        Serial.println("Cannot send message: Characteristic not initialized");
        return;
    }
    
    Serial.print("Sending message: ");
    Serial.println(message);
    
    pCharacteristic->setValue(message.c_str());
    pCharacteristic->notify();
}

String BluetoothManager::getLastReceivedMessage() const {
    return receivedMessage;
}

void BluetoothManager::onConnect() {
    deviceConnected = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("BLE device connected");
    
    // 연결 성공 메시지 전송
    sendMessage("Connected to Mecanum Robot");
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
        Serial.println("Processing command...");
        commandProcessor->processCommand(message);
    } else {
        Serial.println("Warning: No command processor set!");
    }
    
    // 응답 전송
    String response = "Received: " + message;
    sendMessage(response);
}

// ServerCallbacks 구현
ServerCallbacks::ServerCallbacks(BluetoothManager* manager) : btManager(manager) {
    Serial.println("ServerCallbacks constructor called");
}

void ServerCallbacks::onConnect(BLEServer* pServer) {
    Serial.println("ServerCallbacks::onConnect called");
    if (btManager) {
        btManager->onConnect();
    } else {
        Serial.println("Warning: btManager is null in onConnect!");
    }
}

void ServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("ServerCallbacks::onDisconnect called");
    if (btManager) {
        btManager->onDisconnect();
    } else {
        Serial.println("Warning: btManager is null in onDisconnect!");
    }
}

// CharacteristicCallbacks 구현
CharacteristicCallbacks::CharacteristicCallbacks(BluetoothManager* manager) : btManager(manager) {
    Serial.println("CharacteristicCallbacks constructor called");
}

void CharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    Serial.println("CharacteristicCallbacks::onWrite called");
    std::string value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
        if (btManager) {
            String message = "";
            for (int i = 0; i < value.length(); i++) {
                message += value[i];
            }
            Serial.print("Received value: ");
            Serial.println(message);
            btManager->onMessageReceived(message);
        } else {
            Serial.println("Warning: btManager is null in onWrite!");
        }
    } else {
        Serial.println("Warning: Received empty value in onWrite!");
    }
}