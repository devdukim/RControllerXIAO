#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <Adafruit_PWMServoDriver.h>

// OLED 디스플레이 객체 생성 (I2C 통신 사용)
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// PCA9685 초기화
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// 4개 모터 PCA9685 채널 정의
// 전방 좌측 모터 (Front Left)
#define MOTOR_FL_SPEED 0   // ENA - 속도 제어
#define MOTOR_FL_DIR_A 1   // IN1 - 방향 제어 A
#define MOTOR_FL_DIR_B 2   // IN2 - 방향 제어 B

// 전방 우측 모터 (Front Right)
#define MOTOR_FR_SPEED 3   // ENB - 속도 제어
#define MOTOR_FR_DIR_A 4   // IN3 - 방향 제어 A
#define MOTOR_FR_DIR_B 5   // IN4 - 방향 제어 B

// 후방 좌측 모터 (Rear Left)
#define MOTOR_RL_SPEED 6   // ENA - 속도 제어
#define MOTOR_RL_DIR_A 7   // IN1 - 방향 제어 A
#define MOTOR_RL_DIR_B 8   // IN2 - 방향 제어 B

// 후방 우측 모터 (Rear Right)
#define MOTOR_RR_SPEED 9   // ENB - 속도 제어
#define MOTOR_RR_DIR_A 10  // IN3 - 방향 제어 A
#define MOTOR_RR_DIR_B 11  // IN4 - 방향 제어 B

// 인코더 핀 정의 (4개 모터) - I2C 핀(GPIO4,GPIO5) 회피
#define ENCODER_FL_A 0  // GPIO0 - 전방 좌측
#define ENCODER_FL_B 1  // GPIO1
#define ENCODER_FR_A 2  // GPIO2 - 전방 우측
#define ENCODER_FR_B 3  // GPIO3
#define ENCODER_RL_A 6  // GPIO6 - 후방 좌측
#define ENCODER_RL_B 7  // GPIO7
#define ENCODER_RR_A 8  // GPIO8 - 후방 우측
#define ENCODER_RR_B 9  // GPIO9

// PWM 값 설정 (0-4095)
#define PWM_MAX 4095
#define PWM_HALF 2048

// 내장 LED 핀
#define LED_PIN 10

// BLE UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// BLE 서버 및 특성 변수
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String receivedMessage = "";
bool isAutoMode = false;

// 모터 상태 관리 변수
int currentSpeed = PWM_HALF;  // 기본 속도
String currentDirection = "STOP";
bool motorRunning = false;

// 인코더 관련 변수
volatile long encoderFL_Count = 0;  // 전방 좌측
volatile long encoderFR_Count = 0;  // 전방 우측
volatile long encoderRL_Count = 0;  // 후방 좌측
volatile long encoderRR_Count = 0;  // 후방 우측

bool showEncoderInfo = false;

// 인코더 인터럽트 처리 함수들
void IRAM_ATTR encoderFL_ISR() {
  if (digitalRead(ENCODER_FL_B) == HIGH) {
    encoderFL_Count++;
  } else {
    encoderFL_Count--;
  }
}

void IRAM_ATTR encoderFR_ISR() {
  if (digitalRead(ENCODER_FR_B) == HIGH) {
    encoderFR_Count++;
  } else {
    encoderFR_Count--;
  }
}

void IRAM_ATTR encoderRL_ISR() {
  if (digitalRead(ENCODER_RL_B) == HIGH) {
    encoderRL_Count++;
  } else {
    encoderRL_Count--;
  }
}

void IRAM_ATTR encoderRR_ISR() {
  if (digitalRead(ENCODER_RR_B) == HIGH) {
    encoderRR_Count++;
  } else {
    encoderRR_Count--;
  }
}

