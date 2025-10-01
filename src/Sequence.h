#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <Arduino.h>
#include <./MoterController.h>

// 전역 객체 선언 (다른 파일에서 사용하기 위함)
// 메인에서 인스턴스를 초기화할 때 이들을 연결합니다.
extern MotorController motor;
// extern bool isStoppingSequence; // 메인에서 사용하는 플래그 (현재 시퀀스가 정지 중인지)

// --- 1. Sequence 인터페이스 ---
class Sequence
{
public:
    virtual ~Sequence() {}
    virtual void enter() = 0;
    virtual void execute() = 0;
    virtual void exit() = 0;
};

// --- 2. StateMachine 클래스 (수정) ---
class StateMachine {
private:
    Sequence* currentState = nullptr;

public:
    // nextState는 이미 메모리에 존재하는 객체의 포인터여야 합니다.
    void transitionTo(Sequence* nextState, const char* name = "Unknown") {
        if (currentState != nullptr) {
            currentState->exit(); // 이전 시퀀스 종료 로직 실행
        }

        currentState = nextState;
        currentState->enter(); // 새 시퀀스 진입 로직 실행

        Serial.print("Transitioned to: ");
        Serial.println(name);
    }

    void update() {
        if (currentState != nullptr) {
            currentState->execute(); // 현재 시퀀스의 로직 실행
        }
    }
};

/// --- 3. 개별 시퀀스 클래스 선언 (수정: StateMachine& sm 인수 제거) ---

// ====================== StopSequence =================
class StopSequence : public Sequence {
public:
    // 생성자에서 StateMachine 인수를 제거합니다.
    void enter() override;
    void execute() override;
    void exit() override {}
};

// ==================== ForwardSequence ==================
class ForwardSequence : public Sequence
{
private:
    StateMachine &sm;

public:
    ForwardSequence(StateMachine &machine) : sm(machine) {}
    void enter() override;
    void execute() override;
    void exit() override {}
};

// ==================== BackwardSequence =================
class BackwardSequence : public Sequence
{
private:
    StateMachine &sm;

public:
    BackwardSequence(StateMachine &machine) : sm(machine) {}
    void enter() override;
    void execute() override;
    void exit() override {}
};

// ==================== StoppingSequence =================
class StoppingSequence : public Sequence {
private:
    Direction nextDirection;
    unsigned long stopStartTime = 0;
    unsigned long lastLedToggleTime = 0;
    const unsigned long STOP_DURATION = 3000;
    const int LED_PIN = 13;

public:
    // 생성자에서 StateMachine 인수를 제거하고, Direction만 받습니다.
    StoppingSequence(Direction nextDir) : nextDirection(nextDir) {} 
    void enter() override;
    void execute() override;
    void exit() override;
};

#endif