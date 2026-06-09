# Hardware-Arm Stage 2 - Sim 물리 파라미터 매칭


> 2027.04 (Phase 6 진입 직전)


---


## 매칭할 물리 파라미터


| 항목 | Real (자작 팔) | Sim (Isaac Sim) |
|---|---|---|
| Joint friction | 측정 (week 10 of Phase 6) | URDF / PhysX param |
| Joint damping | 추정 | PhysX joint damping |
| Link mass | 무게 (저울) | URDF inertial |
| Link inertia | 추정 (mesh) | URDF inertial |
| Gravity | 9.81 m/s^2 | Default |


---


## 측정 절차


### 1. Friction 측정 (Real)


```bash
# Joint 를 천천히 움직여 stall 시점 토크 측정
ros2 topic pub /joint_command ... "data: [slow speed]"
# Joint 가 stop 하는 토크 = friction limit
```


### 2. Inertia 추정


```
3D 프린트 부품:
- mesh 의 volume 계산
- PLA density: 1.24 g/cm^3
- mass = volume * density
- inertia: mesh-based (Fusion 360 또는 trimesh)
```


### 3. URDF 의 inertial 갱신


```xml
<link name="link_1">
  <inertial>
    <origin xyz="..." rpy="..."/>
    <mass value="0.150"/> <!-- 실측 -->
    <inertia ixx="..." iyy="..." izz="..." ixy="..." ixz="..." iyz="..."/>
  </inertial>
</link>
```


### 4. PhysX joint property 설정


```python
# Isaac Sim
joint.GetFrictionAttr().Set(0.05) # 실측 friction
joint.GetDampingAttr().Set(0.1)
```


---


## 매칭 검증


```python
# Same trajectory in Sim and Real
# 비교: ee position, joint angle, force
```


기대: Sim/Real gap 줄어듦 (Phase 6 의 측정 baseline).


---


## Phase 6 / 7 와의 연결


```
Stage 2 의 sim param matching:
  -> Phase 6 의 Sim/Real gap 측정의 baseline
  -> Phase 7 의 Real-to-Sim-to-Real 의 sim 정확도
  -> Domain Randomization 의 시작점
```


---


## 체크리스트
- [ ] Friction 측정
- [ ] Link mass 실측
- [ ] URDF inertial 갱신
- [ ] PhysX joint property 설정
- [ ] Sim/Real 같은 trajectory 비교
- [ ] gap 줄어듦 확인
- [ ] Phase 6 진입 준비 완료


---


## 다음 단계


Phase 6 (2027.05-07) 와 병행 + Phase 7 (2027.08~) 산출물 v3 의 hardware 기반 완성.


Stage 2 의 모든 산출물 git tag stage2.
