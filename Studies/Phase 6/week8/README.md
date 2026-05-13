# Week 8: Latency 측정 인프라 (Sim, Real, gap)

> [goal] **이번 주 목표**: Sim 과 Real 의 latency 를 표준화된 방법으로 측정. Phase 7 산출물 #4 의 핵심 측정.
> [time] **예상 시간**: 8시간

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Timestamping 표준 | `PRACTICE.md` 1 | header.stamp 통일 |
| 2 | Sim latency 측정 | `PRACTICE.md` 2 | step time |
| 3 | Real latency 측정 | `PRACTICE.md` 3 | serial response |
| 4 | Gap 계산 + 시각화 | `PRACTICE.md` 4 | histogram |
| 5 | 퀴즈 | | |

---

## [ref] 핵심 개념

### 1. Latency 의 4 가지 measurement

```
A) Inference latency : image -> action (OpenVLA)
   - Phase 4 week 6 에서 165 ms 측정 완료

B) Sim step latency  : world.step() time
   - 30 FPS = 33 ms

C) Real actuator latency : action publish -> motor response
   - Dynamixel ~ 5~10 ms

D) Closed loop latency : image -> Real motor 응답
   - Phase 7 의 최종 측정 (모든 합)
```

### 2. Timestamping 표준

```python
# ROS2 header.stamp 사용
msg.header.stamp = self.get_clock().now().to_msg()
```

모든 노드가 ROS time 또는 Sim time 일관.

### 3. Sim latency 측정

```python
import time
t0 = time.time()
world.step(render=True)
sim_step_ms = (time.time() - t0) * 1000
```

기대: 33~50 ms (Sim 의 render 부담).

### 4. Real latency 측정

```python
# Joint command 발행
t_cmd = self.get_clock().now()

# 다음 joint_state callback 에서:
t_response = self.get_clock().now()
real_lat = (t_response - t_cmd).nanoseconds / 1e6
```

기대: Dynamixel XM430 ~ 5~10 ms.

### 5. Gap 계산

```
sim_real_gap = sim_latency - real_latency
```

음수면 Sim 이 느림 (보통). Phase 7 의 양산 의미 분석:
- "30 ms gap 이 robot 의 1 cm 위치 오차 (속도 0.3 m/s 시)"

### 6. Histogram 시각화

```python
import matplotlib.pyplot as plt
import numpy as np

sim_lats = [...]  # 1000 sample
real_lats = [...]

fig, ax = plt.subplots(1, 2, figsize=(10, 4))
ax[0].hist(sim_lats, bins=50)
ax[0].set_title("Sim Latency")
ax[1].hist(real_lats, bins=50)
ax[1].set_title("Real Latency")
plt.savefig("latency_hist.png")
```

### 7. Phase 7 산출물 #4 의 핵심

본 주의 인프라가 Phase 7 산출물 #4 의 결정타.
"Real-to-Sim-to-Real" 의 latency gap 을 정량적으로 측정.

---

## [search] 자체 점검

**Q1. 4 latency measurement?** > Inference / Sim step / Real actuator / Closed loop.
**Q2. Sim step latency?** > ~ 33~50 ms.
**Q3. Real Dynamixel?** > ~ 5~10 ms.
**Q4. Gap 의 양산 의미?** > 위치 오차 (속도 * gap).
**Q5. Phase 7 핵심 측정?** > Closed loop latency = inference + Sim/Real + actuator.

---

## [note] 실습 + 다음

### 이번 주: latency 측정 인프라 + quiz
### 다음 주 (week 9): 반복성

---

## [goal] 핵심 요약

1. **Timestamping 표준** header.stamp
2. **Sim latency** ~ 33-50 ms
3. **Real latency** ~ 5-10 ms
4. **Gap = sim - real**, 양산 의미
5. **Phase 7 의 결정타 측정**

[O] [Week 7](../week7/README.md) | [Week 9](../week9/README.md)
