#pragma once

#include <Arduino.h>

// --- 핀 정의 ---
const int POT_PIN = A0;            // 가변 저항 (0 ~ 1023)
const int R_EN_PIN = 3;            // 정회전 EN 핀
const int L_EN_PIN = 4;            // 역회전 EN 핀
const int RPWM_PIN = 5;            // 정회전 PWM (PWM 핀) - 녹색 LED
const int LPWM_PIN = 6;            // 역회전 PWM (PWM 핀) - 황색 LED
const int BACKWARD_BUTTON_PIN = 7; // 역회전 버튼 (LOW 입력) - 오른쪽 버튼
const int FORWARD_BUTTON_PIN = 8;  // 정회전 버튼 (LOW 입력) - 가운데 버튼
const int STOP_BUTTON_PIN = 9;     // 정지 버튼 (LOW 입력) - 왼쪽 버튼
const int LED_PIN = 13;            // 상태 표시 LED (내장 LED)

// --- 제어 상수 ---
const int MAX_SPEED = 255;      // 최대 PWM 값 (0-255)
const int ACCEL_STEP = 2;       // 가/감속 속도 단계
const int UPDATE_INTERVAL = 20; // 가/감속 업데이트 주기 (ms)
const int STOP_DURATION = 3000; // 방향 전환 시 정지 시간 (ms)