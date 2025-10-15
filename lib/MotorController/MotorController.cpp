#include <MotorController.h>
#include <Configuration.h>

// --- 생성자 구현 ---
// MotorController::MotorController(int potPin, int rpwmPin, int lpwmPin, int maxSpeed, int accelStep, int updateInterval)
//     : _potPin(potPin), _rpwmPin(rpwmPin), _lpwmPin(lpwmPin),
//       _maxSpeed(maxSpeed), _accelStep(accelStep), _updateInterval(updateInterval), currentDirection(STOPPED)
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
    break;
  case BACKWARD:
    _currentPin = LPWM_PIN;
  default:
    break;
  }

  // if (STOPPED == requestedDir)
  // {
  //   _targetSpeed = 0; // 목표 속도를 0으로 설정하여 감속 시작
  // }
  // else
  // {
  //   _targetSpeed = _maxSpeed; // 최대 속도로 설정하여 가속 시작
  // }
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
  if (currentDirection == STOPPED)
  {
    _targetSpeed = 0; // 목표 속도를 0으로 설정하여 감속 시작
  } 
  else 
  {
    int pot = analogRead(POT_PIN);
    // Serial.print("가변저항 : ");
    // Serial.println(pot);

    _targetSpeed = map(pot, 1023, 0, 0, 255);
    // Serial.print("목표 속도: ");
    // Serial.println(_targetSpeed);
  }
  
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
  updateSpeed();
  
  if (_currentSpeed == 0)
  {
    // 모터가 완전히 정지했을 때 양쪽 핀 모두 LOW로 설정
    analogWrite(RPWM_PIN, 0);
    analogWrite(LPWM_PIN, 0);
    return;
  }

  analogWrite(_currentPin, _currentSpeed); // 현재 방향 핀에 PWM 출력

  Serial.print("현재 속도: ");
  Serial.print(_currentSpeed);
  Serial.print(" === 목표 속도: ");
  Serial.println(_targetSpeed);
}