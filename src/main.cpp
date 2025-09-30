#include <Arduino.h>
#include "./MoterController.h" // 새로 정의한 클래스 포함

// --- 핀 정의 ---
#define RPWM_PIN 9        // 정회전 PWM (PWM 핀)
#define LPWM_PIN 10       // 역회전 PWM (PWM 핀)
#define FORWARD_BUTTON 2  // 정회전 버튼 (LOW 입력)
#define BACKWARD_BUTTON 3 // 역회전 버튼 (LOW 입력)
#define STATUS_LED 13     // 상태 표시 LED (내장 LED)

// --- 제어 상수 ---
#define MAX_SPEED 200      // 최대 PWM 값 (0-255)
#define ACCEL_STEP 2       // 가/감속 속도 단계
#define UPDATE_INTERVAL 20 // 가/감속 업데이트 주기 (ms)
#define STOP_DURATION 3000 // 방향 전환 시 정지 시간 (ms)

// --- 객체 생성 ---
// MotorController(RPWM 핀, LPWM 핀, 최대 속도, 가속 단계, 업데이트 간격)
MotorController motor(RPWM_PIN, LPWM_PIN, MAX_SPEED, ACCEL_STEP, UPDATE_INTERVAL);

// --- 상태 변수 ---
bool isStoppingSequence = false;
Direction requestedDirection = STOPPED;
unsigned long stopStartTime = 0;
unsigned long lastLedToggleTime = 0;

void checkButtons();
void handleStoppingSequence();

void setup()
{
  // 시리얼 통신 초기화
  Serial.begin(9600);

  // 버튼 핀 설정 (내부 풀업 사용)
  pinMode(FORWARD_BUTTON, INPUT_PULLUP);
  pinMode(BACKWARD_BUTTON, INPUT_PULLUP);

  // LED 핀 설정
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  // 모터 컨트롤러 초기화
  motor.begin();
  Serial.println("초기화 완료.");
}

void loop()
{
  checkButtons(); // 1. 버튼 입력 처리

  if (isStoppingSequence) 
  {
    handleStoppingSequence(); // 2. 방향 전환 시 정지 시퀀스 처리
  }

  motor.update(); // 3. 모터 제어 업데이트 (가/감속 실행)
}

/**
 * @brief 버튼 입력을 확인하고 모터의 목표 상태를 요청합니다.
 * 시퀀스: a, b, d의 요청 부분을 담당
 */
void checkButtons()
{
  Direction currentDir = motor.getCurrentDirection();

  if (digitalRead(FORWARD_BUTTON) == LOW) // 정회전 버튼이 눌렸을 때 (LOW)
  {
    if (currentDir == BACKWARD && !isStoppingSequence)
    {
      // d. 역회전 중 반대 버튼: 정지 시퀀스 시작
      isStoppingSequence = true;
      stopStartTime = millis();
      requestedDirection = FORWARD;
      motor.setTarget(STOPPED, true); // 목표 속도를 0으로 설정하여 감속 시작
      Serial.println("STOP Sequence: BACKWARD -> FORWARD");
    }
    else if (currentDir != FORWARD)
    {
      // a. 정지 또는 b. 역회전 후 정지 시퀀스 완료 시 -> 정회전 시작 (가속)
      motor.setTarget(FORWARD);
    }
  }
  else if (digitalRead(BACKWARD_BUTTON) == LOW) // 역회전 버튼이 눌렸을 때 (LOW)
  {
    if (currentDir == FORWARD && !isStoppingSequence)
    {
      // d. 정회전 중 반대 버튼: 정지 시퀀스 시작
      isStoppingSequence = true;
      stopStartTime = millis();
      requestedDirection = BACKWARD;
      motor.setTarget(STOPPED, true); // 목표 속도를 0으로 설정하여 감속 시작
      Serial.println("STOP Sequence: FORWARD -> BACKWARD");
    }
    else if (currentDir != BACKWARD)
    {
      // a. 정지 또는 b. 정회전 후 정지 시퀀스 완료 시 -> 역회전 시작 (가속)
      motor.setTarget(BACKWARD);
    }
  }
}

/**
 * @brief 방향 전환 시 서서히 정지 -> 대기 -> 서서히 반대 방향 회전을 처리합니다.
 * 시퀀스: d의 실행 부분을 담당
 */
void handleStoppingSequence()
{
  // 1. 서서히 정지 단계 (목표 속도: 0, 현재 속도 > 0)
  if (motor.getCurrentSpeed() > 0 || !motor.isRampFinished())
  {
    // 모터 컨트롤러의 update()가 감속을 처리

    // LED 깜빡임: 정지하는 동안 (빠르게 100ms)
    if (millis() - lastLedToggleTime >= 100)
    {
      digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
      lastLedToggleTime = millis();
    }
    return; // 감속 중에는 다음 단계로 넘어가지 않음
  }

  // 2. 3초간 정지 및 대기 단계 (현재 속도 == 0)
  if (millis() - stopStartTime < STOP_DURATION)
  {
    // 모터는 이미 정지 상태 (motor.update()에서 처리)

    // LED 깜빡임: 1초마다 깜빡
    if (millis() - lastLedToggleTime >= 1000)
    {
      digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
      lastLedToggleTime = millis();
    }
    return; // 대기 중에는 다음 단계로 넘어가지 않음
  }

  // 3. 반대 회전 방향으로 서서히 회전 시작
  isStoppingSequence = false; // 정지 시퀀스 종료

  // 요청된 방향으로 최대 속도를 목표로 설정하여 가속 시작
  motor.setTarget(requestedDirection);

  // LED 끄기
  digitalWrite(STATUS_LED, LOW);
  Serial.println("Rotation started in new direction.");
}