# Week 12 실습: 종합 보고서 + Phase 7 준비


> **예상 시간**: 4시간


---


## 실습 1: Sim/Real Gap 종합 보고서


`~/phase6_notes/week12/sim_real_gap_report.md` (week 8-11 결과 통합):


```markdown
# Sim/Real Gap Report


## 환경
- Robot: 자작 6DOF 팔 (Dynamixel XM430)
- Sim: Isaac Sim 4.x
- Real: ROS2 Humble
- 측정 일자: 2027.07 (Phase 6 종료)


## 1. Latency (week 8)
| Component | Sim | Real | Gap |
|---|---|---|---|
| Inference (OpenVLA) | 165 ms | 165 ms | 0 |
| Step / Actuator | 35 ms | 8 ms | -27 ms |
| Closed loop | 210 ms | 183 ms | -27 ms |


## 2. 반복성 (week 9, 100회)
| | Sim std | Real std |
|---|---|---|
| EE | 0.05 mm | 2.3 mm |
| Joint | <0.001 rad | 0.005 rad |


## 3. Force / Torque (week 10)
- Friction parameter Sim 0.01 vs Real ~ 0.05
- 정밀 force 작업 시 Sim/Real 큰 gap


## 4. Camera Image (week 11)
- RGB hist distance: 145 -> 38 (Domain Randomization 후)
- Noise variance: Sim 12 vs Real 320 -> Sim+DR 280


## 결론
1. Latency gap: Sim 이 27ms 느림 -> 양산 시 hierarchical 필요
2. Real variability 5~10x Sim -> noise robust 학습 필요
3. Force friction tuning 필요
4. Image DR 4x 향상


Phase 7 의 산출물 #4 영상에 인용 가능한 정량 증거.
```


---


## 실습 2: Rerun 통합 시각화


```python
"""
practice_rerun_integration.py
- 4 gap 데이터를 Rerun 으로 동시 표시
"""
import rerun as rr
rr.init("phase6_sim_real_gap", spawn=True)


# 시간 축
for i, t in enumerate(timestamps):
    rr.set_time_seconds("time", t)


    # camera image
    rr.log("real/camera/image", rr.Image(real_imgs[i]))
    rr.log("sim/camera/image", rr.Image(sim_imgs[i]))


    # joint state
    for j in range(6):
        rr.log(f"real/joint_{j}", rr.Scalar(real_joints[i, j]))
        rr.log(f"sim/joint_{j}", rr.Scalar(sim_joints[i, j]))


    # latency
    rr.log("latency/closed_loop", rr.Scalar(latencies[i]))


    # torque
    for j in range(6):
        rr.log(f"real/torque_{j}", rr.Scalar(real_torques[i, j]))
        rr.log(f"sim/effort_{j}", rr.Scalar(sim_efforts[i, j]))
```


Rerun UI 에서 4 panel 동시 시각화. Phase 7 산출물 #4 영상의 핵심 컨텐츠.


---


## 실습 3: Phase 6 회고


`~/phase6_notes/retro.md`:


```markdown
# Phase 6 회고 (2027.05 ~ 2027.07)


## 시간 분배
- Isaac Sim 셋업 (week 1-3): 계획 3주, 실제 ___
- URDF + 디지털 트윈 (week 4-7): ___
- Sim/Real gap (week 8-12): ___


## 잘 한 것
- 4 gap 정량 측정 (Phase 7 의 직접 입력)
- Rerun 통합 시각화
- ___


## 아쉬운 것
- ___


## 다음 (Phase 7) 에 적용할 것
- Sim/Real gap 의 dominant factor (image / friction) 우선 보완
- Domain Randomization 의 범위 확장
- ___


## 분기 재평가 #2 (2027.05)
- Phase 5/6 결과
- 자작 팔 Stage 2 완성도
- VLA 모델 갱신 검토
- Phase 7 시점 + 실지원 점검


## 산출물 list
- ~/phase6_notes/week8/latency_compare.png
- week9/repeatability.png
- week10/torque_compare.png
- week11/image_gap_report.md
- week12/sim_real_gap_report.md
- (Rerun) phase6_sim_real_gap.rrd
```


---


## 실습 4: Phase 7 진입 직전 점검


```
- [ ] Isaac Sim + ROS2 Bridge 안정 동작
- [ ] 자작 팔 Stage 2 (6DOF + teleop) 완성
- [ ] OpenVLA + ROS2 (Phase 4) 환경 그대로 사용 가능
- [ ] LoRA fine-tune (Phase 5) 환경 준비
- [ ] Sim/Real gap 측정 코드 모두 통합 가능
- [ ] 분기 재평가 #2 결과 반영
```


---


## 체크리스트
- [ ] 종합 보고서 작성
- [ ] Rerun 통합 시각화
- [ ] retro 작성
- [ ] Phase 7 진입 점검
- [ ] 분기 재평가 #2 결과 반영
- [ ] quiz
- [ ] Phase 6 git tag (v0.6)
