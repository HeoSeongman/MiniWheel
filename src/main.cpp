#include <Arduino.h>
#include "./MoterController.h" // 새로 정의한 클래스 포함
#include "./Sequence.h"        // 시퀀스 관련 클래스 포함

// --- 핀 정의 ---
#define RPWM_PIN 9        // 정회전 PWM (PWM 핀)
#define LPWM_PIN 10       // 역회전 PWM (PWM 핀)
#define FORWARD_BUTTON_PIN 2  // 정회전 버튼 (LOW 입력)
#define BACKWARD_BUTTON_PIN 3 // 역회전 버튼 (LOW 입력)
#define LED_PIN 13     // 상태 표시 LED (내장 LED)

// --- 제어 상수 ---
#define MAX_SPEED 200      // 최대 PWM 값 (0-255)
#define ACCEL_STEP 2       // 가/감속 속도 단계
#define UPDATE_INTERVAL 20 // 가/감속 업데이트 주기 (ms)
#define STOP_DURATION 3000 // 방향 전환 시 정지 시간 (ms)

// --- 객체 생성 ---
// MotorController(RPWM 핀, LPWM 핀, 최대 속도, 가속 단계, 업데이트 간격)
MotorController motor(RPWM_PIN, LPWM_PIN, MAX_SPEED, ACCEL_STEP, UPDATE_INTERVAL);
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
  stateMachine.transitionTo(&stopSeq, "Stop");

  // 모터 컨트롤러 초기화
  motor.begin();
  Serial.println("초기화 완료.");
}

void loop()
{
  // checkButtons(); // 1. 버튼 입력 처리

  // if (isStoppingSequence)
  // {
  //   handleStoppingSequence(); // 2. 방향 전환 시 정지 시퀀스 처리
  // }

  stateMachine.update(); // 3. 상태 머신 업데이트 (현재 시퀀스 실행)
  motor.update();        // 4. 모터 제어 업데이트 (가/감속 실행)
}

/**
 * @brief 버튼 입력을 확인하고 모터의 목표 상태를 요청합니다.
 * 시퀀스: a, b, d의 요청 부분을 담당
 */
// void checkButtons()
// {
//   Direction currentDir = motor.getCurrentDirection();

//   if (digitalRead(FORWARD_BUTTON_PIN) == LOW) // 정회전 버튼이 눌렸을 때 (LOW)
//   {
//     if (currentDir == BACKWARD && !isStoppingSequence) // 현재 역회전 중이고 정지 시퀀스가 아닌 경우
//     {
//       // d. 역회전 중 반대 버튼: 정지 시퀀스 시작
//       isStoppingSequence = true;
//       stopStartTime = millis();
//       requestedDirection = FORWARD;
//       motor.setDirection(STOPPED, true); // 목표 속도를 0으로 설정하여 감속 시작
//       Serial.println("STOP Sequence: BACKWARD -> FORWARD");
//     }
//     else if (currentDir != FORWARD) // 현재 정회전이 아닌 경우
//     {
//       // a. 정지 또는 b. 역회전 후 정지 시퀀스 완료 시 -> 정회전 시작 (가속)
//       motor.setDirection(FORWARD);
//     }
//   }
//   else if (digitalRead(BACKWARD_BUTTON_PIN) == LOW) // 역회전 버튼이 눌렸을 때 (LOW)
//   {
//     if (currentDir == FORWARD && !isStoppingSequence) // 현재 정회전 중이고 정지 시퀀스가 아닌 경우
//     {
//       // d. 정회전 중 반대 버튼: 정지 시퀀스 시작
//       isStoppingSequence = true;
//       stopStartTime = millis();
//       requestedDirection = BACKWARD;
//       motor.setDirection(STOPPED, true); // 목표 속도를 0으로 설정하여 감속 시작
//       Serial.println("STOP Sequence: FORWARD -> BACKWARD");
//     }
//     else if (currentDir != BACKWARD) // 현재 역회전이 아닌 경우
//     {
//       // a. 정지 또는 b. 정회전 후 정지 시퀀스 완료 시 -> 역회전 시작 (가속)
//       motor.setDirection(BACKWARD);
//     }
//   }
// }

/**
 * @brief 방향 전환 시 서서히 정지 -> 대기 -> 서서히 반대 방향 회전을 처리합니다.
 * 시퀀스: d의 실행 부분을 담당
 */
// void handleStoppingSequence()
// {
//   // 1. 서서히 정지 단계 (목표 속도: 0, 현재 속도 > 0)
//   if (motor.getCurrentSpeed() > 0 || !motor.isRampFinished())
//   {
//     // 모터 컨트롤러의 update()가 감속을 처리

//     // LED 깜빡임: 정지하는 동안 (빠르게 100ms)
//     if (millis() - lastLedToggleTime >= 100)
//     {
//       digitalWrite(LED_PIN, !digitalRead(LED_PIN));
//       lastLedToggleTime = millis();
//     }
//     return; // 감속 중에는 다음 단계로 넘어가지 않음
//   }

//   // 2. 3초간 정지 및 대기 단계 (현재 속도 == 0)
//   if (millis() - stopStartTime < STOP_DURATION)
//   {
//     // 모터는 이미 정지 상태 (motor.update()에서 처리)

//     // LED 깜빡임: 1초마다 깜빡
//     if (millis() - lastLedToggleTime >= 1000)
//     {
//       digitalWrite(LED_PIN, !digitalRead(LED_PIN));
//       lastLedToggleTime = millis();
//     }
//     return; // 대기 중에는 다음 단계로 넘어가지 않음
//   }

//   // 3. 반대 회전 방향으로 서서히 회전 시작
//   isStoppingSequence = false; // 정지 시퀀스 종료

//   // 요청된 방향으로 최대 속도를 목표로 설정하여 가속 시작
//   motor.setDirection(requestedDirection);

//   // LED 끄기
//   digitalWrite(LED_PIN, LOW);
//   Serial.println("Rotation started in new direction.");
// }