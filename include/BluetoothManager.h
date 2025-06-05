#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"

// 전방 선언
class CommandProcessor;

class BluetoothManager {
private:
    BLEServer* pServer;
    BLECharacteristic* pCharacteristic;
    CommandProcessor* commandProcessor;
    
    bool deviceConnected;
    bool oldDeviceConnected;
    String receivedMessage;
    
public:
    BluetoothManager();
    ~BluetoothManager();
    
    // 초기화
    bool initialize();
    void setCommandProcessor(CommandProcessor* processor);
    
    // 연결 상태 관리
    bool isConnected() const;
    void handleConnectionChange();
    
    // 메시지 송수신
    void sendMessage(const String& message);
    String getLastReceivedMessage() const;
    
    // BLE 서버 콜백 클래스 (친구 클래스)
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
    
    // 내부 콜백 처리
    void onConnect();
    void onDisconnect();
    void onMessageReceived(const String& message);
};

// BLE 서버 콜백 클래스
class ServerCallbacks : public BLEServerCallbacks {
private:
    BluetoothManager* btManager;
    
public:
    ServerCallbacks(BluetoothManager* manager);
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
};

// BLE 특성 콜백 클래스
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
private:
    BluetoothManager* btManager;
    
public:
    CharacteristicCallbacks(BluetoothManager* manager);
    void onWrite(BLECharacteristic* pCharacteristic) override;
};

#endif // BLUETOOTH_MANAGER_H