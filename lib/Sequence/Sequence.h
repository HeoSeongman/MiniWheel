#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <Arduino.h>
#include "DirectionType.h"

class MoterController; // 전방 선언
class StateMachine; // 전방 선언

// --- Sequence 인터페이스 ---
class Sequence
{
public:
    Sequence(StateMachine *sm) : stateMachine(sm) {}
    virtual ~Sequence() {}
    virtual void enter() = 0;
    virtual void execute() = 0;
    virtual void exit() = 0;

    const char *getName() const
    {
        return name.c_str();
    }

protected:
    String name = "Unnamed Sequence";
    StateMachine *stateMachine; // StateMachine 객체에 접근할 수 있도록 추가
};

/// --- 개별 시퀀스 클래스 선언 ---

// ====================== StopSequence =================
class StopSequence : public Sequence
{
public:
    virtual void enter() override;
    virtual void execute() override;
    void exit() override {}

    StopSequence(StateMachine *sm) : Sequence(sm) { name = "StopSequence"; }
};

// ==================== ForwardSequence ==================
class ForwardSequence : public Sequence
{
public:
    virtual void enter() override;
    virtual void execute() override;
    void exit() override {}

    ForwardSequence(StateMachine *sm) : Sequence(sm) { name = "ForwardSequence"; }
};

// ==================== BackwardSequence =================
class BackwardSequence : public Sequence
{
public:
    virtual void enter() override;
    virtual void execute() override;
    void exit() override {}

    BackwardSequence(StateMachine *sm) : Sequence(sm) { name = "BackwardSequence"; }
};

// ==================== StoppingSequence =================
class StoppingSequence : public Sequence
{
private:
    Direction nextDirection;
    unsigned long stopStartTime = 0;
    unsigned long lastLedToggleTime = 0;
    // const unsigned long STOP_DURATION = 3000;
    // const int LED_PIN = 13;

public:
    // 생성자에서 StateMachine 인수를 제거하고, Direction만 받습니다.
    StoppingSequence(StateMachine *sm, Direction nextDir) : Sequence(sm), nextDirection(nextDir)
    {
        name = "StoppingSequence";
    }
    virtual void enter() override;
    virtual void execute() override;
    void exit() override;
};

#endif