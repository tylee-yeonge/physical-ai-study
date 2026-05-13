# Week 5: e-stop + 충돌 감지


> **이번 주 목표**: e-stop button + 토크 급증 기반 충돌 감지.
> **예상 시간**: 8시간


## 학습 순서
1. e-stop hardware (button or keyboard)
2. 토크 급증 감지
3. 통합 + Sim 검증
4. 퀴즈


## 핵심 개념


### e-stop
- Hardware button (preferred): GPIO 입력
- Software (keyboard): test 용
- Action: 즉시 motor disable + ROS2 emergency state


### 충돌 감지 (토크 급증)
```cpp
bool collision_detected(JointState current) {
    for (auto i : range(6)) {
        double torque_change = abs(current.effort[i] - prev_effort_[i]);
        if (torque_change > collision_threshold[i])
            return true; // 1ms 안에 토크 급증 = 충돌
    }
    return false;
}
```


### Latency
- e-stop -> motor disable: ~ 2 ms
- collision detect -> motor disable: ~ 5 ms


## 자체 점검
Q1. e-stop 방식? > GPIO button (실), keyboard (test).
Q2. 충돌 감지? > 토크 급증.
Q3. Reaction time? > < 5 ms.


## 요약
1. e-stop button + keyboard
2. 토크 급증 = 충돌
3. ~ 5 ms reaction
4. "안전 메커니즘" 직접 증거


- [Week 4](../week4/README.md) | [Week 6](../week6/README.md)
