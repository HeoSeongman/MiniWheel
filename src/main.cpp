#include <Arduino.h>
#include "../include/FerrisMoter.h"

// 핀 설정
const int leftEnablePin = 7;
const int rightEnablePin = 8;
const int leftPWMPin = 5;
const int rightPWMPin = 6;

const int forwardBtn = 2;    // 정방향 버튼
const int reverseBtn = 3;    // 역방향 버튼
const int potentialPin = A0; // 가변저항 연결 핀
const int ledPin = 13;       // 상태 LED

Motor motor(leftEnablePin, rightEnablePin, leftPWMPin, rightPWMPin);

enum State
{
  IDLE,
  RUNNING,
  SWITCHING
};

State sysState = IDLE;
unsigned long stateStart = 0;

void setup()
{
  motor.begin();

  pinMode(forwardBtn, INPUT_PULLUP);
  pinMode(reverseBtn, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("시스템 시작");
}

void loop()
{
  int rawPotential = analogRead(potentialPin);
  int potentialSpeed = map(rawPotential, 0, 1023, 0, 255);

  motor.setMaxSpeed(potentialSpeed);
  motor.update();

  switch (sysState)
  {
  case IDLE:                            // 대기 상태일 때
    if (digitalRead(forwardBtn) == LOW) // 정방향 버튼이 눌리면 모터를 정방향으로 회전
    {
      motor.forward();
      sysState = RUNNING;
    }
    if (digitalRead(reverseBtn) == LOW) // 역방향 버튼이 눌리면 모터를 역방향으로 회전
    {
      motor.backward();
      sysState = RUNNING;
    }
    break;

  case RUNNING:                                                                        // 모터가 회전 중일 때
    if (motor.getDirection() == Direction::BACKWARD && digitalRead(forwardBtn) == LOW) // 모터가 역방향이고 정방향 버튼이 눌리면
    {
      // 반대 방향 누르면
      motor.stop();
      sysState = SWITCHING;
      stateStart = millis();
    }
    if (motor.getDirection() == Direction::BACKWARD && digitalRead(reverseBtn) == LOW) // 모터가 역방향이고 정방향 버튼이 눌리면
    {
      // 반대 방향 누르면
      motor.stop();
      sysState = SWITCHING;
      stateStart = millis();
    }
    break;

  case SWITCHING: // 방향 전환 중일 때
    if (motor.isStopped())
    {
      // 3초간 정지 + LED 깜빡
      if (millis() - stateStart < 3000) // 3초 대기
      {
        if ((millis() / 1000) % 2 == 0) // LED 깜빡
        {
          digitalWrite(ledPin, HIGH);
        }
        else
        {
          digitalWrite(ledPin, LOW);
        }
      }
      else
      {
        // 방향 결정
        if (digitalRead(forwardBtn) == LOW)
          motor.forward();
        if (digitalRead(reverseBtn) == LOW)
          motor.backward();
        sysState = RUNNING;
      }
    }
    break;
  }
}

// millis() 는 아두이노가 켜진 후 경과된 시간을 밀리초 단위로 반환합니다.
// pinMode(pin, mode) 는 핀의 모드를 설정합니다. mode는 INPUT, OUTPUT, INPUT_PULLUP 등이 있습니다.
// digitalRead(pin) 은 지정된 핀의 디지털 값을 읽습니다. HIGH 또는 LOW를 반환합니다.
// digitalWrite(pin, value) 는 지정된 핀에 디지털 값을 씁니다. value는 HIGH 또는 LOW입니다.
// Serial.begin(baudrate) 는 시리얼 통신을 시작합니다. baudrate는 통신 속도입니다.
// Serial.println(data) 는 시리얼 포트로 데이터를 출력하고 줄 바꿈을 추가합니다.
// INPUT_PULLUP 모드는 내부 풀업 저항을 활성화하여 버튼이 눌리지 않았을 때 HIGH 상태를 유지하게 합니다.