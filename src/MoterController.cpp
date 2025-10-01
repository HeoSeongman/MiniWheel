#include <MoterController.h>

// --- 생성자 구현 ---
MotorController::MotorController(int rpwmPin, int lpwmPin, int maxSpeed, int accelStep, int updateInterval)
    : _rpwmPin(rpwmPin), _lpwmPin(lpwmPin),
      _maxSpeed(maxSpeed), _accelStep(accelStep), _updateInterval(updateInterval), currentSpeed(0), targetSpeed(0), currentDirection(STOPPED)
{
  // 초기화 리스트에서 변수들을 초기화합니다.
}

// --- 초기화 함수 ---
void MotorController::begin()
{
  pinMode(_rpwmPin, OUTPUT); // 우회전 PWM 핀 설정
  pinMode(_lpwmPin, OUTPUT); // 좌회전 PWM 핀 설정

  // 초기 상태: 정지
  updateSpeed();
}

// --- 방향 설정 함수 ---
void MotorController::setDirection(Direction requestedDir)
{
  currentDirection = requestedDir;

  if (STOPPED == requestedDir)
  {
    targetSpeed = 0; // 목표 속도를 0으로 설정하여 감속 시작
  }
  else
  {
    targetSpeed = _maxSpeed; // 최대 속도로 설정하여 가속 시작
  }
}

// --- 실제 PWM 출력 함수 ---
void MotorController::updateSpeed()
{
  // currentSpeed = constrain(newCurrentSpeed, 0, 255); // PWM 값 범위 제한

  switch (currentDirection)
  {
  case FORWARD:
    analogWrite(_rpwmPin, currentSpeed);
    // analogWrite(_lpwmPin, 0);
    break;
  case BACKWARD:
    // analogWrite(_rpwmPin, 0);
    analogWrite(_lpwmPin, currentSpeed);
    break;
  case STOPPED:
    // analogWrite(_rpwmPin, 0);
    // analogWrite(_lpwmPin, 0);
    break;
  default:
    break;
  }
}

// --- 메인 업데이트 함수 (가/감속 처리) ---
void MotorController::update()
{
  if (millis() - lastUpdateTime < _updateInterval) // 경과된 시간이 설정된 주기보다 작으면 반환
  {
    return;
  }

  lastUpdateTime = millis();

  if (currentSpeed < targetSpeed) // 1. 가속 처리
  {
    currentSpeed += _accelStep;
    if (currentSpeed > targetSpeed)
    {
      currentSpeed = targetSpeed;
    }
    updateSpeed();
  }
  else if (currentSpeed > targetSpeed) // 2. 감속 처리
  {
    currentSpeed -= _accelStep;
    if (currentSpeed < targetSpeed)
    {
      currentSpeed = targetSpeed;
    }
    updateSpeed();
  }
}