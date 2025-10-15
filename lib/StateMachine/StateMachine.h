#pragma once

#include <Arduino.h>
#include <DirectionType.h>
#include "Sequence.h"

// class Sequence; // 전방 선언
// class StoppingSequence;
// class StopSequence;
// class ForwardSequence;
// class BackwardSequence;

class MotorController; // 전방 선언

class StateMachine
{
public:
private:
    // 현재 상태를 저장할 포인터
    Sequence *currentState = nullptr;
    // 다음 상태를 저장할 포인터 추가
    Sequence *nextState = nullptr;

    // int _forwrardButtonPin;
    // int _backwardButtonPin;
    // int _ledPin;
    // unsigned long _stopDuration;

    StopSequence stopSeq;
    ForwardSequence forwardSeq;
    BackwardSequence backwardSeq;
    StoppingSequence stoppingFBSeq; // Forward -> Backward 용
    StoppingSequence stoppingBFSeq; // Backward -> Forward 용

    MotorController *_motorController; // MotorController 객체에 접근할 수 있도록 추가

public:
    StateMachine(MotorController *motorController);

    // int getForwardButtonPin() const { return _forwrardButtonPin; }
    // int getBackwardButtonPin() const { return _backwardButtonPin; }
    // int getLedPin() const { return _ledPin; }
    // unsigned long getStopDuration() const { return _stopDuration; }

    StoppingSequence *getStoppingFBSeq() { return &stoppingFBSeq; }
    StoppingSequence *getStoppingBFSeq() { return &stoppingBFSeq; }
    StopSequence *getStopSeq() { return &stopSeq; }
    ForwardSequence *getForwardSeq() { return &forwardSeq; }
    BackwardSequence *getBackwardSeq() { return &backwardSeq; }

    MotorController *getMotorController() const { return _motorController; }

    // nextState는 이미 메모리에 존재하는 객체의 포인터여야 합니다.
    void transitionTo(Sequence *nextStatePtr);
    void update();
};