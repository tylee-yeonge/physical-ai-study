# Week 5 실습


```cpp
// e_stop_node.cpp
void on_estop(...) {
    if (estop_pressed) {
        publish_motor_disable();
    }
}


// collision_detector.cpp
void on_state(JointState msg) {
    auto change = abs(msg.effort - prev);
    if (change > threshold) {
        publish_motor_disable();
    }
    prev = msg.effort;
}
```


체크리스트:
- [ ] e-stop button 동작
- [ ] 충돌 감지 동작
- [ ] Reaction time 측정