// 개별 모터 제어 함수
void setMotor(int motorIndex, int speed) {
  int speedChannel, dirAChannel, dirBChannel;
  String motorName;
  
  switch(motorIndex) {
    case 1: // Front Left
      speedChannel = MOTOR_FL_SPEED;
      dirAChannel = MOTOR_FL_DIR_A;
      dirBChannel = MOTOR_FL_DIR_B;
      motorName = "FL";
      break;
    case 2: // Front Right
      speedChannel = MOTOR_FR_SPEED;
      dirAChannel = MOTOR_FR_DIR_A;
      dirBChannel = MOTOR_FR_DIR_B;
      motorName = "FR";
      break;
    case 3: // Rear Left
      speedChannel = MOTOR_RL_SPEED;
      dirAChannel = MOTOR_RL_DIR_A;
      dirBChannel = MOTOR_RL_DIR_B;
      motorName = "RL";
      break;
    case 4: // Rear Right
      speedChannel = MOTOR_RR_SPEED;
      dirAChannel = MOTOR_RR_DIR_A;
      dirBChannel = MOTOR_RR_DIR_B;
      motorName = "RR";
      break;
    default:
      Serial.println("Invalid motor index");
      return;
  }
  
  int pwmSpeed = abs(speed);
  if (pwmSpeed > PWM_MAX) pwmSpeed = PWM_MAX;
  
  if (speed > 0) {
    // 정방향
    pwm.setPWM(dirAChannel, 0, PWM_MAX);
    pwm.setPWM(dirBChannel, 0, 0);
  } else if (speed < 0) {
    // 역방향
    pwm.setPWM(dirAChannel, 0, 0);
    pwm.setPWM(dirBChannel, 0, PWM_MAX);
  } else {
    // 정지
    pwm.setPWM(dirAChannel, 0, 0);
    pwm.setPWM(dirBChannel, 0, 0);
  }
  
  // 속도 설정
  pwm.setPWM(speedChannel, 0, pwmSpeed);
  
  Serial.print(motorName);
  Serial.print(" Motor: ");
  Serial.println(speed);
}

// 메카넘 휠 4모터 동시 제어 함수
void setMecanumMotors(int frontLeft, int frontRight, int rearLeft, int rearRight) {
  // 전방 좌측 모터
  if (frontLeft > 0) {
    pwm.setPWM(MOTOR_FL_DIR_A, 0, PWM_MAX);
    pwm.setPWM(MOTOR_FL_DIR_B, 0, 0);
  } else if (frontLeft < 0) {
    pwm.setPWM(MOTOR_FL_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_FL_DIR_B, 0, PWM_MAX);
  } else {
    pwm.setPWM(MOTOR_FL_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_FL_DIR_B, 0, 0);
  }
  
  // 전방 우측 모터
  if (frontRight > 0) {
    pwm.setPWM(MOTOR_FR_DIR_A, 0, PWM_MAX);
    pwm.setPWM(MOTOR_FR_DIR_B, 0, 0);
  } else if (frontRight < 0) {
    pwm.setPWM(MOTOR_FR_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_FR_DIR_B, 0, PWM_MAX);
  } else {
    pwm.setPWM(MOTOR_FR_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_FR_DIR_B, 0, 0);
  }
  
  // 후방 좌측 모터
  if (rearLeft > 0) {
    pwm.setPWM(MOTOR_RL_DIR_A, 0, PWM_MAX);
    pwm.setPWM(MOTOR_RL_DIR_B, 0, 0);
  } else if (rearLeft < 0) {
    pwm.setPWM(MOTOR_RL_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_RL_DIR_B, 0, PWM_MAX);
  } else {
    pwm.setPWM(MOTOR_RL_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_RL_DIR_B, 0, 0);
  }
  
  // 후방 우측 모터
  if (rearRight > 0) {
    pwm.setPWM(MOTOR_RR_DIR_A, 0, PWM_MAX);
    pwm.setPWM(MOTOR_RR_DIR_B, 0, 0);
  } else if (rearRight < 0) {
    pwm.setPWM(MOTOR_RR_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_RR_DIR_B, 0, PWM_MAX);
  } else {
    pwm.setPWM(MOTOR_RR_DIR_A, 0, 0);
    pwm.setPWM(MOTOR_RR_DIR_B, 0, 0);
  }
  
  // 속도 설정 (동시에)
  int pwmFL = abs(frontLeft);
  int pwmFR = abs(frontRight);
  int pwmRL = abs(rearLeft);
  int pwmRR = abs(rearRight);
  
  if (pwmFL > PWM_MAX) pwmFL = PWM_MAX;
  if (pwmFR > PWM_MAX) pwmFR = PWM_MAX;
  if (pwmRL > PWM_MAX) pwmRL = PWM_MAX;
  if (pwmRR > PWM_MAX) pwmRR = PWM_MAX;
  
  pwm.setPWM(MOTOR_FL_SPEED, 0, pwmFL);
  pwm.setPWM(MOTOR_FR_SPEED, 0, pwmFR);
  pwm.setPWM(MOTOR_RL_SPEED, 0, pwmRL);
  pwm.setPWM(MOTOR_RR_SPEED, 0, pwmRR);
  
  motorRunning = (frontLeft != 0 || frontRight != 0 || rearLeft != 0 || rearRight != 0);
  
  Serial.print("Mecanum Motors - FL:");
  Serial.print(frontLeft);
  Serial.print(" FR:");
  Serial.print(frontRight);
  Serial.print(" RL:");
  Serial.print(rearLeft);
  Serial.print(" RR:");
  Serial.println(rearRight);
}

