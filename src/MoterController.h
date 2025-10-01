#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>

enum Direction
{
  STOPPED,
  FORWARD,
  BACKWARD
};

class MotorController
{
public:
  // 생성자: 모터 드라이버 핀 번호 초기화
  MotorController(int rpwmPin, int lpwmPin, int maxSpeed, int accelStep, int updateInterval);

  // 모터 핀 설정
  void begin();

  // 목표 속도와 방향 설정
  void setDirection(Direction requestedDir);

  // 속도 가감속 및 정지 로직 업데이트 (loop()에서 주기적으로 호출)
  void update();

  // 현재 상태 확인용 함수
  int getCurrentSpeed() const { return currentSpeed; }
  Direction getCurrentDirection() const { return currentDirection; }
  bool isRampFinished() const { return currentSpeed == targetSpeed; }

private:
  // 핀 번호
  int _rpwmPin;
  int _lpwmPin;

  // 제어 변수
  int _maxSpeed;
  int _accelStep;
  int _updateInterval;

  // 상태 변수
  int targetSpeed = 0;
  int currentSpeed = 0;
  Direction currentDirection = STOPPED;

  // 실제 PWM 출력 함수
  void _setSpeedAndDirection(int speed, Direction dir);

  unsigned long lastUpdateTime = 0;
};

#endif