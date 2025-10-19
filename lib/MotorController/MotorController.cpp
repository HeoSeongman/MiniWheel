#include <MotorController.h>
#include <Configuration.h>

// --- 생성자 구현 ---
// MotorController::MotorController(int potPin, int rpwmPin, int lpwmPin, int maxSpeed, int accelStep, int updateInterval)
//     : _potPin(potPin), _rpwmPin(rpwmPin), _lpwmPin(lpwmPin),
//       _maxSpeed(maxSpeed), _accelStep(accelStep), _updateInterval(updateInterval), currentDirection(STOP)
// {
//   // 초기화 리스트에서 변수들을 초기화합니다.
// }

// --- 초기화 함수 ---
void MotorController::begin()
{
  pinMode(RPWM_PIN, OUTPUT); // 우회전 PWM 핀 설정
  pinMode(LPWM_PIN, OUTPUT); // 좌회전 PWM 핀 설정

  // 초기 상태: 정지
  updateSpeed();
}

// --- 방향 설정 함수 ---
void MotorController::setDirection(Direction requestedDir)
{
  currentDirection = requestedDir;

  switch (requestedDir)
  {
  case FORWARD:
    _currentPin = RPWM_PIN;
    digitalWrite(R_EN_PIN, HIGH); // 정회전 EN 핀 활성화
    break;
  case BACKWARD:
    _currentPin = LPWM_PIN;
    digitalWrite(L_EN_PIN, HIGH); // 역회전 EN 핀 활성화
    break;
  case STOPPING:
    _targetSpeed = 0; // 목표 속도를 0으로 설정
    break;
  case STOP:
    digitalWrite(R_EN_PIN, LOW); // 정회전 EN 핀 비활성화
    digitalWrite(L_EN_PIN, LOW); // 역회전 EN 핀 비활성화
  default:
    break;
  }
}

// --- 실제 PWM 출력 함수 ---
void MotorController::updateSpeed()
{
  if (_currentSpeed < _targetSpeed) // 1. 가속 처리
  {
    Serial.print("가속 처리: ");
    _currentSpeed += ACCEL_STEP;
    if (_currentSpeed > _targetSpeed)
    {
      _currentSpeed = _targetSpeed;
    }
    // updateSpeed();
  }
  else if (_currentSpeed > _targetSpeed) // 2. 감속 처리
  {
    Serial.print("감속 처리: ");
    _currentSpeed -= ACCEL_STEP;
    if (_currentSpeed < _targetSpeed)
    {
      _currentSpeed = _targetSpeed;
    }
    // updateSpeed();
  }
}

void MotorController::updateTargetSpeed()
{
  // 가변 저항을 읽어 목표 속도 설정
  int pot = analogRead(POT_PIN);
  // Serial.print("가변저항 : ");
  // Serial.println(pot);

  _targetSpeed = map(pot, 1023, 0, 0, 255);
  // Serial.print("목표 속도: ");
  // Serial.println(_targetSpeed);
}

// --- 메인 업데이트 함수 (가/감속 처리) ---
void MotorController::update()
{
  if (millis() - lastUpdateTime < UPDATE_INTERVAL) // 경과된 시간이 설정된 주기보다 작으면 반환
  {
    return;
  }

  lastUpdateTime = millis();
  updateTargetSpeed();
  
  if (currentDirection == STOP)
  {
    return; // 정지 상태에서는 아무 작업도 하지 않음
  }
  
  updateSpeed();
  analogWrite(_currentPin, _currentSpeed); // 현재 방향 핀에 PWM 출력

  if (_currentSpeed == 0 && currentDirection != STOP)
  {
    currentDirection = STOP; // 속도가 0이 되면 방향을 STOP으로 설정
  }
  
  Serial.print("현재 속도: ");
  Serial.print(_currentSpeed);
  Serial.print(" === 목표 속도: ");
  Serial.println(_targetSpeed);
}