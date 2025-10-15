#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>
#include "DirectionType.h"

class MotorController
{
public:
  // 생성자: 모터 드라이버 핀 번호 초기화
  // MotorController(int potPin, int rpwmPin, int lpwmPin, int maxSpeed, int accelStep, int updateInterval);

  // 모터 핀 설정
  void begin();

  // 목표 속도와 방향 설정
  void setDirection(Direction requestedDir);

  // 속도 가감속 및 정지 로직 업데이트 (loop()에서 주기적으로 호출)
  void update();

  int getCurrentSpeed() const { return _currentSpeed; }
  Direction getCurrentDirection() const { return currentDirection; }
  // 속도 변화가 완료되었는지 확인
  bool isRampFinished() const { return _currentSpeed == _targetSpeed; }

private:
  // // 핀 번호
  // int _potPin;
  // int _rpwmPin;
  // int _lpwmPin;
  int _currentPin;

  // // 제어 변수
  // int _maxSpeed;
  // int _accelStep;
  // unsigned int _updateInterval;

  // 상태 변수
  int _targetSpeed = 0;
  int _currentSpeed = 0;
  Direction currentDirection = STOPPED;

  // 실제 PWM 출력 함수
  void updateSpeed();
  void updateTargetSpeed();

  unsigned long lastUpdateTime = 0;
};

#endif