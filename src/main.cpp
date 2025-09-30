#include <Arduino.h>
#include "../include/FerrisMoter.h"

// 핀 설정
const int L_EN = 7;
const int R_EN = 8;
const int L_PWM = 5;
const int R_PWM = 6;

const int BTN_FWD = 2;  // 정방향 버튼
const int BTN_BWD = 3;  // 역방향 버튼
const int POT_PIN = A0; // 가변저항 연결 핀
const int LED_PIN = 13; // 상태 LED

Motor motor(L_EN, R_EN, L_PWM, R_PWM);

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
  // motor.

  pinMode(BTN_FWD, INPUT_PULLUP);
  pinMode(BTN_BWD, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("FerrisCtrl 시작");
}

void loop()
{
  int potSpeed = readSpeed();
  motor.forward();

  motor.update();

  switch (sysState)
  {
  case IDLE:
    if (digitalRead(BTN_FWD) == LOW)
    {
      motor.forward();
      sysState = RUNNING;
    }
    if (digitalRead(BTN_BWD) == LOW)
    {
      motor.backward();
      sysState = RUNNING;
    }
    break;

  case RUNNING:
    if (digitalRead(BTN_FWD) == LOW && motor.isStopped() == false)
    {
      // 반대 방향 누르면
      motor.stop();
      sysState = SWITCHING;
      stateStart = millis();
    }
    if (digitalRead(BTN_BWD) == LOW && motor.isStopped() == false)
    {
      // 반대 방향 누르면
      motor.stop();
      sysState = SWITCHING;
      stateStart = millis();
    }
    break;

  case SWITCHING:
    if (motor.isStopped())
    {
      // 3초간 정지 + LED 깜빡
      if (millis() - stateStart < 3000)
      {
        if ((millis() / 1000) % 2 == 0)
        {
          digitalWrite(LED_PIN, HIGH);
        }
        else
        {
          digitalWrite(LED_PIN, LOW);
        }
      }
      else
      {
        // 방향 결정
        if (digitalRead(BTN_FWD) == LOW)
          motor.forward();
        if (digitalRead(BTN_BWD) == LOW)
          motor.backward();
        sysState = RUNNING;
      }
    }
    break;
  }
}

int readSpeed()
{
  int raw = analogRead(POT_PIN);         // 0 ~ 1023
  int speed = map(raw, 0, 1023, 0, 255); // 0 ~ 255
  return speed;
}

// millis() 는 아두이노가 켜진 후 경과된 시간을 밀리초 단위로 반환합니다.
// pinMode(pin, mode) 는 핀의 모드를 설정합니다. mode는 INPUT, OUTPUT, INPUT_PULLUP 등이 있습니다.
// digitalRead(pin) 은 지정된 핀의 디지털 값을 읽습니다. HIGH 또는 LOW를 반환합니다.
// digitalWrite(pin, value) 는 지정된 핀에 디지털 값을 씁니다. value는 HIGH 또는 LOW입니다.
// Serial.begin(baudrate) 는 시리얼 통신을 시작합니다. baudrate는 통신 속도입니다.
// Serial.println(data) 는 시리얼 포트로 데이터를 출력하고 줄 바꿈을 추가합니다.
// INPUT_PULLUP 모드는 내부 풀업 저항을 활성화하여 버튼이 눌리지 않았을 때 HIGH 상태를 유지하게 합니다.