# Week 11: 카메라 Image Gap + Domain Randomization 시작

> [goal] **이번 주 목표**: Sim camera vs Real camera image 차이 정량 측정. Domain Randomization 시작.
> [time] **예상 시간**: 8시간

---

## [list] 학습 순서

| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 4 dim gap 측정 | `PRACTICE.md` 1 | Light/Color/Geo/Noise |
| 2 | 동일 viewpoint | `PRACTICE.md` 2 | 일관 비교 |
| 3 | Domain Randomization | `PRACTICE.md` 3 | Light/Material random |
| 4 | 퀴즈 | | |

---

## [ref] 핵심 개념

### 1. 4 dimension gap

| 차원 | Measurement |
|---|---|
| Lighting | RGB histogram 비교 |
| Color | per-channel mean/std |
| Geometry | image alignment |
| Noise | Laplacian variance |

### 2. RGB histogram

```python
real_hist = cv2.calcHist([real], [0,1,2], None, [16]*3, [0, 256]*3)
sim_hist = cv2.calcHist([sim], [0,1,2], None, [16]*3, [0, 256]*3)
dist = cv2.compareHist(real_hist, sim_hist, cv2.HISTCMP_CHISQR)
```

### 3. Color 분석

```python
real_mean = real.mean(axis=(0,1))
sim_mean = sim.mean(axis=(0,1))
diff = np.abs(real_mean - sim_mean)
```

### 4. Noise variance

```python
lap = cv2.Laplacian(img_gray, cv2.CV_64F)
noise_var = lap.var()
# Real: high
# Sim: ~ 0
```

### 5. Domain Randomization

Sim 에 random noise 주입:

```python
# Light intensity
intensity = np.random.uniform(2000, 5000)

# Color jitter (post-render)
sim_img += np.random.normal(0, 10, sim_img.shape).astype(np.uint8)
```

### 6. Phase 7 활용

Sim randomized -> OpenVLA action.
Real -> OpenVLA action.
두 결과 비교 -> robust 확인.

---

## [search] 자체 점검

**Q1. 4 dim gap?** > Light/Color/Geo/Noise.
**Q2. RGB histogram?** > cv2.calcHist + compareHist.
**Q3. Noise variance method?** > Laplacian filter.
**Q4. DR 목적?** > Sim 학습 model 의 Real robustness.
**Q5. Phase 7 활용?** > Sim DR vs Real OpenVLA action 비교.

---

## [note] 실습 + 다음

### 이번 주: image gap + DR + quiz
### 다음 주 (week 12): Sim/Real gap 종합 보고서

---

## [goal] 핵심 요약

1. **4 dim image gap** measurement
2. **Histogram + Laplacian**
3. **Domain Randomization 시작**
4. **Sim noise injection**
5. **Phase 7 robustness 검증**

[O] [Week 10](../week10/README.md) | [Week 12](../week12/README.md)
