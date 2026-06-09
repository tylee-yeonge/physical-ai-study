# Week 3: ROS2 Bridge - Sim <-> ROS2 통합


> **이번 주 목표**: omni.isaac.ros2_bridge 로 Isaac Sim 데이터를 ROS2 topic 으로. Phase 4 의 vla_node 와 통합 준비.
> **예상 시간**: 8시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Bridge extension 활성화 | `PRACTICE.md` 1 | omni.isaac.ros2_bridge |
| 2 | joint_state publish | `PRACTICE.md` 2 | Sim joint -> /joint_states |
| 3 | camera image publish | `PRACTICE.md` 3 | /camera/image_raw |
| 4 | tf publish | `PRACTICE.md` 4 | /tf |
| 5 | 퀴즈 | | |


---


## 핵심 개념


### 1. omni.isaac.ros2_bridge


활성화:
```python
import omni.kit.app
m = omni.kit.app.get_app().get_extension_manager()
m.set_extension_enabled_immediate("omni.isaac.ros2_bridge", True)
```


### 2. 표준 publish list


| Topic | Type | Source |
|---|---|---|
| /clock | Clock | Sim 시간 |
| /joint_states | JointState | Articulation joint |
| /camera/image_raw | Image | Camera prim |
| /tf | TFMessage | 모든 Xform |
| /camera/depth | Image | (선택) depth |


### 3. Action Graph


Sim 의 publish/subscribe graph. GUI 또는 Python API.


```
ROS2 Camera Helper -> ROS2 Publish Image
SubscribeJointState -> Articulation Controller
```


### 4. Latency 특성


```
Sim step (30 FPS) : 33 ms
Bridge publish : <1 ms
ROS2 transport (DDS) : <5 ms
Total : ~ 38 ms
```


### 5. Sim time vs Real time


- Sim time: deterministic, `use_sim_time:=true`
- Real time: wall clock


본 phase: Sim time 사용 (측정 일관성).


### 6. Phase 4 와의 통합 (Phase 7 토대)


```
Isaac Sim (ROS2 Bridge)
  -> /camera/image_raw, /joint_states


vla_node (Phase 4)
  -> /camera/image_raw subscribe
  -> /vla/action publish


joint_command_node
  -> /vla/action -> /joint_command


Isaac Sim
  -> /joint_command -> articulation control
```


Closed loop = Phase 7 산출물 v3 의 토대.


---


## 자체 점검


**Q1. Bridge extension 이름?** > omni.isaac.ros2_bridge.
**Q2. 표준 publish 5?** > /clock, /joint_states, /image_raw, /tf, /camera/depth.
**Q3. Action Graph?** > Sim publish/subscribe graph.
**Q4. Sim time?** > deterministic, use_sim_time parameter.
**Q5. Bridge latency?** > ~38 ms.


---


## 실습


### 이번 주: Bridge 활성 + 표준 publish + quiz
### 다음 주 (week 4): URDF -> USD


---


## 핵심 요약


1. **omni.isaac.ros2_bridge** activate
2. **표준 publish 5** topic
3. **Action Graph** 로 구성
4. **Sim time** 일관성
5. **Phase 7 closed loop 토대**


- [Week 2](../week2/README.md) | [Week 4](../week4/README.md)