// 모터 상태를 LCD에 표시하는 함수
void updateMotorStatusOnLCD() {
  u8x8.clearLine(5);
  u8x8.setCursor(0, 5);
  u8x8.print("Dir: ");
  u8x8.print(currentDirection);
  
  u8x8.clearLine(6);
  u8x8.setCursor(0, 6);
  u8x8.print("Speed: ");
  u8x8.print(map(currentSpeed, 0, PWM_MAX, 0, 100));
  u8x8.print("%");
  
  // 인코더 정보 표시 또는 모터 상태 표시
  if (showEncoderInfo) {
    u8x8.clearLine(7);
    u8x8.setCursor(0, 7);
    u8x8.print("FL:");
    u8x8.print(encoderFL_Count/100); // 표시용으로 100으로 나눔
    u8x8.print(" FR:");
    u8x8.print(encoderFR_Count/100);
  } else {
    u8x8.clearLine(7);
    u8x8.setCursor(0, 7);
    u8x8.print("Status: ");
    u8x8.print(motorRunning ? "RUNNING" : "STOPPED");
  }
}

// 연결 상태 콜백 클래스
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Device connected");
      
      u8x8.clearLine(0);
      u8x8.setCursor(0, 0);
      u8x8.print("Connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("Device disconnected");
      
      u8x8.clearLine(0);
      u8x8.setCursor(0, 0);
      u8x8.print("Disconnected");
      
      // 연결 해제 시 모터 정지
      setMecanumMotors(0, 0, 0, 0);
      currentDirection = "STOP";
      motorRunning = false;
      
      updateMotorStatusOnLCD();
    }
};

// LCD 화면 업데이트 함수
void updateLCDWithMessage(String message) {
  u8x8.clearLine(2);
  u8x8.setCursor(0, 2);
  u8x8.print("Received:");
  
  u8x8.clearLine(3);
  u8x8.clearLine(4);
  
  if (message.length() <= 16) {
    u8x8.setCursor(0, 3);
    u8x8.print(message);
  } else if (message.length() <= 32) {
    u8x8.setCursor(0, 3);
    u8x8.print(message.substring(0, 16));
    u8x8.setCursor(0, 4);
    u8x8.print(message.substring(16));
  } else {
    u8x8.setCursor(0, 3);
    u8x8.print(message.substring(0, 16));
    u8x8.setCursor(0, 4);
    u8x8.print(message.substring(16, 32));
    u8x8.print("...");
  }
  
  // LED 깜빡임
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  if (deviceConnected) {
    digitalWrite(LED_PIN, HIGH);
  }
}

