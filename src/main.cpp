#include <Arduino.h>
#include "./MoterController.h" // 새로 정의한 클래스 포함
#include "./Sequence.h"        // 시퀀스 관련 클래스 포함

// --- 핀 정의 ---
const int RPWM_PIN = 9;            // 정회전 PWM (PWM 핀)
const int LPWM_PIN = 10;           // 역회전 PWM (PWM 핀)
const int FORWARD_BUTTON_PIN = 2;  // 정회전 버튼 (LOW 입력)
const int BACKWARD_BUTTON_PIN = 3; // 역회전 버튼 (LOW 입력)
const int LED_PIN = 13;            // 상태 표시 LED (내장 LED)

// --- 제어 상수 ---
const int MAX_SPEED = 200;      // 최대 PWM 값 (0-255)
const int ACCEL_STEP = 2;       // 가/감속 속도 단계
const int UPDATE_INTERVAL = 20; // 가/감속 업데이트 주기 (ms)
const int STOP_DURATION = 3000; // 방향 전환 시 정지 시간 (ms)

// --- 객체 생성 ---
// MotorController(RPWM 핀, LPWM 핀, 최대 속도, 가속 단계, 업데이트 간격)
MotorController motorController(RPWM_PIN, LPWM_PIN, MAX_SPEED, ACCEL_STEP, UPDATE_INTERVAL);
StateMachine stateMachine;

// --- 상태 변수 ---
// bool isStoppingSequence = false;
Direction requestedDirection = STOPPED;
unsigned long stopStartTime = 0;
unsigned long lastLedToggleTime = 0;

// 기본 시퀀스 객체 생성 (재사용 대상)
StopSequence stopSeq;
ForwardSequence forwardSeq;
BackwardSequence backwardSeq;

// 전환용 StoppingSequence 객체 생성 (인수가 필요하므로 2개 생성)
// F->B (정회전 -> 역회전) 전환을 요청할 때 사용
StoppingSequence stoppingFBSeq(BACKWARD);
// B->F (역회전 -> 정회전) 전환을 요청할 때 사용
StoppingSequence stoppingBFSeq(FORWARD);

// void checkButtons();
// void handleStoppingSequence();

void setup()
{
  // 시리얼 통신 초기화
  Serial.begin(9600);

  // 버튼 핀 설정 (내부 풀업 사용)
  pinMode(FORWARD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACKWARD_BUTTON_PIN, INPUT_PULLUP);

  // LED 핀 설정
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 상태 머신 초기 시퀀스 설정: 미리 생성된 객체의 포인터를 전달
  stateMachine.transitionTo(&stopSeq);

  // 모터 컨트롤러 초기화
  motorController.begin();
  Serial.println("초기화 완료.");
}

void loop()
{
  stateMachine.update();    // 상태 머신 업데이트 (현재 시퀀스 실행)
  motorController.update(); // 모터 제어 업데이트 (가/감속 실행)
}
