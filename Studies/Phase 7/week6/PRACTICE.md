# Week 6 실습


```bash
# Terminal 1: 자작 팔 driver
ros2 run dynamixel_hardware dxl_node


# Terminal 2: safety node (C++)
ros2 run vla_safety safety_node


# Terminal 3: vla_node
ros2 run vla_node vla_inference_node


# Terminal 4: instruction
ros2 topic pub --once /vla/instruction std_msgs/String "data: 'pick the red cup'"


# Terminal 5: latency monitor
ros2 topic echo /vla/latency_ms
```


체크리스트:
- [ ] 통합 파이프라인 동작
- [ ] 첫 Real 실행 (low speed)
- [ ] Latency breakdown 측정
- [ ] 안전 인터록 동작 확인 (intentional violation 테스트)