// 명령 처리 함수 - 메카넘 휠 동작
void processCommand(String command) {
  command.trim();
  command.toLowerCase();
  
  // 모드 관리
  if (command == "auto") {
    isAutoMode = true;
    return;
  } 
  else if (command == "manual") {
    isAutoMode = false;
    return;
  }
  // 인코더 정보 표시 토글
  else if (command == "encoder") {
    showEncoderInfo = !showEncoderInfo;
    updateMotorStatusOnLCD();
    return;
  }
  // 인코더 카운터 리셋
  else if (command == "reset") {
    encoderFL_Count = 0;
    encoderFR_Count = 0;
    encoderRL_Count = 0;
    encoderRR_Count = 0;
    updateMotorStatusOnLCD();
    return;
  }
  
  // 메카넘 휠 이동 명령 처리
  if (command == "forward") {
    // 전진: 모든 바퀴 정방향
    setMecanumMotors(currentSpeed, currentSpeed, currentSpeed, currentSpeed);
    currentDirection = "FORWARD";
  } 
  else if (command == "backward") {
    // 후진: 모든 바퀴 역방향
    setMecanumMotors(-currentSpeed, -currentSpeed, -currentSpeed, -currentSpeed);
    currentDirection = "BACKWARD";
  } 
  else if (command == "left") {
    // 좌측 이동: FL,RR 역방향, FR,RL 정방향
    setMecanumMotors(-currentSpeed, currentSpeed, currentSpeed, -currentSpeed);
    currentDirection = "LEFT";
  } 
  else if (command == "right") {
    // 우측 이동: FL,RR 정방향, FR,RL 역방향
    setMecanumMotors(currentSpeed, -currentSpeed, -currentSpeed, currentSpeed);
    currentDirection = "RIGHT";
  }
  else if (command == "rotate_left") {
    // 좌회전: 좌측 바퀴 역방향, 우측 바퀴 정방향
    setMecanumMotors(-currentSpeed, currentSpeed, -currentSpeed, currentSpeed);
    currentDirection = "ROT_LEFT";
  }
  else if (command == "rotate_right") {
    // 우회전: 좌측 바퀴 정방향, 우측 바퀴 역방향
    setMecanumMotors(currentSpeed, -currentSpeed, currentSpeed, -currentSpeed);
    currentDirection = "ROT_RIGHT";
  }
  else if (command == "diagonal_fl") {
    // 전방 좌측 대각선: FR, RL만 작동
    setMecanumMotors(0, currentSpeed, currentSpeed, 0);
    currentDirection = "DIAG_FL";
  }
  else if (command == "diagonal_fr") {
    // 전방 우측 대각선: FL, RR만 작동  
    setMecanumMotors(currentSpeed, 0, 0, currentSpeed);
    currentDirection = "DIAG_FR";
  }
  else if (command == "stop") {
    // 정지: 모든 모터 정지
    setMecanumMotors(0, 0, 0, 0);
    currentDirection = "STOP";
    motorRunning = false;
  } 
  else if (command.startsWith("speed:")) {
    // 속도 설정
    String speedValue = command.substring(6);
    int speed = speedValue.toInt();
    currentSpeed = map(constrain(speed, 0, 100), 0, 100, 0, PWM_MAX);
    
    // 현재 방향에 따라 새 속도 적용
    if (motorRunning) {
      String direction = currentDirection;
      direction.toLowerCase();
      processCommand(direction);
    }
  }
  
  updateMotorStatusOnLCD();
}

// 특성 콜백 클래스
class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      
      if (value.length() > 0) {
        receivedMessage = "";
        Serial.print("Received: ");
        for (int i = 0; i < value.length(); i++) {
          Serial.print(value[i]);
          receivedMessage += value[i];
        }
        Serial.println();
        
        updateLCDWithMessage(receivedMessage);
        processCommand(receivedMessage);
        
        String response = "Received: " + receivedMessage;
        pCharacteristic->setValue(response.c_str());
        pCharacteristic->notify();
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32C3 Mecanum Wheel Robot (4 Motors)");
  
  // LED 핀 설정
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
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
  
  // I2C 시작 및 PCA9685 초기화
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(1000);
  
  // 모든 모터 정지 상태로 초기화
  setMecanumMotors(0, 0, 0, 0);
  
  // LCD 초기화
  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  u8x8.clear();
  u8x8.setCursor(0, 0);
  u8x8.print("ESP32C3 4Motor");
  u8x8.setCursor(0, 1);
  u8x8.print("Mecanum Robot");
  
  // BLE 초기화
  //BLEDevice::init("XIAO-ESP32C3");  // 원래 이름으로 복원
  BLEDevice::init("KIMSF1");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                    
  pCharacteristic->setCallbacks(new CharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("Mecanum Ready");
  
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE server started, waiting for connections...");
  
  u8x8.setCursor(0, 2);
  u8x8.print("BLE Ready!");
  
  updateMotorStatusOnLCD();
  
  // 시작 신호
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("4-Motor Mecanum setup complete");
}

void loop() {
  // 연결 상태 변경 감지
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarting advertising");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  // 인코더 정보 출력 (5초마다)
  static unsigned long lastEncoderPrint = 0;
  if (millis() - lastEncoderPrint > 5000) {
    Serial.print("Encoders - FL:");
    Serial.print(encoderFL_Count);
    Serial.print(" FR:");
    Serial.print(encoderFR_Count);
    Serial.print(" RL:");
    Serial.print(encoderRL_Count);
    Serial.print(" RR:");
    Serial.println(encoderRR_Count);
    lastEncoderPrint = millis();
  }
  
  delay(10);
}