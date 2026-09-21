# Week 5 실습


정지 동작은 motor disable (토크 OFF) 이 아니라 **현 위치 정지** 다. 토크를 끄면 팔이 자중으로 떨어지므로, 명령을 막고 현재 위치를 목표로 한 번 써서 토크를 유지한 채 그 자리에 세운다 ([Stage 2 안전 인터록](../../Hardware-Arm/stage2/safety_interlock.md) e-stop 절).


```cpp
// e_stop_node.cpp
void on_estop(...) {
    if (estop_pressed) {
        // 토크는 끄지 않는다 — 명령을 막고 현재 위치를 목표로 써서 그 자리에 세운다
        publish_hold_position();
    }
}


// collision_detector.cpp
void on_state(JointState msg) {
    auto change = abs(msg.effort - prev);
    if (change > threshold) {
        // 충돌도 같은 정지 동작을 쓴다 (토크 OFF 는 낙하)
        publish_hold_position();
    }
    prev = msg.effort;
}
```


체크리스트:
- [ ] e-stop button 동작 — 이동 중에 걸어도 토크를 유지한 채 그 자리에 정지 (낙하 없음)
- [ ] 충돌 감지 동작 — 감지 후 같은 현 위치 정지
- [ ] Reaction time 측정
