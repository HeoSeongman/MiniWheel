#include "Sequence.h"

// --- 핀 정의 ---
#define FORWARD_BUTTON_PIN 2    // 정회전 버튼 (LOW 입력)
#define BACKWARD_BUTTON_PIN 3   // 역회전 버튼 (LOW 입력)
#define STATUS_LED 13

// --- 전역 객체 선언 (다른 시퀀스에서 접근하기 위함) ---
extern StateMachine stateMachine;
extern StopSequence stopSeq;
extern ForwardSequence forwardSeq;
extern BackwardSequence backwardSeq;

// StoppingSequence는 전환될 때마다 필요한 Direction 인수가 다르므로,
// 메인에서 두 개의 인스턴스(F->B용, B->F용)를 미리 생성한다고 가정합니다.
extern StoppingSequence stoppingFBSeq; // Forward -> Backward 용
extern StoppingSequence stoppingBFSeq; // Backward -> Forward 용

// --- StopSequence 구현 (보드 On 시 정지 유지) ---
void StopSequence::enter() {
    motor.setDirection(STOPPED); // 목표 속도 0, 방향 정지
}

void StopSequence::execute() {
    if (digitalRead(FORWARD_BUTTON_PIN) == LOW) {
        // 미리 생성된 forwardSeq 객체를 사용하여 전환
        stateMachine.transitionTo(&forwardSeq, "Forward"); 
    } else if (digitalRead(BACKWARD_BUTTON_PIN) == LOW) {
        // 미리 생성된 backwardSeq 객체를 사용하여 전환
        stateMachine.transitionTo(&backwardSeq, "Backward");
    }
}

// --- ForwardSequence 구현 (정회전) ---
void ForwardSequence::enter() {
    motor.setDirection(FORWARD); // 목표 속도 MAX_SPEED, 방향 정회전 (가속 시작)
}

void ForwardSequence::execute() {
    if (digitalRead(BACKWARD_BUTTON_PIN) == LOW) { // 역회전 버튼이 눌렸을 때
        // stateMachine 에게 StoppingSequence(Forward->Backward) 객체를 사용하여 전환 요청
        stateMachine.transitionTo(&stoppingFBSeq, "Stopping (F->B)");
    }
}

// --- BackwardSequence 구현 (역회전) ---
void BackwardSequence::enter() {
    motor.setDirection(BACKWARD); // 목표 속도 MAX_SPEED, 방향 역회전 (가속 시작)
}

void BackwardSequence::execute() {
    if (digitalRead(FORWARD_BUTTON_PIN) == LOW) {
        // stateMachine 에게 StoppingSequence(Backward->Forward) 객체를 사용하여 전환 요청
        stateMachine.transitionTo(&stoppingBFSeq, "Stopping (B->F)");
    }
}

// --- StoppingSequence 구현 (서서히 정지 -> 3초 대기 -> 반대 회전) ---
void StoppingSequence::enter() {
    motor.setDirection(STOPPED, true); // 목표 속도를 0으로 설정하여 감속 시작
    // isStoppingSequence = true;
    stopStartTime = 0; // 아직 정지 상태에 진입하지 않았으므로 0으로 유지
}

void StoppingSequence::execute() {
    // 1. 서서히 정지 단계
    if (motor.getCurrentSpeed() > 0 || !motor.isRampFinished()) {
        // LED 깜빡임 (빠르게 100ms)
        if (millis() - lastLedToggleTime >= 100) { 
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            lastLedToggleTime = millis();
        }
        // 정지 완료 시점 기록
        if (motor.isRampFinished() && motor.getCurrentSpeed() == 0) {
            stopStartTime = millis(); // 정지 완료 시점 기록
            Serial.println("Motor fully stopped. Starting 3s hold.");
        }
        return; 
    }
    
    // 2. 3초간 정지 및 대기 단계 (stopStartTime이 기록된 후)
    if (stopStartTime > 0 && millis() - stopStartTime < STOP_DURATION) {
        // LED 깜빡임 (느리게 1000ms)
        if (millis() - lastLedToggleTime >= 1000) { 
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            lastLedToggleTime = millis();
        }
        return;
    }

    // 3. 반대 회전 방향으로 서서히 회전 시작 (대기 종료)
    if (stopStartTime > 0 && millis() - stopStartTime >= STOP_DURATION) {
        if (nextDirection == FORWARD) {
            // 미리 생성된 forwardSeq 객체를 사용하여 전환
            stateMachine.transitionTo(&forwardSeq, "Forward");
        } else if (nextDirection == BACKWARD) {
            // 미리 생성된 backwardSeq 객체를 사용하여 전환
            stateMachine.transitionTo(&backwardSeq, "Backward");
        }
        // *주의*: 여기서 delete this를 하지 않습니다!
    }
}

void StoppingSequence::exit() {
    // isStoppingSequence = false;
    digitalWrite(LED_PIN, LOW); // LED 끄기
}