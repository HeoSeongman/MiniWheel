#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <Arduino.h>
#include <./MoterController.h>
#include "./StateMachine.h"

// 전역 객체 선언 (다른 파일에서 사용하기 위함)
// 메인에서 인스턴스를 초기화할 때 이들을 연결합니다.
extern MotorController motorController;
// extern bool isStoppingSequence; // 메인에서 사용하는 플래그 (현재 시퀀스가 정지 중인지)

// --- Sequence 인터페이스 ---
class Sequence
{
public:
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
};

/// --- 개별 시퀀스 클래스 선언 ---

// ====================== StopSequence =================
class StopSequence : public Sequence
{
public:
    void enter() override;
    void execute() override;
    void exit() override {}

private:
    StopSequence() { name = "StopSequence"; }
};

// ==================== ForwardSequence ==================
class ForwardSequence : public Sequence
{
public:
    void enter() override;
    void execute() override;
    void exit() override {}

private:
    ForwardSequence() { name = "ForwardSequence"; }
};

// ==================== BackwardSequence =================
class BackwardSequence : public Sequence
{
public:
    void enter() override;
    void execute() override;
    void exit() override {}

private:
    BackwardSequence() { name = "BackwardSequence"; }
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
    StoppingSequence(Direction nextDir) : nextDirection(nextDir)
    {
        name = "StoppingSequence";
    }
    void enter() override;
    void execute() override;
    void exit() override;
};

#endif