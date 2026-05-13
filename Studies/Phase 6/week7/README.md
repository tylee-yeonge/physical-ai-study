# Week 7: 카메라 부착 + Sim/Real 시각 비교

> [goal] **이번 주 목표**: 자작 팔 카메라 (ELP Stereo) + Sim Camera 의 image 비교.
> [time] **예상 시간**: 6시간

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Real 카메라 부착 | `PRACTICE.md` 1 | ELP Stereo |
| 2 | Sim Camera 동일 위치 | `PRACTICE.md` 2 | external |
| 3 | Side-by-side 비교 | `PRACTICE.md` 3 | 시각 gap |
| 4 | 퀴즈 | | |

---

## [ref] 핵심 개념

### 1. Camera 부착 위치

- ee-mount (wrist cam): in-hand perspective
- external (table cam): third-person, 자작 팔 전체

본 phase: **external** (ELP Stereo 보유).

### 2. Sim Camera 부착

```python
from omni.isaac.sensor import Camera
camera = Camera(
    prim_path="/World/ExternalCamera",
    position=np.array([0.5, 0.0, 0.4]),
    orientation=np.array([0.0, 0.7071, 0.0, 0.7071]),
    resolution=(640, 480),
)
camera.set_focal_length(15.0)
```

### 3. 시각 gap 4 차원

| 차원 | Real | Sim |
|---|---|---|
| Lighting | 자동 노출 / 변동 | 고정 |
| Color | white balance, gamma | linear |
| Geometry | lens distortion | pinhole 표준 |
| Noise | sensor noise | clean |

이게 week 11 의 자세한 측정 대상.

### 4. Side-by-side 영상

```python
real = elp.read()
sim = sim_camera.get_rgba()[:, :, :3]
side = np.hstack([real, sim])
cv2.imwrite("comparison.png", side)
```

Phase 7 산출물 #4 의 핵심 컨텐츠.

---

## [search] 자체 점검

**Q1. 카메라 위치?** > External (ELP Stereo).
**Q2. focal_length?** > Real 과 같이 ~ 15 mm.
**Q3. 시각 gap 4?** > Lighting / Color / Geometry / Noise.
**Q4. Side-by-side 의 가치?** > Phase 7 산출물 #4 컨텐츠.
**Q5. 측정 시기?** > week 11.

---

## [note] 실습 + 다음

### 이번 주: 카메라 부착 + 비교 + quiz
### 다음 주 (week 8): latency 측정 인프라 시작

---

## [goal] 핵심 요약

1. **External cam** ELP
2. **Sim Camera** 동일 viewpoint
3. **Side-by-side** Phase 7 컨텐츠
4. **시각 gap 4 차원**
5. **Week 11 의 사전 측정**

[O] [Week 6](../week6/README.md) | [Week 8](../week8/README.md)
