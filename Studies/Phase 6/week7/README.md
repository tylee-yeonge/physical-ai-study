# Week 7: 카메라 부착 + Sim/Real 시각 비교


> **이번 주 목표**: 자작 팔의 전체 뷰 카메라 + Sim Camera 의 image 비교.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Real 카메라 확인 + FOV 측정 | `PRACTICE.md` 1 | 전체 뷰 카메라 (팔에 이미 고정돼 있다) |
| 2 | Sim Camera 동일 위치 | `PRACTICE.md` 2 | external, 실측 FOV 로 focal 맞춤 |
| 3 | Side-by-side 비교 | `PRACTICE.md` 3 | 시각 gap |
| 4 | 퀴즈 | | |


---


## 핵심 개념


### 1. Camera 부착 위치


- ee-mount (wrist cam): in-hand perspective
- external (table cam): third-person, 자작 팔 전체


본 phase: **external**. 자작 팔에는 카메라가 2대 달려 있다 — 손목 카메라와 전체 뷰 카메라 (둘 다 Realtek 웹캠, 1280x720 MJPG 30 fps). 여기서 Sim 과 비교하는 것은 전체 뷰 카메라다. 위치는 Stage 1 W3 에서 확정했다: 팔로워의 대각선 앞쪽 위에서 약 45도로 내려다보며, 받침 자리가 작업대에 표시돼 있다. **이번 주에 카메라를 옮기지 않는다** — 옮기면 v2.5 데이터셋의 시점과 달라진다. 카메라 설정의 원본은 [week2_guide](../../Hardware-Arm/spike/week2/week2_guide.md) §1.3-§1.4 다.


### 2. FOV 맞추기 — Real 과 Sim 의 "같은 렌즈"


용어: **FOV**(field of view, 화각) 는 카메라가 한 번에 담는 각도다. 가로 화각을 **HFOV** 라 한다. Sim 카메라를 Real 과 같은 자리에 두어도 화각이 다르면 물체 크기와 원근이 달라져 비교가 되지 않는다.


Real 카메라의 HFOV 는 사양서가 없어도 줄자로 잰다. 카메라에서 거리 **D** 만큼 떨어진 벽(또는 책상 위 종이)에서 화면 가로에 꽉 차는 폭 **L** 을 재면:


```
HFOV = 2 * atan(L / (2 * D))
```


Sim 카메라는 화각을 직접 받지 않고 **focal length**(초점거리, mm) 와 **aperture**(센서 가로 폭, mm) 로 정한다. 둘의 관계는 pinhole 공식 그대로다:


```
focal = aperture / (2 * tan(HFOV / 2))
```


aperture 값은 외워 쓰지 말고 `camera.get_horizontal_aperture()` 로 읽는다. Isaac Sim 버전에 따라 기본값이 다르다.


더 정확하게 하려면 Phase 2 week2 의 단일 카메라 캘리브레이션으로 내부 파라미터 `fx` 를 구한다. 그러면 `HFOV = 2 * atan(W / (2 * fx))` (W 는 가로 픽셀 수, 1280) 이다. 이번 주는 줄자 방식이면 충분하다 — 비교 영상의 목적은 수치 검증이 아니라 시각 gap 을 눈으로 보는 것이다.


### 3. Sim Camera 부착


```python
from omni.isaac.sensor import Camera
camera = Camera(
    prim_path="/World/ExternalCamera",
    position=np.array([0.45, -0.35, 0.50]),   # 줄자로 잰 Real 카메라 위치 (팔로워 베이스 기준, m)
    orientation=np.array([0.0, 0.7071, 0.0, 0.7071]),
    resolution=(1280, 720),                    # Real 과 같은 해상도
)
camera.set_focal_length(focal)                 # 실측 HFOV 로 계산한 값 (위 공식)
```


### 4. 시각 gap 4 차원


| 차원 | Real | Sim |
|---|---|---|
| Lighting | 자동 노출 / 변동 | 고정 |
| Color | white balance, gamma | linear |
| Geometry | lens distortion | pinhole 표준 |
| Noise | sensor noise | clean |


이게 week 11 의 자세한 측정 대상.


### 5. Side-by-side 영상


```python
ret, real = cap.read()                         # 전체 뷰 카메라 1280x720
sim = sim_camera.get_rgba()[:, :, :3]
side = np.hstack([real, sim])
cv2.imwrite("comparison.png", side)
```


Phase 7 산출물 v3 의 핵심 컨텐츠.


---


## 자체 점검


**Q1. 카메라 위치?** > External (전체 뷰 카메라, Stage 1 W3 에서 고정한 자리).
**Q2. focal_length?** > 줄자로 잰 HFOV 를 aperture 와 함께 공식에 넣어 계산한 값.
**Q3. 시각 gap 4?** > Lighting / Color / Geometry / Noise.
**Q4. Side-by-side 의 가치?** > Phase 7 산출물 v3 컨텐츠.
**Q5. 측정 시기?** > week 11.


---


## 실습 + 다음


### 이번 주: 카메라 확인 + FOV 측정 + 비교 + quiz
### 다음 주 (week 8): latency 측정 인프라 시작


---


## 핵심 요약


1. **External cam** 전체 뷰 카메라 (옮기지 않는다)
2. **Sim Camera** 동일 viewpoint + 실측 FOV
3. **Side-by-side** Phase 7 컨텐츠
4. **시각 gap 4 차원**
5. **Week 11 의 사전 측정**


- [Week 6](../week6/README.md) | [Week 8](../week8/README.md)
