#include "StateMachine.h"
// #include "Sequence.h"
#include <Arduino.h>

StateMachine::StateMachine(MotorController *motorController)
    // 1. 초기화 목록을 사용하여 인수가 필요한 멤버 변수를 초기화합니다.
    : stopSeq(this),
      forwardSeq(this),
      backwardSeq(this),
      stoppingFBSeq(this, BACKWARD),
      stoppingBFSeq(this, FORWARD),
      _motorController(motorController) // 포인터로 저장
{
  // 생성자 본문은 비워둡니다.
}

void StateMachine::transitionTo(Sequence *nextStatePtr)
{
  if (nextState != nullptr)
  {
    // 이미 전환 요청이 있다면, 현재 요청이 마지막 요청이 됩니다.
    Serial.println("경고: 이미 요청된 시퀀스가 존재합니다.");
  }
  nextState = nextStatePtr;
  // 디버깅 용
  Serial.print("요청된 시퀀스 : ");
  Serial.println(nextState->getName());
}

void StateMachine::update()
{
  // A. 현재 시퀀스 로직 실행
  if (currentState != nullptr)
  {
    currentState->execute();
  }

  // B. 지연된 전환 실행 (가장 마지막에 실행)
  if (nextState != nullptr)
  {

    // 1. 이전 상태 종료
    if (currentState != nullptr)
    {
      currentState->exit();
    }

    // 2. 상태 포인터 업데이트
    currentState = nextState;
    nextState = nullptr; // 다음 요청을 위해 초기화

    // 3. 새 상태 진입
    currentState->enter();
  }
}