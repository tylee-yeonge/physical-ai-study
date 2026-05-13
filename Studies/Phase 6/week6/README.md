# Week 6: Sim Joint State <-> Real Joint State 매칭


> **이번 주 목표**: Real 자작 팔 (Hardware-Arm Stage 2) 의 joint state 를 ROS2 로 받아 Isaac Sim 의 joint 와 매칭. 첫 디지털 트윈.
> **예상 시간**: 8시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Real robot ROS2 셋업 점검 | `PRACTICE.md` 1 | Hardware-Arm Stage 2 |
| 2 | /joint_states subscribe | `PRACTICE.md` 2 | Real -> Sim |
| 3 | Sim joint set + 시각 매칭 | `PRACTICE.md` 3 | 두 robot 같은 자세 |
| 4 | 첫 디지털 트윈 영상 | `PRACTICE.md` 4 | side-by-side |
| 5 | 퀴즈 | | |


---


## 핵심 개념


### 1. 디지털 트윈의 의미


```
Real robot 의 joint state -> ROS2 /joint_states
                                |
                                v
                    Isaac Sim 의 동일 robot 에 적용
                                |
                                v
                    Sim 의 robot 이 Real 과 같은 자세
```


매 ~ 30 Hz 로 sync. Latency ~ 50 ms.


### 2. ROS2 통합 코드


```python
# digital_twin_node.py
class DigitalTwinNode(Node):
    def __init__(self):
        super().__init__('digital_twin')
        self.sub = self.create_subscription(
            JointState, '/joint_states', self.on_joint, 30)
        # Sim 의 arm reference (Isaac Sim 안에서)


    def on_joint(self, msg):
        # ROS2 joint order -> Sim DOF order 매핑
        target = self.map_ros_to_sim(msg.position)
        self.arm.set_joint_position_targets(target)
```


### 3. Latency 측정


```python
t_real = msg.header.stamp # joint_state 의 timestamp
t_sim = self.get_clock().now()
latency = (t_sim - t_real).nanoseconds / 1e6
print(f"Real->Sim latency: {latency:.1f} ms")
# 기대 < 100 ms
```


### 4. 매칭 정확도 측정


```
관절각 비교:
  Real joint angle [j_1, ..., j_6]
  Sim joint angle [s_1, ..., s_6]
  diff = max( |j_i - s_i| )
  # 기대 < 0.01 rad (대략 0.5 deg)


end-effector 위치 비교:
  Real ee position (Real FK)
  Sim ee position (Sim FK)
  err = norm(real_ee - sim_ee)
  # 기대 < 5 mm
```


### 5. 한계


- Sim 의 actuator response 가 Real 과 다름
- Sim 의 inertia 가 정확하지 않으면 dynamic mismatch
- Sensor noise (Real 만) -> Sim 의 결과는 cleaner


이 한계가 Phase 6 week 9~10 의 측정 인프라의 토대.


---


## 자체 점검


**Q1. 디지털 트윈의 정의?** > Real robot 의 state 를 Sim 에 동기화한 환경.
**Q2. ROS2 의 /joint_states subscribe + apply?** > callback 에서 set_joint_position_targets.
**Q3. 매칭 정확도 기준?** > joint < 0.01 rad, ee < 5 mm.
**Q4. Latency 기대?** > < 100 ms.
**Q5. 한계?** > Sim actuator / inertia 가 Real 과 다름 -> dynamic mismatch.


---


## 실습 + 다음


### 이번 주: digital_twin_node + 시각 매칭 + quiz
### 다음 주 (week 7): 카메라 부착 + 시각 비교


---


## 핵심 요약


1. **디지털 트윈** Real -> Sim 동기화
2. **ROS2 /joint_states subscribe** + set_joint_position_targets
3. **Latency < 100 ms, joint < 0.5 deg, ee < 5 mm**
4. **매핑 표** 필수 (week 5 의 결과)
5. **Dynamic mismatch** 한계 + week 9~10 의 측정


- [Week 5](../week5/README.md) | [Week 7](../week7/README.md)
