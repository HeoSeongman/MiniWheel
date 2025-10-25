#include <Arduino.h>

// --- 핀 정의 ---
const int POT_PIN = A0;            // 가변 저항 (0 ~ 1023)
const int R_EN_PIN = 3;            // 정회전 EN 핀
const int L_EN_PIN = 4;            // 역회전 EN 핀
const int RPWM_PIN = 5;            // 정회전 PWM (PWM 핀)
const int LPWM_PIN = 6;            // 역회전 PWM (PWM 핀)
const int BACKWARD_BUTTON_PIN = 7; // 역회전 버튼
const int FORWARD_BUTTON_PIN = 8;  // 정회전 버튼
const int STOP_BUTTON_PIN = 9;     // 정지 버튼
const int LED_PIN = 13;            // 상태 표시 LED

// --- 제어 상수 ---
const int MAX_SPEED = 255;      // 최대 PWM 값 (0-255)
const int ACCEL_STEP = 5;       // 가/감속 속도 단계
const int UPDATE_INTERVAL = 20; // 가/감속 업데이트 주기 (ms)
const int STOP_DURATION = 3000; // 방향 전환 시 정지 시간 (ms)

// --- 상태 열거형 ---
enum Direction
{
  STOP,
  STOPPING,
  FORWARD,
  BACKWARD
};

// --- 모터 컨트롤러 클래스 ---
class MotorController
{
public:
  void begin();
  void setDirection(Direction requestedDir);
  void update();
  int getCurrentSpeed() const { return _currentSpeed; }
  Direction getCurrentDirection() const { return currentDirection; }
  bool isRampFinished() const { return _currentSpeed == _targetSpeed; }

private:
  int _currentPin;
  int _targetSpeed = 0;
  int _currentSpeed = 0;
  Direction currentDirection = STOP;
  unsigned long lastUpdateTime = 0;

  void updateSpeed();
  void updateTargetSpeed();
};

// --- 전역 객체 및 변수 ---
MotorController motorController;
Direction currentState = STOP;
Direction requestedState = STOP; // 사용자가 요청한 다음 상태
unsigned long transitionTimestamp = 0;

