#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==============================================
// 하드웨어 핀 정의
// ==============================================

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

// 인코더 핀 정의
#define ENCODER_FL_A 0  // GPIO0 - 전방 좌측
#define ENCODER_FL_B 1  // GPIO1
#define ENCODER_FR_A 2  // GPIO2 - 전방 우측
#define ENCODER_FR_B 3  // GPIO3
#define ENCODER_RL_A 6  // GPIO6 - 후방 좌측
#define ENCODER_RL_B 7  // GPIO7
#define ENCODER_RR_A 8  // GPIO8 - 후방 우측
#define ENCODER_RR_B 9  // GPIO9

// 기타 핀 정의
#define LED_PIN 10      // 내장 LED

// ==============================================
// PWM 및 모터 설정
// ==============================================
#define PWM_MAX 4095
#define PWM_HALF 2048
#define PWM_FREQUENCY 1000

// ==============================================
// BLE 설정
// ==============================================
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_DEVICE_NAME     "KIMSF1"

// ==============================================
// 시스템 설정
// ==============================================
#define SERIAL_BAUD_RATE 115200
#define ENCODER_PRINT_INTERVAL 5000  // 5초마다 인코더 정보 출력

// ==============================================
// 모터 인덱스 열거형
// ==============================================
enum MotorIndex {
    MOTOR_FRONT_LEFT = 1,
    MOTOR_FRONT_RIGHT = 2,
    MOTOR_REAR_LEFT = 3,
    MOTOR_REAR_RIGHT = 4
};

// ==============================================
// 이동 방향 열거형
// ==============================================
enum Direction {
    DIR_STOP,
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_ROTATE_LEFT,
    DIR_ROTATE_RIGHT,
    DIR_DIAGONAL_FL,
    DIR_DIAGONAL_FR
};

#endif // CONFIG_H