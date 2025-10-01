#include <Arduino.h>
#include "Sequence.h"

class StateMachine
{
private:
    Sequence *currentState = nullptr;
    // 다음 상태를 저장할 포인터 추가
    Sequence *nextState = nullptr;

public:
    // nextState는 이미 메모리에 존재하는 객체의 포인터여야 합니다.
    void transitionTo(Sequence *nextStatePtr, const char *name = "Unknown")
    {
        if (nextState != nullptr)
        {
            // 이미 전환 요청이 있다면, 현재 요청이 마지막 요청이 됩니다.
            Serial.println("Warning: Transition already requested, overwriting.");
        }
        nextState = nextStatePtr;
        // 디버깅 용
        Serial.print("Transition requested to: ");
        Serial.println(name);
    }

    void update()
    {
        // A. 현재 시퀀스 로직 실행
        if (currentState != nullptr) {
            currentState->execute();
        }

        // B. 지연된 전환 실행 (가장 마지막에 실행)
        if (nextState != nullptr) {
            
            // 1. 이전 상태 종료
            if (currentState != nullptr) {
                currentState->exit(); 
            }
            
            // 2. 상태 포인터 업데이트
            currentState = nextState;
            nextState = nullptr; // 다음 요청을 위해 초기화
            
            // 3. 새 상태 진입
            currentState->enter();
        }
    }
};