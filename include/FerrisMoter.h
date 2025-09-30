#include <Arduino.h>

// 방향 열거형
enum Direction
{
    STOP,
    FORWARD,
    BACKWARD
};

// 모터 클래스
class Motor
{
private:
    int pinLEnable; // 좌회전 Enable
    int pinREnable; // 우회전 Enable
    int pinLPWM;    // 좌회전 PWM
    int pinRPWM;    // 우회전 PWM

    int minSpeed;     // 최소 속도
    int maxSpeed;     // 최대 속도
    int currentSpeed; // 현재 속도 (0 ~ 255)
    int targetSpeed;  // 목표 속도 (0 ~ 255)

    // int direction;    // 1 = forward, -1 = backward, 0 = stop
    Direction direction;
    int rampStep; // 가속/감속 단계

public:
    // 생성자
    Motor(int lEnable, int rEnable, int lPWM, int rPWM,
          int minS = 0, int maxS = 255, int ramp = 5)
        : pinLEnable(lEnable), pinREnable(rEnable),
          pinLPWM(lPWM), pinRPWM(rPWM),
          minSpeed(minS), maxSpeed(maxS),
          currentSpeed(0), targetSpeed(0),
          direction(Direction::STOP), rampStep(ramp)
    {
    }

    // 핀 초기화
    void begin()
    {
        pinMode(pinLEnable, OUTPUT);
        pinMode(pinREnable, OUTPUT);
        pinMode(pinLPWM, OUTPUT);
        pinMode(pinRPWM, OUTPUT);

        digitalWrite(pinLEnable, LOW);
        digitalWrite(pinREnable, LOW);

        analogWrite(pinLPWM, 0);
        analogWrite(pinRPWM, 0);
    }

    // 방향 설정
    void forward()
    {
        direction = Direction::FORWARD;
    }

    void backward()
    {
        direction = Direction::BACKWARD;
    }

    void stop()
    {
        direction = Direction::STOP;
        targetSpeed = 0;
    }

    // 목표 속도 설정 (클램핑)
    void setTargetSpeed(int speed)
    {
        if (speed < minSpeed)
            speed = minSpeed;
        if (speed > maxSpeed)
            speed = maxSpeed;
        targetSpeed = speed;
    }

    bool isStopped() { return (direction == STOP && currentSpeed == 0); }

    // 가속/감속 업데이트 (loop에서 주기적으로 호출)
    void update()
    {
        // 속도 조정
        if (currentSpeed < targetSpeed)
        {                             // 현재 속도가 목표 속도보다 낮으면 가속
            currentSpeed += rampStep; // 가속 단계만큼 증가
            if (currentSpeed > targetSpeed)
            { // 목표 속도 초과 방지
                currentSpeed = targetSpeed;
            }
        }
        else if (currentSpeed > targetSpeed)
        {                             // 현재 속도가 목표 속도보다 높으면 감속
            currentSpeed -= rampStep; // 감속 단계만큼 감소
            if (currentSpeed < targetSpeed)
            { // 목표 속도 미만 방지
                currentSpeed = targetSpeed;
            }
        }

        // 방향에 따라 Enable 및 PWM 출력
        if (direction == 1)
        { // 정방향
            digitalWrite(pinLEnable, HIGH);
            digitalWrite(pinREnable, LOW);
            analogWrite(pinLPWM, currentSpeed);
            analogWrite(pinRPWM, 0);
        }
        else if (direction == -1)
        { // 역방향
            digitalWrite(pinLEnable, LOW);
            digitalWrite(pinREnable, HIGH);
            analogWrite(pinLPWM, 0);
            analogWrite(pinRPWM, currentSpeed);
        }
        else
        { // 정지
            digitalWrite(pinLEnable, LOW);
            digitalWrite(pinREnable, LOW);
            analogWrite(pinLPWM, 0);
            analogWrite(pinRPWM, 0);
        }
    }

    // 현재 속도 반환
    int getCurrentSpeed() { return currentSpeed; }
};
