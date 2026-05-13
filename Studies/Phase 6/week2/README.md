# Week 2: USD / Stage / Prim 심화 + Camera, Light, Material


> **이번 주 목표**: Isaac Sim Stage / Prim API 를 자유롭게. Camera + Light + Material 사용.
> **예상 시간**: 8시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | USD / Stage API | `PRACTICE.md` 1 | prim_path |
| 2 | Camera Prim | `PRACTICE.md` 2 | 위치 / FOV |
| 3 | Light + Material | `PRACTICE.md` 3 | 사실성 |
| 4 | Stage 저장 / 로드 | `PRACTICE.md` 4 | reusable |
| 5 | 퀴즈 | | |


---


## 핵심 개념


### 1. Stage hierarchy


```
/World
   /cube
   /sphere
   /Camera
      /Camera_node
   /Lights
      /defaultLight
```


### 2. Prim 종류


| Prim | 용도 |
|---|---|
| Xform | Transform (위치/회전) |
| Cube/Sphere | 기본 객체 |
| Mesh | 사용자 정의 |
| Camera | 시각 sensor |
| RectLight | 조명 |
| PhysicsScene | 물리 설정 |


### 3. Camera 추가


```python
from omni.isaac.sensor import Camera


camera = Camera(
    prim_path="/World/Camera",
    position=np.array([0.5, 0.5, 1.0]),
    orientation=np.array([1.0, 0.0, 0.0, 0.0]),
    resolution=(640, 480),
)
camera.initialize()
camera.set_focal_length(15.0)
```


### 4. Light


```python
from omni.isaac.core.utils.prims import create_prim
create_prim("/World/DistantLight", "DistantLight",
            attributes={"intensity": 3000})
```


### 5. Material


```python
from omni.isaac.core.materials import OmniPBR
mat = OmniPBR("/Materials/red", color=np.array([1.0, 0, 0]),
              metallic=0.0, roughness=0.5)
cube.apply_visual_material(mat)
```


### 6. Sim/Real 시각 gap 의 시작


본 주가 Phase 6 week 11 (시각 gap 측정) 의 토대:
- Light 부족 -> 그림자 부자연
- Material 정확성 -> reflective surface
- Camera distortion 모사


---


## 자체 점검


**Q1. prim_path 형식?** > `/World/<name>` linux 비슷.
**Q2. Camera 인자?** > position, orientation (quat), resolution, focal_length.
**Q3. Material 의 의의?** > Sim 의 시각 realism. Sim/Real gap 의 핵심.
**Q4. Light intensity 단위?** > Lumens. indoor 1000-5000.
**Q5. Sim/Real 시각 gap 원인?** > Light / Material / Camera distortion 차이.


---


## 실습 + 다음


### 이번 주: Camera/Light/Material + Stage 저장 + quiz
### 다음 주 (week 3): ROS2 Bridge


---


## 핵심 요약


1. **Stage hierarchy** `/World/...`
2. **Camera** prim
3. **Light** intensity
4. **Material** color/metallic/roughness
5. **Sim/Real 시각 gap 의 토대**


- [Week 1](../week1/README.md) | [Week 3](../week3/README.md)
