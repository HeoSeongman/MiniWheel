#include <Arduino.h>

// Delegate 클래스는 "멀티캐스트 이벤트" 구조를 구현합니다.
// - 하나의 이벤트에 여러 콜백 함수를 등록(bind)할 수 있음
// - 등록된 콜백을 모두 호출(broadcast) 가능
// - 콜백 등록 해제(unbind) 가능
// - 매개변수 타입과 개수를 템플릿 인자로 지정할 수 있음
// 예시: Delegate<int, float> → void(int, float) 타입 콜백

/**
  // Delegate 객체 생성
  Delegate<> buttonEvent;                   // void()
  Delegate<int> speedEvent;                 // void(int)
  Delegate<int, float> directionEvent;      // void(int, float)

  // 구독 등록
  buttonEvent.bind(OnButtonAPressed);
  speedEvent.bind(OnSpeedChanged);
  directionEvent.bind(OnDirectionChanged);

  // 호출
  buttonEvent.broadcast();
  speedEvent.broadcast(120);
  directionEvent.broadcast(1, 0.75f);
 */

template <typename... Args>
class Delegate {
public:
    // Callback 타입 정의
    // Args... 는 가변 매개변수
    using Callback = void (*)(Args...);

    // 생성자: 배열 초기화
    Delegate() : callbacks(nullptr), count(0), capacity(0) {}

    // 소멸자: 동적할당 해제
    ~Delegate() {
        if (callbacks) {
            free(callbacks);
        }
    }

    // ==================== bind ====================
    // 콜백 함수 등록
    // - count >= capacity이면 grow()를 통해 배열 확장
    void bind(Callback cb) {
        if (count >= capacity) {
            grow();
        }
        callbacks[count++] = cb;
    }

    // ==================== unbind ====================
    // 콜백 함수 제거
    // - 배열에서 cb를 찾아 제거 후, 뒤의 요소들을 앞으로 이동
    void unbind(Callback cb) {
        for (int i = 0; i < count; i++) {
            if (callbacks[i] == cb) {
                for (int j = i; j < count - 1; j++) {
                    callbacks[j] = callbacks[j + 1];
                }
                count--;
                break;
            }
        }
    }

    // ==================== broadcast ====================
    // 등록된 모든 콜백 호출
    void broadcast(Args... args) {
        for (int i = 0; i < count; i++) {
            callbacks[i](args...);
        }
    }

private:
    Callback* callbacks; // 콜백 함수 배열 (동적할당)
    int count;           // 현재 등록된 콜백 수
    int capacity;        // 배열 용량

    // ==================== grow ====================
    // 배열 용량을 2배로 늘림
    // - capacity == 0이면 초기 용량 4
    void grow() {
        int newCapacity = (capacity == 0) ? 4 : capacity * 2;
        Callback* newArray = (Callback*)malloc(sizeof(Callback) * newCapacity);

        // 기존 콜백 복사
        for (int i = 0; i < count; i++) {
            newArray[i] = callbacks[i];
        }

        // 이전 배열 해제
        if (callbacks) {
            free(callbacks);
        }

        callbacks = newArray;
        capacity = newCapacity;
    }
};