// --- setup() 함수 ---
void setup()
{
  Serial.begin(9600);

  // 버튼 핀을 내부 풀업 저항으로 설정
  pinMode(FORWARD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACKWARD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

  // LED 핀 설정
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 모터 컨트롤러 초기화
  motorController.begin();
  Serial.println("초기화 완료. 시스템 시작.");
}

// --- loop() 함수 ---
void loop()
{
  // 1. 버튼 입력 감지 (INPUT_PULLUP이므로 LOW일 때 눌림)
  if (digitalRead(FORWARD_BUTTON_PIN) == LOW)
  {
    requestedState = FORWARD;
  }
  if (digitalRead(BACKWARD_BUTTON_PIN) == LOW)
  {
    requestedState = BACKWARD;
  }
  if (digitalRead(STOP_BUTTON_PIN) == LOW)
  {
    requestedState = STOP;
  }

  // 2. 상태 플래그에 따른 로직 처리
  switch (currentState)
  {
  case STOP:
    digitalWrite(LED_PIN, LOW); // 정지 상태일 때 LED 끔
    motorController.setDirection(STOP);

    if (requestedState == FORWARD && currentState != FORWARD)
    {
      currentState = FORWARD;
      motorController.setDirection(FORWARD);
      Serial.println("상태 변경: 정지 -> 정회전");
    }
    else if (requestedState == BACKWARD && currentState != BACKWARD)
    {
      currentState = BACKWARD;
      motorController.setDirection(BACKWARD);
      Serial.println("상태 변경: 정지 -> 역회전");
    }
    break;

  case FORWARD:
    digitalWrite(LED_PIN, HIGH); // 동작 상태일 때 LED 켬

    // 반대 방향 또는 정지 요청 시 '정지 중' 상태로 전환
    if (requestedState == BACKWARD || requestedState == STOP)
    {
      currentState = STOPPING;
      transitionTimestamp = millis();    // 상태 전환 시간 기록
      motorController.setDirection(STOPPING); // 모터 감속 시작
      Serial.println("상태 변경: 정회전 -> 정지 중");
    }
    break;

  case BACKWARD:
    digitalWrite(LED_PIN, HIGH); // 동작 상태일 때 LED 켬

    // 반대 방향 또는 정지 요청 시 '정지 중' 상태로 전환
    if (requestedState == FORWARD || requestedState == STOP)
    {
      currentState = STOPPING;
      transitionTimestamp = millis();
      motorController.setDirection(STOPPING);
      Serial.println("상태 변경: 역회전 -> 정지 중");
    }
    break;

  case STOPPING:
    // LED를 깜빡여 '정지 중' 상태 표시
    digitalWrite(LED_PIN, (millis() / 500) % 2);

    // 모터가 완전히 멈췄는지 확인
    if (motorController.getCurrentSpeed() == 0)
    {
      // 정지 후 일정 시간(STOP_DURATION)이 지났는지 확인
      if (millis() - transitionTimestamp >= STOP_DURATION)
      {
        if (requestedState == FORWARD)
        {
          currentState = FORWARD;
          motorController.setDirection(FORWARD);
          Serial.println("상태 변경: 정지 중 -> 정회전");
        }
        else if (requestedState == BACKWARD)
        {
          currentState = BACKWARD;
          motorController.setDirection(BACKWARD);
          Serial.println("상태 변경: 정지 중 -> 역회전");
        }
        else
        {
          currentState = STOP; // 정지 요청이었으면 정지 상태로 전환
          Serial.println("상태 변경: 정지 중 -> 정지");
        }
      }
    }
    break;
  }

  // 3. 모터 컨트롤러 업데이트 (주기적으로 속도 제어)
  motorController.update();
}

// --- MotorController 메소드 구현 ---
void MotorController::begin()
{
  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  pinMode(R_EN_PIN, OUTPUT);
  pinMode(L_EN_PIN, OUTPUT);

  // 초기 상태: 정지
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
  updateSpeed();
}

void MotorController::setDirection(Direction requestedDir)
{
  // STOPPING 상태가 아닐 때만 현재 방향을 업데이트
  if (requestedDir != STOPPING)
  {
    currentDirection = requestedDir;
  }

  switch (requestedDir)
  {
  case FORWARD:
    _currentPin = RPWM_PIN;
    digitalWrite(L_EN_PIN, LOW);
    digitalWrite(R_EN_PIN, HIGH);
    break;
  case BACKWARD:
    _currentPin = LPWM_PIN;
    digitalWrite(R_EN_PIN, LOW);
    digitalWrite(L_EN_PIN, HIGH);
    break;
  case STOPPING:
    _targetSpeed = 0; // 목표 속도를 0으로 설정하여 감속
    break;
  case STOP:
    _targetSpeed = 0;
    _currentSpeed = 0;
    digitalWrite(R_EN_PIN, LOW);
    digitalWrite(L_EN_PIN, LOW);
    analogWrite(RPWM_PIN, 0);
    analogWrite(LPWM_PIN, 0);
    break;
  }
}

void MotorController::updateSpeed()
{
  if (_currentSpeed < _targetSpeed)
  {
    _currentSpeed += ACCEL_STEP;
    if (_currentSpeed > _targetSpeed)
    {
      _currentSpeed = _targetSpeed;
    }
  }
  else if (_currentSpeed > _targetSpeed)
  {
    _currentSpeed -= ACCEL_STEP;
    if (_currentSpeed < 0)
    {
      _currentSpeed = 0;
    }
  }
}

void MotorController::updateTargetSpeed()
{
  // 정지 또는 정지 중 상태가 아닐 때만 속도 업데이트
  if (currentState == FORWARD || currentState == BACKWARD)
  {
    int pot = analogRead(POT_PIN);
    _targetSpeed = map(pot, 0, 1023, 0, MAX_SPEED);
  }
}

void MotorController::update()
{
  if (millis() - lastUpdateTime < UPDATE_INTERVAL)
  {
    return;
  }
  lastUpdateTime = millis();

  updateTargetSpeed();
  updateSpeed();

  if (currentState != STOP)
  {
    analogWrite(_currentPin, _currentSpeed);
  }

  if (Serial.peek() == -1) { // 시리얼 버퍼가 비어있을 때만 출력
    Serial.print("현재 상태: ");
    Serial.print(currentState);
    Serial.print(" | 현재 속도: ");
    Serial.print(_currentSpeed);
    Serial.print(" | 목표 속도: ");
    Serial.println(_targetSpeed);
  }
}
