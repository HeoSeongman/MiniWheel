#include "Sequence.h"
#include "StateMachine.h"
#include "MotorController.h"
#include <Configuration.h>

// --- StopSequence 구현 (보드 On 시 정지 유지) ---
void StopSequence::enter()
{
    stateMachine->getMotorController()->setDirection(STOPPING); // 목표 속도 0, 방향 정지
}

void StopSequence::execute()
{
    if (digitalRead(FORWARD_BUTTON_PIN) == LOW)
    {
        // forwardSeq 객체를 사용하여 전환
        stateMachine->transitionTo(stateMachine->getForwardSeq());
    }
    else if (digitalRead(BACKWARD_BUTTON_PIN) == LOW)
    {
        // backwardSeq 객체를 사용하여 전환
        stateMachine->transitionTo(stateMachine->getBackwardSeq());
    }
}

// --- ForwardSequence 구현 (정회전) ---
void ForwardSequence::enter()
{
    stateMachine->getMotorController()->setDirection(FORWARD); // 목표 속도 MAX_SPEED, 방향 정회전 (가속 시작)
}

void ForwardSequence::execute()
{
    if (digitalRead(STOP_BUTTON_PIN) == LOW)
    {
        Serial.println("정지 버튼 눌림");
        // stateMachine 에게 StopSequence 객체를 사용하여 전환 요청
        stateMachine->transitionTo(stateMachine->getStopSeq());
        return;
    }

    if (digitalRead(BACKWARD_BUTTON_PIN) == LOW)
    { // 역회전 버튼이 눌렸을 때
        // stateMachine 에게 StoppingSequence(Forward->Backward) 객체를 사용하여 전환 요청
        stateMachine->transitionTo(stateMachine->getStoppingFBSeq());
    }
}

// --- BackwardSequence 구현 (역회전) ---
void BackwardSequence::enter()
{
    stateMachine->getMotorController()->setDirection(BACKWARD); // 목표 속도 MAX_SPEED, 방향 역회전 (가속 시작)
}

void BackwardSequence::execute()
{
    if (digitalRead(STOP_BUTTON_PIN) == LOW)
    {
        Serial.println("정지 버튼 눌림");
        // stateMachine 에게 StopSequence 객체를 사용하여 전환 요청
        stateMachine->transitionTo(stateMachine->getStopSeq());
        return;
    }

    if (digitalRead(FORWARD_BUTTON_PIN) == LOW)
    {
        // stateMachine 에게 StoppingSequence(Backward->Forward) 객체를 사용하여 전환 요청
        stateMachine->transitionTo(stateMachine->getStoppingBFSeq());
    }
}

// --- StoppingSequence 구현 (서서히 정지 -> 3초 대기 -> 반대 회전) ---
void StoppingSequence::enter()
{
    stateMachine->getMotorController()->setDirection(STOPPING); // 목표 속도 0, 방향 정지 (감속 시작)
    stopStartTime = 0;                     // 아직 정지 상태에 진입하지 않았으므로 0으로 유지
}

void StoppingSequence::execute()
{
    if (digitalRead(STOP_BUTTON_PIN) == LOW)
    {
        // stateMachine 에게 StopSequence 객체를 사용하여 전환 요청
        stateMachine->transitionTo(stateMachine->getStopSeq());
        return;
    }

    // Serial.println("StoppingSequence 실행 중...");
    // 1. 서서히 정지 단계
    // 모터가 완전히 정지하지 않았거나, 감속이 완료되지 않은 경우
    if (stateMachine->getMotorController()->getCurrentSpeed() > 0)
    {
        // Serial.println("모터가 완전히 정지하지 않았거나, 감속이 완료되지 않은 경우");
        // LED 깜빡임 (빠르게 100ms)
        if (millis() - lastLedToggleTime >= 100)
        {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            lastLedToggleTime = millis();
        }
        return;
    }

    // 정지 완료 시점 기록
    if (stateMachine->getMotorController()->getCurrentSpeed() == 0 && stopStartTime == 0)
    {
        stopStartTime = millis(); // 정지 완료 시점 기록
        Serial.print("모터 정지됨. ");
        Serial.print(STOP_DURATION / 1000);
        Serial.println("초간 대기 시작");
        return;
    }

    // 2. 3초간 정지 및 대기 단계 (stopStartTime이 기록된 후)
    if (stopStartTime > 0 && millis() - stopStartTime < STOP_DURATION)
    {
        // LED 깜빡임 (느리게 1000ms)
        if (millis() - lastLedToggleTime >= 1000)
        {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            lastLedToggleTime = millis();
        }
        return;
    }

    // 3. 반대 회전 방향으로 서서히 회전 시작 (대기 종료)
    if (stopStartTime > 0 && millis() - stopStartTime >= STOP_DURATION)
    {
        Serial.print("모터 시작됨. ");
        Serial.print(STOP_DURATION / 1000);
        Serial.println("초간 대기 종료");
        if (nextDirection == FORWARD)
        {
            // forwardSeq 객체를 사용하여 전환
            stateMachine->transitionTo(stateMachine->getForwardSeq());
        }
        else if (nextDirection == BACKWARD)
        {
            // backwardSeq 객체를 사용하여 전환
            stateMachine->transitionTo(stateMachine->getBackwardSeq());
        }
    }
}

void StoppingSequence::exit()
{
    digitalWrite(LED_PIN, LOW); // LED 끄기
}