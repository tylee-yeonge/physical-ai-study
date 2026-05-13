# Week 8 실습


```bash
# Real + Sim 동시
ros2 run dynamixel_hardware dxl_node &
ros2 launch isaacsim digital_twin.launch.py &
ros2 run vla_node vla_inference_node &
ros2 run vla_node rerun_logger &


# 카메라 capture (좌측: Real, 우측: Sim)
python capture_side_by_side.py
```


체크리스트:
- [ ] 동시 실행 동작
- [ ] 3 trials 영상 capture
- [ ] 차이 시계열 plot
- [ ] 보고서 작성
