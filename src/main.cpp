#include <Arduino.h>
#include <MotorController.h> // 새로 정의한 클래스 포함
#include "StateMachine.h"    // 시퀀스 관련 클래스 포함
#include "Sequence.h"        // 시퀀스 관련 클래스 포함
#include "Configuration.h"  // 설정 파일 포함

// --- 객체 생성 ---
// MotorController(RPWM 핀, LPWM 핀, 최대 속도, 가속 단계, 업데이트 간격)
MotorController motorController;
StateMachine stateMachine(&motorController);

void setup()
{
  // 시리얼 통신 초기화
  Serial.begin(9600);

  // 버튼 핀 설정 (내부 풀업 사용)
  pinMode(FORWARD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACKWARD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

  // LED 핀 설정
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 상태 머신 초기 시퀀스 설정: 미리 생성된 객체의 포인터를 전달
  stateMachine.transitionTo(stateMachine.getStopSeq());

  // 모터 컨트롤러 초기화
  motorController.begin();
  Serial.println("초기화 완료.");
}

void loop()
{
  stateMachine.update();    // 상태 머신 업데이트 (현재 시퀀스 실행)
  motorController.update(); // 모터 제어 업데이트 (가/감속 실행)
  delay(500);               // 너무 빠른 루프 방지
}
