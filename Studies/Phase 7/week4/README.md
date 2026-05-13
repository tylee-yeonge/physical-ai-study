# Week 4: 안전 인터록 - 위치/속도/토크 한계 (C++)

> [goal] **이번 주 목표**: C++ 기반 안전 인터록 노드. "안전 메커니즘" (e-stop/토크 한계/충돌 감지) 의 직접 증거.
> [time] **예상 시간**: 10시간

---

## 학습 순서
1. C++ ROS2 노드 골격
2. 위치/속도/토크 한계 함수
3. ROS2 통합 (subscribe inference + publish motor)
4. 퀴즈

## 핵심 개념

### C++ 의 이유
- Python 보다 1-2 ms 빠름
- Real-time safety critical
- 양산 SW 표준

### 한계 매트릭스
```
Position limits : URDF 의 joint limit 그대로
Velocity limits : 30 deg/s per joint (자작 팔)
Torque limits   : 모터 max 의 80% (XM430: ~3 Nm)
```

### 인터록 알고리즘
```cpp
bool check_safety(JointCommand cmd, JointState current) {
    // Position
    for (auto i : range(6)) {
        if (cmd.position[i] < lower[i] || cmd.position[i] > upper[i])
            return false;
    }
    // Velocity (delta)
    for (auto i : range(6)) {
        double vel = (cmd.position[i] - current.position[i]) / dt;
        if (abs(vel) > max_velocity[i])
            return false;
    }
    // Torque
    for (auto i : range(6)) {
        if (abs(current.effort[i]) > max_torque[i])
            return false;
    }
    return true;
}
```

### 인터록 오버헤드
- C++: ~ 1 ms (양산 가능)
- Python: ~ 3-5 ms

## 자체 점검
Q1. C++ 의 이유? > 1-2 ms 빠름, 양산 SW 표준.
Q2. 3 한계? > Position / Velocity / Torque.
Q3. Torque 한계? > 모터 max 의 80%.
Q4. 오버헤드? > C++ ~ 1 ms.
Q5. 양산 차별화 메시지? > "안전 메커니즘" 의 직접 증거 (e-stop / 토크 한계 / 충돌 감지).

## 요약
1. C++ ROS2 노드
2. Position/Velocity/Torque 한계
3. ~ 1 ms 오버헤드
4. "안전 메커니즘" 직접 증거

[O] [Week 3](../week3/README.md) | [Week 5](../week5/README.md)
